/******************************************************************************/
#include "stdafx.h"
/******************************************************************************

   Warning/TODO/FIXME: Once every few months 'UpdateCertificates' should be called to update latest version of trusted certificates for SSL/TLS/HTTPS
      Last Updated: 1 Feb 2019

/******************************************************************************/
#define UPDATE_CERTIFICATES (DEBUG && 0)
/******************************************************************************/
#define FORCE_IPV6 1 // prefer IPv6 addresses to avoid unnecessary 'convert' calls

#if !WINDOWS
   #define SOCKET        UIntPtr
   #define SOCKET_ERROR -1
#endif

#if WINDOWS_OLD && SUPPORT_WINDOWS_XP
   static const Bool DualStackSocket=(OSVerNumber().x>5); // not supported on WinXP, on WinXP one Socket can't be used for IPv4 and IPv6 connections at the same time, "In order to support both IPv4 and IPv6 on Windows XP with Service Pack 1 (SP1) and on Windows Server 2003, an application has to create two sockets, one socket for use with IPv4 and one socket for use with IPv6. These two sockets must be handled separately by the application." - https://msdn.microsoft.com/en-us/library/windows/desktop/bb513665(v=vs.85).aspx
#else
   #define           DualStackSocket true // other OS's should support it
#endif

#define IP4_ANY        0        // "0.0.0.0"   any address will be selected
#define IP4_LOCAL_HOST 16777343 // "127.0.0.1" localhost

#ifndef INET6_ADDRSTRLEN
   #define INET6_ADDRSTRLEN 65 // maximum length of an IPv6 address in text format (this constant was obtained from "ws2ipdef.h") (45: IPv6 address including embedded IPv4 address, 11: Scope Id, 2: Brackets around IPv6 address when port is present, 6: Port (including colon), 1: Terminating null byte)
#endif

#define LOG_ERROR 0
#if     LOG_ERROR
   #pragma message("!! Warning: Use this only for debugging !!")
#endif

namespace EE{
/******************************************************************************/
#define IPV6_SIZE 16 // 16 bytes
ASSERT(MEMBER_SIZE(sockaddr_in6, sin6_addr)==IPV6_SIZE);

#define IPV6_UINTS (16/SIZE(UInt)) // number of UInt's in IPv6 address
ASSERT(IPV6_SIZE%SIZE(UInt)==0);

static INLINE Int CompareV6(C UInt *a, C UInt *b) // assumes that a!=null && b!=null
{
   REP(IPV6_UINTS)if(Int c=Compare(*a++, *b++))return c;
   return 0;
}

#if WINDOWS_OLD && SUPPORT_WINDOWS_XP // Windows XP doesn't have 'inet_pton' and 'inet_ntop' and apps won't start, so the functions need to be written manually
static Int inet_ptonXP(Int addr_family, CChar8 *src, Ptr addr)
{
   Char8            src_copy[INET6_ADDRSTRLEN]; Set(src_copy, src); // need to make a copy because the 'WSAStringToAddressA' param is not const
   sockaddr_storage sock_addr; Zero(sock_addr);
   int              size=SIZE(sock_addr);
   if(WSAStringToAddressA(src_copy, addr_family, null, (sockaddr*)&sock_addr, &size)==0)switch(addr_family)
   {
      case AF_INET : *( in_addr*)addr=((sockaddr_in &)sock_addr). sin_addr; return 1;
      case AF_INET6: *(in6_addr*)addr=((sockaddr_in6&)sock_addr).sin6_addr; return 1;
   }
   return 0;
}
static CChar8* inet_ntopXP(Int addr_family, CPtr addr, Char8 *dest, DWORD dest_length)
{
   sockaddr_storage sock_addr; Zero(sock_addr);
   switch(sock_addr.ss_family=addr_family)
   {
      case AF_INET : ((sockaddr_in &)sock_addr). sin_addr=*( in_addr*)addr; break;
      case AF_INET6: ((sockaddr_in6&)sock_addr).sin6_addr=*(in6_addr*)addr; break;
      default      : return null;
   }
   return (WSAAddressToStringA((sockaddr*)&sock_addr, SIZE(sock_addr), null, dest, &dest_length)==0) ? dest : null;
}
#define inet_pton inet_ptonXP
#define inet_ntop inet_ntopXP
#endif
/******************************************************************************/
static ULong   Mac;
static Str     ComputerName;
#if WINDOWS
static WSADATA WsaData;
#endif
static CChar8 *GlobalIPSites[]=
{
   "http://checkip.dyndns.org", // 0 - returns text "Current IP Address: X.X.X.X"
   "http://www.icanhazip.com/", // 1
   "http://ifconfig.me/ip"    , // 2
 //"http://automation.whatismyip.com/n09230945.asp", noticed on Feb 2013 that it is now paid only
 //"http://www.whatismyip.org/", noticed on May 2012 that this no longer returns IP in text format (but in HTML+graphics image)
}; static const Int GlobalIPSiteSkipText=0;
/******************************************************************************/
static Bool TextToIP4(CChar8 *text, UInt &ip)
{
   VecB4     v;
   CalcValue c;
   FREPA(v.c)
   {
      text=TextValue(text, c, false); if(c.type!=CVAL_INT || c.i<0 || c.i>255)goto error;
      if((i<3) ? !text || *text++!='.' : Is(text))goto error;
      v.c[i]=c.i;
   }
   ip=v.u; return true;
error:
   ip=0; return false;
}
static Bool EqualIgnorePort(C SockAddr &a, C SockAddr &b) // !! this ignores port and v6(v4-mapped) IP4_LOCAL_HOST and IP4_ANY !!
{
   if(a.family()!=b.family()) // they can still be the same if it's a v4 and v6(v4-mapped)
   {
    C SockAddr *v4=null, *v6=null;
      switch(a.family())
      {
         case AF_INET : v4=&a; break;
         case AF_INET6: v6=&a; break;
         default      : return false;
      }
      switch(b.family())
      {
         case AF_INET : v4=&b; break;
         case AF_INET6: v6=&b; break;
         default      : return false;
      }
      return v4 && v6 && v6->v6Ip4()==v4->v4Ip4() && IN6_IS_ADDR_V4MAPPED(&v6->v6().sin6_addr); // check IP4's first because it's faster
   }
   switch(a.family()) // at this stage a.family==b.family
   {
      default      : return true;
      case AF_INET : return a.v4Ip4()==b.v4Ip4();
      case AF_INET6: return IN6_ARE_ADDR_EQUAL(&a.v6().sin6_addr, &b.v6().sin6_addr)!=0;
   }
}
/******************************************************************************/
struct GlobalIPChecker
{
   Bool     busy;
   SockAddr addr;
   Download d[Elms(GlobalIPSites)];
   SyncLock lock;

   GlobalIPChecker() {busy=false;}
  ~GlobalIPChecker() {del();}

   static void Update(GlobalIPChecker &gipc) {gipc.update(true);}

   Bool valid()C {return addr.family()!=0;}
   void start()
   {
      if(!busy)
      {
         SyncLocker locker(lock); if(!busy)
         {
            busy=true;
            REPAO(d).create(GlobalIPSites[i]);
            App._callbacks.add(Update, T);
         }
      }
   }
   void del()
   {
      if(busy)
      {
         SyncLocker locker(lock); if(busy)
         {
            REPAO(d).stop(); App._callbacks.exclude(Update, T);
            REPAO(d).del ();
            busy=false;
         }
      }
   }
   void update(Bool callback)
   {
      SyncLocker locker(lock);
      Bool waiting=false; // if any of the downloads are still active
      REPA(d)switch(d[i].state())
      {
         case DWNL_CONNECTING:
         case DWNL_AUTH      :
         case DWNL_SENDING   :
         case DWNL_DOWNLOAD  : waiting=true; break;

         case DWNL_DONE:
         {
            FileText f; f.readMem(d[i].data(), d[i].done());
            Str8     t; f.getLine(t); CChar8 *text=t;
            if(i==GlobalIPSiteSkipText)
            {
               if(text=TextPos(text, ':'))text++;
               if(text && text[0]==' '   )text++;
            }
            if(addr.ipText(text))
            {
               del(); return;
            }
            d[i].del(); // don't process this download again
         }break;
      }
      if(!waiting)del();else if(callback)App._callbacks.add(Update, T);
   }
   void wait()
   {
      for(;;)
      {
         update(false);
      #if HAS_THREADS
         if(!busy)break;
         Time.wait(1);
      #else
         break; // when there are no threads, waiting can do nothing
      #endif
      }
   }
};
static GlobalIPChecker GIPC;
/******************************************************************************/
// SOCKET ADDRESS
/******************************************************************************/
Bool SockAddr::valid()C
{
   switch(family())
   {
      case AF_INET :
      case AF_INET6: return true;
   }
   return false;
}
Bool SockAddr::thisDevice()C
{
   switch(family())
   {
      default      : return false;
      case AF_INET : if(v4Ip4()==IP4_LOCAL_HOST)return true; break;
      case AF_INET6: if(v6Ip4()==IP4_LOCAL_HOST && IN6_IS_ADDR_V4MAPPED(&v6().sin6_addr) // check IP4 first because it's faster
                     ||                            IN6_IS_ADDR_LOOPBACK(&v6().sin6_addr))return true; break;
   }
   Memt<SockAddr> addrs; GetLocalAddresses(addrs); REPA(addrs)if(EqualIgnorePort(T, addrs[i]))return true;
   return false;
}
Bool SockAddr::needsV6()C
{
   return family()==AF_INET6
   && !IN6_IS_ADDR_V4MAPPED   (&v6().sin6_addr)
   && !IN6_IS_ADDR_LOOPBACK   (&v6().sin6_addr)
   && !IN6_IS_ADDR_UNSPECIFIED(&v6().sin6_addr);
}
/******************************************************************************
Bool SockAddr::universal()C // if socket can be of any family
{
   switch(family())
   {
      case AF_INET: return v4Ip4()==IP4_LOCAL_HOST
                        || v4Ip4()==IP4_ANY;

      case AF_INET6:
      {
         if(IN6_IS_ADDR_LOOPBACK   (&v6().sin6_addr)
         || IN6_IS_ADDR_UNSPECIFIED(&v6().sin6_addr))return true;

         if(IN6_IS_ADDR_V4MAPPED   (&v6().sin6_addr))
         {
            return v6Ip4()==IP4_LOCAL_HOST
            ||     v6Ip4()==IP4_ANY;
         }
      }break;
   }
   return false;
}
/******************************************************************************/
SockAddr& SockAddr::clear()
{
   Zero(T); return T;
}
/******************************************************************************/
Int SockAddr::port()C
{
   return ntohs(portN());
}
SockAddr& SockAddr::port(Int port)
{
   portN()=htons(port);
   return T;
}
/******************************************************************************/
UInt SockAddr::ip4()C
{
   switch(family())
   {
      case AF_INET : return v4Ip4();
      case AF_INET6:
      {
         if(IN6_IS_ADDR_V4MAPPED   (&v6().sin6_addr))return v6Ip4();
         if(IN6_IS_ADDR_LOOPBACK   (&v6().sin6_addr))return IP4_LOCAL_HOST;
       //if(IN6_IS_ADDR_UNSPECIFIED(&v6().sin6_addr))return IP4_ANY; IP4_ANY is 0 which is returned either way below
      }break;
   }
   return 0;
}
SockAddr& SockAddr::ip4(UInt ip4)
{
   Int port_n=portN(); // we're doing a direct copy so 'ntohs' is not needed
   clear();
   portN()=port_n; // we're doing a direct copy so 'htons' is not needed
#if FORCE_IPV6
   #if APPLE
      v6().sin6_len=SIZE(sockaddr_in6);
   #endif
   v6setAfterClearV4Mapped();
   v6Ip4 ()=ip4;
   family()=AF_INET6; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#else
   #if APPLE
      v4().sin_len=SIZE(sockaddr_in);
   #endif
   v4Ip4 ()=ip4;
   family()=AF_INET; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#endif
   return T;
}

Bool SockAddr::ip4Text(C Str8 &ip4)
{
   Int port_n=portN(); // we're doing a direct copy so 'ntohs' is not needed
   clear();
   portN()=port_n; // always keep port, we're doing a direct copy so 'htons' is not needed
#if FORCE_IPV6
   if(TextToIP4(ip4, v6Ip4()))
   {
   #if APPLE
      v6().sin6_len=SIZE(sockaddr_in6);
   #endif
      v6setAfterClearV4Mapped();
      family()=AF_INET6; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#else
   if(TextToIP4(ip4, v4Ip4()))
   {
   #if APPLE
      v4().sin_len=SIZE(sockaddr_in);
   #endif
      family()=AF_INET; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#endif
      return true;
   }
   return false;
}
Bool SockAddr::ip6Text(C Str8 &ip6)
{
   Int port_n=portN(); // we're doing a direct copy so 'ntohs' is not needed
   clear();
   portN()=port_n; // always keep port, we're doing a direct copy so 'htons' is not needed
#if WINDOWS_OLD && SUPPORT_WINDOWS_XP // instead of calling manually written 'inet_pton' because of Windows XP, just call 'WSAStringToAddressA' directly
   sockaddr_in6 temp;
   int          size=SIZE(temp);
   if(WSAStringToAddressA(ConstCast(ip6()), AF_INET6, null, (sockaddr*)&temp, &size)==0) // Warning: this assumes that 'WSAStringToAddressA' will not modify the 'ip6'
   {
      v6().sin6_addr=temp.sin6_addr; // copy only 'addr', without the 'port' and 'scope' from 'temp' to match 'inet_pton'
      family()=AF_INET6; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
      return true;
   }
#else
   if(inet_pton(AF_INET6, ip6, &v6().sin6_addr)==1)
   {
   #if APPLE
      v6().sin6_len=SIZE(sockaddr_in6);
   #endif
      family()=AF_INET6; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
      return true;
   }
#endif
   return false;
}
Bool SockAddr::ipText(C Str8 &ip)
{
   return ip4Text(ip) || ip6Text(ip);
}

Str8 SockAddr::ip4Text()C
{
   switch(family())
   {
      case AF_INET:
      {
         VecB4 v; v.u=v4Ip4(); return v.asTextDots();
      }break;

      case AF_INET6:
      {
         VecB4 v;
         if(IN6_IS_ADDR_V4MAPPED   (&v6().sin6_addr))v.u=v6Ip4()       ;else
         if(IN6_IS_ADDR_LOOPBACK   (&v6().sin6_addr))v.u=IP4_LOCAL_HOST;else
         if(IN6_IS_ADDR_UNSPECIFIED(&v6().sin6_addr))v.u=IP4_ANY       ;else break;
         return v.asTextDots();
      }break;
   }
   return S;
}
Str8 SockAddr::ip6Text()C
{
   switch(family())
   {
      case AF_INET:
      {
         VecB4 v; v.u=v4Ip4(); return S+"::ffff:"+v.asTextDots();
      }break;

      case AF_INET6:
      {
         Char8 text[INET6_ADDRSTRLEN];
      #if WINDOWS_OLD && SUPPORT_WINDOWS_XP // instead of calling manually written 'inet_ntop' because of Windows XP, just call 'WSAAddressToStringA' directly
         sockaddr_in6 temp; Zero(temp); // we need to set only 'family' and 'addr', because otherwise, 'scope' and 'port' can be included in the name
         temp.sin6_family=AF_INET6;
         temp.sin6_addr  =v6().sin6_addr;
         DWORD length=Elms(text); return (WSAAddressToStringA((sockaddr*)&temp, SIZE(temp), null, text, &length)==0) ? text : null;
      #else
         return inet_ntop(AF_INET6, (Ptr)&v6().sin6_addr, text, Elms(text));
      #endif
      }break;
   }
   return S;
}
Str8 SockAddr::ipText()C
{
   switch(family())
   {
      case AF_INET :                    ip4: return ip4Text();
      case AF_INET6: if(!needsV6())goto ip4; return ip6Text();
   }
   return S;
}
/******************************************************************************/
Bool SockAddr::setFrom(C Socket &socket)
{
   socklen_t size=SIZE(_data);
   return getsockname((SOCKET)socket._s, (sockaddr*)&_data, &size)!=SOCKET_ERROR;
}
SockAddr& SockAddr::setLocal(Int port)
{
#if 1 // prefer IPv4, this function is most often used for displaying address of the local device, so it can be used on other devices for connecting to it. Prefer IPv4 as they're shorter and easier to type. As it was tested on Jul 2015, using 3 laptops: Windows 7, Windows 8.1, Mac OS X 10.10. When using 'GetHostAddresses' for getting local addresses of other devices, all devices displayed IPv4, as for IPv6 only Win8 displayed address for Win7 laptop (it connected fine to that address). However when trying to connect via manually typed IPv6 addresses obtained from 'GetLocalAddresses': Win laptops connected through only half of the IPv6 addresses, and Mac failed to connect to all Win IPv6 with EHOSTUNREACH error.
   Memt<SockAddr> addrs; GetHostAddresses(addrs, ComputerName, port);
   FREPA(addrs)if(addrs[i].family()==AF_INET){Swap(T, addrs[i]); return T;} // look for IPv4 first
   if(addrs.elms())Swap(T, addrs.first());else clear(); // grab the first result as it is recommended to process results in order
#else
   setHost(ComputerName, port);
#endif
   return T;
}
Bool SockAddr::setHost(C Str &host, Int port)
{
   Memt<SockAddr> addrs; GetHostAddresses(addrs, host, port);
   if(addrs.elms())
   {
      Swap(T, addrs.first()); // grab the first result as it is recommended to process results in order
      return true;
   }
   clear(); return false;
}
SockAddr& SockAddr::setServer(Int port)
{
   clear();
   portN()=htons(port);
#if 1 // prefer IPv6 to avoid calling 'convert'
   #if APPLE
      v6().sin6_len=SIZE(sockaddr_in6);
   #endif
   v6setAfterClearAny();
   family()=AF_INET6; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#else
   #if APPLE
      v4().sin_len=SIZE(sockaddr_in);
   #endif
   v4Ip4 ()=IP4_ANY;
   family()=AF_INET; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#endif
   return T;
}
SockAddr& SockAddr::setLocalFast(Int port)
{
   clear();
   portN()=htons(port);
#if 1 // prefer IPv6 to avoid calling 'convert'
   #if APPLE
      v6().sin6_len=SIZE(sockaddr_in6);
   #endif
   v6setAfterClearLocalHost();
   family()=AF_INET6; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#else
   #if APPLE
      v4().sin_len=SIZE(sockaddr_in);
   #endif
   v4Ip4 ()=IP4_LOCAL_HOST;
   family()=AF_INET; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#endif
   return T;
}
Bool SockAddr::setGlobal(Int port)
{
   if(!GIPC.valid()){GIPC.start(); GIPC.wait();}
   if( GIPC.valid()){T=GIPC.addr; T.port(port); return true;}
   clear(); return false;
}
SockAddr& SockAddr::setIp4Port(UInt ip4, Int port)
{
   clear();
   portN()=htons(port);
#if FORCE_IPV6
   #if APPLE
      v6().sin6_len=SIZE(sockaddr_in6);
   #endif
   v6setAfterClearV4Mapped();
   v6Ip4 ()=ip4;
   family()=AF_INET6; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#else
   #if APPLE
      v4().sin_len=SIZE(sockaddr_in);
   #endif
   v4Ip4 ()=ip4;
   family()=AF_INET; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#endif
   return T;
}
Bool SockAddr::setIp4Port(C Str8 &ip4, Int port)
{
   clear();
#if FORCE_IPV6
   if(TextToIP4(ip4, v6Ip4()))
   {
   #if APPLE
      v6().sin6_len=SIZE(sockaddr_in6);
   #endif
      v6setAfterClearV4Mapped();
      portN ()=htons(port);
      family()=AF_INET6; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#else
   if(TextToIP4(ip4, v4Ip4()))
   {
   #if APPLE
      v4().sin_len=SIZE(sockaddr_in);
   #endif
      portN ()=htons(port);
      family()=AF_INET; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
#endif
      return true;
   }
   return false;
}
Bool SockAddr::setIp6Port(C Str8 &ip6, Int port)
{
   if(ip6Text(ip6))
   {
      portN()=htons(port);
      return true;
   }
   clear(); return false;
}
Bool SockAddr::setIpPort(C Str8 &ip, Int port)
{
   if(ipText(ip))
   {
      portN()=htons(port);
      return true;
   }
   clear(); return false;
}
/******************************************************************************/
Str SockAddr::asText()C
{
   switch(family())
   {
      case AF_INET :                    ip4: return       ip4Text()+ ':'+port();
      case AF_INET6: if(!needsV6())goto ip4; return S+'['+ip6Text()+"]:"+port();
   }
   return S;
}
Bool SockAddr::fromText(C Str8 &ip_port)
{
   if(CChar8 *addr=_SkipStartPath(_SkipStartPath(ip_port, "http://"), "https://"))
   {
      if(*addr=='[') // try IPv6
      {
         Int pos =TextPosI(addr, ']');
         if( pos>=0 && addr[pos+1]==':')
         {
            Int port=TextInt(addr+pos+2); if(InRange(port, 0x10000))return setIp6Port(Str8(addr+1).clip(pos-1), port);
         }
      }else // try IPv4 + Name
      {
         Int pos =TextPosI(addr, ':');
         if( pos>=0)
         {
            Int port=TextInt(addr+pos+1); if(InRange(port, 0x10000))
            {
               Str8 a=addr; a.clip(pos);
               return setIp4Port(a, port) || setHost(a, port);
            }
         }
      }
   }
   clear(); return false;
}
/******************************************************************************/
Bool SockAddr::convert(C SockAddr &src)
{
   switch(src.family())
   {
      case AF_INET:
      {
         // copy first in case "&src==this"
         Int port_n=src.portN(); // we're doing a direct copy so 'ntohs' is not needed
         UInt   ip4=src.v4Ip4();
         clear();
      #if APPLE
         v6().sin6_len=SIZE(sockaddr_in6);
      #endif
         switch(ip4)
         {
            case IP4_ANY       : v6setAfterClearAny      ();              break; // handle special cases in case dual-stack isn't supported
            case IP4_LOCAL_HOST: v6setAfterClearLocalHost();              break; // handle special cases in case dual-stack isn't supported
            default            : v6setAfterClearV4Mapped (); v6Ip4()=ip4; break; // set v4-mapped
         }
         portN ()=port_n; // we're doing a direct copy so 'htons' is not needed
         family()=AF_INET6; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
      }return true;

      case AF_INET6:
      {
         // copy first in case "&src==this"
         Int  port_n=src.portN(); // we're doing a direct copy so 'ntohs' is not needed
         UInt ip4;
         if(IN6_IS_ADDR_V4MAPPED   (&src.v6().sin6_addr))ip4=src.v6Ip4()   ;else
         if(IN6_IS_ADDR_LOOPBACK   (&src.v6().sin6_addr))ip4=IP4_LOCAL_HOST;else
         if(IN6_IS_ADDR_UNSPECIFIED(&src.v6().sin6_addr))ip4=IP4_ANY       ;else break;
         clear();
      #if APPLE
         v4().sin_len=SIZE(sockaddr_in);
      #endif
         v4Ip4 ()=ip4;
         portN ()=port_n; // we're doing a direct copy so 'htons' is not needed
         family()=AF_INET; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
      }return true;
   }
   return false;
}
/******************************************************************************/
Int Compare(C SockAddr &a, C SockAddr &b)
{
   if(Int c=Compare(a.family(), b.family()))return c;
   switch(a.family())
   {
      case AF_INET : if(Int c=Compare(a.v4Ip4(), b.v4Ip4()))return c; break;
      case AF_INET6: if(Int c=CompareV6((UInt*)&a.v6().sin6_addr, (UInt*)&b.v6().sin6_addr))return c; break;
   }
   return Compare(a.port(), b.port());
}
/******************************************************************************/
Bool SockAddr::save(File &f)C
{
   switch(family()) // we can use 'putByte' because we know that these versions will fit in one byte
   {
      default      :               f.putByte (     0); break;
      case AF_INET :               f.putMulti(Byte(1), UInt(v4Ip4()), U16(port())); break;
      case AF_INET6: if(!needsV6())f.putMulti(Byte(1), UInt(  ip4()), U16(port()));
                     else          f.putMulti(Byte(2),                U16(port()))<<v6().sin6_addr; break;
   }
   return f.ok();
}
Bool SockAddr::load(File &f)
{
   switch(f.decUIntV())
   {
      case 0: clear(); return f.ok();

      case 1:
      {
         UInt ip; U16 port; f.getMulti(ip, port);
         if(f.ok()){setIp4Port(ip, port); return true;}
      }break;

      case 2:
      {
         clear();
         port(f.getUShort());
      #if APPLE
         v6().sin6_len=SIZE(sockaddr_in6);
      #endif
         f>>v6().sin6_addr;
         family()=AF_INET6; // !! set family as last for thread-safety, because some methods (including 'GIPC') detect if address is valid based on family !!
         if(f.ok())return true;
      }break;
   }
   clear(); return false;
}
/******************************************************************************/
// SOCKET
/******************************************************************************/
void Socket::init(Bool ipv6)
{
#if APPLE
   int val=1; setsockopt((SOCKET)_s, SOL_SOCKET, SO_NOSIGPIPE, &val, SIZE(val)); // disable SIGPIPE process signal on Unix machines (it is received when writing to closed socket, and it causes process termination)
#endif
   if(ipv6 && DualStackSocket) // don't bother if it's not supported
   {
      int val=0; setsockopt((SOCKET)_s, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&val, SIZE(val)); // this allows binding to IPv4-mapped IPv6
   }
}
void Socket::del()
{
   if(is())
   {
   #if WINDOWS
      closesocket((SOCKET)_s);
   #else
      close((SOCKET)_s);
   #endif
     _s=NULL_SOCKET;
   }
}
Bool Socket::createTcp(Bool ipv6) {del(); _s=(CPtr)socket(ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP); if(is()){init(ipv6); return true;} return false;}
Bool Socket::createUdp(Bool ipv6) {del(); _s=(CPtr)socket(ipv6 ? AF_INET6 : AF_INET, SOCK_DGRAM , IPPROTO_UDP); if(is()){init(ipv6); return true;} return false;}

// if we support 'DualStackSocket' then always create IPv6, because IPv6 with 'DualStackSocket' can support both modes
// otherwise the address can be v4 only - use IPv4, v6 only - use IPv6, or universal (LOCAL_HOST / ANY) - use IPv4
// this was tested: ConnectionServer was created with IPv6, Connection was created with IPv4 and tried to connect to a IPv4 address of the server, that failed. Result: Server needs to have IPv4 when 'DualStackSocket' is not supported.
Bool Socket::createTcp(C SockAddr &addr) {return createTcp(DualStackSocket || addr.needsV6());}
Bool Socket::createUdp(C SockAddr &addr) {return createUdp(DualStackSocket || addr.needsV6());}
/******************************************************************************/
Int      Socket::port()C {SockAddr addr; addr.setFrom(T); return addr.port();}
UInt     Socket::ip4 ()C {SockAddr addr; addr.setFrom(T); return addr.ip4 ();}
SockAddr Socket::addr()C {SockAddr addr; addr.setFrom(T); return addr       ;}
/******************************************************************************/
Bool Socket::block(Bool on)
{
   if(is())
   {
   #if WEB
      return !on; // web sockets are always non-blocking, this can't be changed, so return success only if non-blocking mode was requested
   #elif WINDOWS
      DWORD non_block=(on ? false : true); return ioctlsocket((SOCKET)_s, FIONBIO, &non_block)!=SOCKET_ERROR;
   #else
      UInt  non_block=(on ? false : true); return ioctl      ((SOCKET)_s, FIONBIO, &non_block)!=SOCKET_ERROR; // works better than : int flags=fcntl((SOCKET)_s, F_GETFL, 0); FlagSet(flags, O_NONBLOCK, !on); return fcntl((SOCKET)_s, F_SETFL, flags)!=SOCKET_ERROR;
   #endif
   }
   return false;
}
Bool Socket::tcpNoDelay(Bool on)
{
   if(is())
   {
      int flag=on;
      return !setsockopt((SOCKET)_s, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, SIZE(flag));
   }
   return false;
}
/******************************************************************************/
Socket::RESULT Socket::connect(C SockAddr &addr, Int timeout) // !! warning: this sets non-blocking mode when timeout is specified, but does not restore the blocking mode after !!
{
   if(timeout>=0)block(false);
#if !WINDOWS
again:
#endif
   if(::connect((SOCKET)_s, (C sockaddr*)&addr._data, addr.size())!=SOCKET_ERROR)return CONNECTED;
   switch(PLATFORM(WSAGetLastError(), errno)) // don't use 'Socket::WouldBlock' here because it's not meant for 'connect'
   {
      case PLATFORM(WSAEWOULDBLOCK, EINPROGRESS): // according to docs only these enums can occur for IN_PROGRESS, Win - https://msdn.microsoft.com/en-us/library/windows/desktop/ms737625(v=vs.85).aspx Mac - http://www.manpages.info/macosx/connect.2.html Linux - http://man7.org/linux/man-pages/man2/connect.2.html
      {
      in_progress:
         if(any(timeout)    )return CONNECTED;
         if(!connectFailed())return IN_PROGRESS;
      }break;

 /*#if WINDOWS
      case WSAECONNREFUSED: return REFUSED;
   #endif*/

   #if !WINDOWS
      case EINTR: goto again; // call was interrupted by a signal
   #endif

      case PLATFORM(WSAEAFNOSUPPORT, EAFNOSUPPORT):
   #if !WINDOWS
      case EINVAL: // this error can happen on Android (Lollipop 5.0) and Linux (Ubuntu 14.10)
      case EPERM : // this error can happen on iOS 9
   #endif
      {
         SockAddr temp; if(temp.convert(addr))
         {
         #if !WINDOWS
         again_2:
         #endif
            if(::connect((SOCKET)_s, (C sockaddr*)&temp._data, temp.size())!=SOCKET_ERROR)return CONNECTED;
            switch(PLATFORM(WSAGetLastError(), errno)) // don't use 'Socket::WouldBlock' here because it's not meant for 'connect'
            {
               case PLATFORM(WSAEWOULDBLOCK, EINPROGRESS): goto in_progress;
            #if !WINDOWS
               case EINTR: goto again_2; // call was interrupted by a signal
            #endif
            }
         }
      }break;

   #if LOG_ERROR
      default: LogN(S+"Socket.connect failed:"+PLATFORM(WSAGetLastError(), errno)); break;
   #endif
   }
   return FAILED;
}
Bool Socket::connectFailed()
{
#if !WEB
   if(is())
   {
      Int error=0; socklen_t size=SIZE(error);
      if(getsockopt((SOCKET)_s, SOL_SOCKET, SO_ERROR, (char*)&error, &size)!=SOCKET_ERROR)return error!=0;
   }
#endif
   return false;
}
Bool Socket::bind(C SockAddr &addr)
{
   if(::bind((SOCKET)_s, (C sockaddr*)&addr._data, addr.size())!=SOCKET_ERROR)return true;
   switch(PLATFORM(WSAGetLastError(), errno))
   {
   #if !WINDOWS    // leave these extra checks just in case
      case EINVAL: // this error can happen on Android (Lollipop 5.0) and Linux (Ubuntu 14.10)
      case EPERM : // this error can happen on iOS 9
   #endif
      case PLATFORM(WSAEAFNOSUPPORT, EAFNOSUPPORT):
      {
         SockAddr temp; if(temp.convert(addr))return ::bind((SOCKET)_s, (C sockaddr*)&temp._data, temp.size())!=SOCKET_ERROR;
      }break;

   #if LOG_ERROR
      default: LogN(S+"Socket.bind:"+PLATFORM(WSAGetLastError(), errno)); break;
   #endif
   }
   return false;
}
Bool Socket::listen(                                  ) {return ::listen((SOCKET)_s, SOMAXCONN)!=SOCKET_ERROR;}
Bool Socket::accept(Socket &connection, SockAddr &addr)
{
   connection.del(); addr.clear();
   if(is())
   {
      socklen_t size=SIZE(addr._data); connection._s=(CPtr)::accept((SOCKET)_s, (sockaddr*)&addr._data, &size);
      if(connection.is())
      {
      	connection.init(false); // don't adjust IPV6_V6ONLY because the socket is already connected to an address and can't be adjusted afterwards, doing that would fail and cause an error
         return true;
      }
   }
   return false;
}
/******************************************************************************/
#if !WINDOWS && !APPLE
   #define SEND_FLAGS MSG_NOSIGNAL // use MSG_NOSIGNAL to disable SIGPIPE process signal when writing to closed socket
#else
   #define SEND_FLAGS 0
#endif
Int Socket::send   (                  CPtr data, Int size) {                                      return ::send    ((SOCKET)_s, (Char8*)data, size, SEND_FLAGS);}
Int Socket::receive(                   Ptr data, Int size) {                                      return ::recv    ((SOCKET)_s, (Char8*)data, size,          0);}
Int Socket::receive(  SockAddr &addr,  Ptr data, Int size) {socklen_t addr_size=SIZE(addr._data); return ::recvfrom((SOCKET)_s, (Char8*)data, size,          0, (sockaddr*)&addr._data, &addr_size);}
Int Socket::send   (C SockAddr &addr, CPtr data, Int size)
{
   Int sent=::sendto((SOCKET)_s, (Char8*)data, size, SEND_FLAGS, (C sockaddr*)&addr._data, addr.size());
   if( sent<0)switch(PLATFORM(WSAGetLastError(), errno))
   {
   #if !WINDOWS    // leave these extra checks just in case
      case EINVAL: // this error can happen on Android (Lollipop 5.0) and Linux (Ubuntu 14.10)
      case EPERM : // this error can happen on iOS 9
   #endif
      case PLATFORM(WSAEAFNOSUPPORT, EAFNOSUPPORT):
      {
         SockAddr temp; if(temp.convert(addr))sent=::sendto((SOCKET)_s, (Char8*)data, size, SEND_FLAGS, (C sockaddr*)&temp._data, temp.size());
      }break;

   #if LOG_ERROR
      default: LogN(S+"Socket.send failed:"+PLATFORM(WSAGetLastError(), errno)); break;
   #endif
   }
   return sent;
}
Bool Socket::WouldBlock()
{
   switch(PLATFORM(WSAGetLastError(), errno))
   {
      case PLATFORM(WSAEWOULDBLOCK, EWOULDBLOCK):
   #if !WINDOWS && EAGAIN!=EWOULDBLOCK
      case EAGAIN:
   #endif
   #if !WINDOWS
      case EINTR: // call was interrupted by a signal
   #endif
         return true;

   #if LOG_ERROR
      default: LogN(S+"Socket failed:"+PLATFORM(WSAGetLastError(), errno)); break;
   #endif
   }
   return false;
}
/******************************************************************************/
Bool Socket::select(Bool read, Bool write, Int time)
{
   if(is())
   {
      fd_set fd;
   #if WINDOWS
      fd.fd_count=1; fd.fd_array[0]=SOCKET(_s);
   #else
      FD_ZERO(&fd); FD_SET(SOCKET(_s), &fd);
   #endif
      timeval tv; if(time>0){tv.tv_sec=time/1000; tv.tv_usec=(time%1000)*1000;}else tv.tv_sec=tv.tv_usec=0;

      return ::select(SOCKET(_s)+1, read ? &fd : null, write ? &fd : null, null, &tv)>0;
   }
   return false;
}
Bool Socket::wait (Int time) {return select( true, false, time);}
Bool Socket::flush(Int time) {return select(false,  true, time);}
Bool Socket::any  (Int time) {return select( true,  true, time);}

Int Socket::available()C
{
   if(is())
   {
   #if WINDOWS
      DWORD size; if(ioctlsocket((SOCKET)_s, FIONREAD, &size)!=SOCKET_ERROR)return size;
   #else
      UInt  size; if(ioctl      ((SOCKET)_s, FIONREAD, &size)!=SOCKET_ERROR)return size;
   #endif
   }
   return -1;
}
/******************************************************************************/
// MAIN
/******************************************************************************/
static Bool ConnectSMTPIPv6Failed;
static Bool ConnectSMTP(Socket &socket)
{
   SockAddr addr; addr.setLocalFast(25); // 25 is the default port for SMTP servers
   if(DualStackSocket && !ConnectSMTPIPv6Failed) // try IPv6 first if dual stack sockets are supported, and only if it didn't fail before (IPv6 can fail with WSAECONNREFUSED for some reason on Windows Server 2012, while IPv4 succeeded. However WSAECONNREFUSED is returned as well for Windows 10 which has no SMTP at all)
   {
      if(socket.createTcp(true))switch(socket.connect(addr))
      {
         case Socket::CONNECTED: return true;
         default               : ConnectSMTPIPv6Failed=true; break;
      }
   }
   return socket.createTcp(false) && socket.connect(addr)==Socket::CONNECTED; // try IPv4 later
}
Bool SendMailSupported()
{
   Socket sock; return ConnectSMTP(sock);
}
Bool SendMail(C Str &from_name, C Str &from_email, C Str &to_name, C Str &to_email, C Str &subject, C Str &body)
{
   Socket sock; if(ConnectSMTP(sock))
   {
            Byte buf[64*1024];
            Str8 m;
      const Bool log=false;
                                                                      if(log)Zero(buf); sock.receive(buf, SIZE(buf)); if(log)LogN(S+(char*)buf);
      m=  "HELO SendMail\r\n"           ; sock.send(m(), m.length()); if(log)Zero(buf); sock.receive(buf, SIZE(buf)); if(log)LogN(S+(char*)buf);
      m=S+"MAIL FROM:"+from_email+"\r\n"; sock.send(m(), m.length()); if(log)Zero(buf); sock.receive(buf, SIZE(buf)); if(log)LogN(S+(char*)buf);
      m=S+"RCPT TO:"  +  to_email+"\r\n"; sock.send(m(), m.length()); if(log)Zero(buf); sock.receive(buf, SIZE(buf)); if(log)LogN(S+(char*)buf);
      m=  "DATA\r\n"                    ; sock.send(m(), m.length()); if(log)Zero(buf); sock.receive(buf, SIZE(buf)); if(log)LogN(S+(char*)buf);
      m=(from_name.is() ? S+   "FROM:"+from_name+" <"+from_email+">\r\n" : S)
       +(  to_name.is() ? S+     "TO:"+  to_name+" <"+  to_email+">\r\n" : S)
                           +"SUBJECT:"+  subject+"\r\n"
                           +             body   +"\r\n"
                           +                    ".\r\n"; sock.send(m(), m.length()); if(log)Zero(buf); sock.receive(buf, SIZE(buf)); if(log)LogN(S+(char*)buf); Bool ok=(buf[0]=='2');
      m="QUIT\r\n"                                     ; sock.send(m(), m.length()); if(log)Zero(buf); sock.receive(buf, SIZE(buf)); if(log)LogN(S+(char*)buf);
      return ok;
   }
   return false;
}
/******************************************************************************/
Bool   GetDualStackSocket() {return DualStackSocket;}
ULong  GetMac            () {return Mac;}
C Str& GetComputerName   () {return ComputerName;}
/******************************************************************************/
void GetLocalAddresses(MemPtr<SockAddr> addresses,              Int port) {return GetHostAddresses(addresses, ComputerName, port);}
void GetHostAddresses (MemPtr<SockAddr> addresses, C Str &host, Int port)
{
   addresses.clear();
   const Bool allow_local_host=true; // allow local host as it's actually needed if for example we want to use 'Download' (which uses this function) to connect to "localhost"

#if WINDOWS
   // 'GetAddrInfoW' requires at least Windows XP with SP2
   ADDRINFOW hints; Zero(hints);
   hints.ai_family  =AF_UNSPEC;
   hints.ai_socktype=SOCK_STREAM;
   hints.ai_protocol=IPPROTO_TCP;

   ADDRINFOW *result=null; if(!GetAddrInfoW(host, null, &hints, &result))
   {
      for(ADDRINFOW *r=result; r; r=r->ai_next)switch(r->ai_family)
      {
         case AF_INET: if(r->ai_addr && r->ai_addrlen<=MEMBER_SIZE(SockAddr, _data))
         {
          C sockaddr_in &addr=*(sockaddr_in*)r->ai_addr;
            if(allow_local_host || addr.sin_addr.s_addr!=IP4_LOCAL_HOST){SockAddr &sa=addresses.New(); sa.v4()=addr; sa.port(port);}
         }break;

         case AF_INET6: if(r->ai_addr && r->ai_addrlen<=MEMBER_SIZE(SockAddr, _data))
         {
          C sockaddr_in6 &addr=*(sockaddr_in6*)r->ai_addr;
            if(allow_local_host || !IN6_IS_ADDR_LOOPBACK(&addr.sin6_addr)){SockAddr &sa=addresses.New(); sa.v6()=addr; sa.port(port);}
         }break;
      }
      FreeAddrInfoW(result);
   }
#else
   if(Equal(host, ComputerName, true)) // try using dedicated methods for local host for more precision
   {
   #if LINUX
      ifaddrs *interfaces=null; if(!getifaddrs(&interfaces))
      {
         for(ifaddrs *r=interfaces; r; r=r->ifa_next)if(r->ifa_addr)switch(r->ifa_addr->sa_family)
         {
            case AF_INET:
            {
             C sockaddr_in &addr=*(sockaddr_in*)r->ifa_addr;
               if(allow_local_host || addr.sin_addr.s_addr!=IP4_LOCAL_HOST){SockAddr &sa=addresses.New(); sa.v4()=addr; sa.port(port);}
            }break;

            case AF_INET6:
            {
             C sockaddr_in6 &addr=*(sockaddr_in6*)r->ifa_addr;
               if(allow_local_host || !IN6_IS_ADDR_LOOPBACK(&addr.sin6_addr)){SockAddr &sa=addresses.New(); sa.v6()=addr; sa.port(port);}
            }break;
         }
         freeifaddrs(interfaces);
      }
   #elif APPLE
      #if 0 // don't use as 'NSHost' is private API on the iOS?
         if(NSHost *host=[NSHost currentHost])
         {
          //if(NSString *addr=[host address]) // this may be IP6, [addr release]; don't release as it causes crashes
            if(NSArray *addrs=[host addresses])
	         {
	            REP([addrs count])
	            {
	               Str addr=[addrs objectAtIndex:i];
	               if( addr.length()<=4*3 + 3) // 4*"255" + 3*'.'
	               {
	                  Int dots=0; REPA(addr)if(addr[i]=='.')dots++;
	                  if( dots==3 && (allow_local_host || addr!="127.0.0.1"))addresses.New().setIP(addr, port);
	               }
	            }
	            //[addrs release]; don't release as it causes crashes
	         }
	         //[host release]; don't release as it causes crashes
	      }
	   #else
         ifaddrs *interfaces=null; if(!getifaddrs(&interfaces))
         {
            for(ifaddrs *r=interfaces; r; r=r->ifa_next)if(r->ifa_addr)switch(r->ifa_addr->sa_family)
            {
               case AF_INET:
               {
                C sockaddr_in &addr=*(sockaddr_in*)r->ifa_addr;
                  if(allow_local_host || addr.sin_addr.s_addr!=IP4_LOCAL_HOST){SockAddr &sa=addresses.New(); sa.v4()=addr; sa.port(port);}
               }break;

               case AF_INET6:
               {
                C sockaddr_in6 &addr=*(sockaddr_in6*)r->ifa_addr;
                  if(allow_local_host || !IN6_IS_ADDR_LOOPBACK(&addr.sin6_addr)){SockAddr &sa=addresses.New(); sa.v6()=addr; sa.port(port);}
               }break;
            }
            freeifaddrs(interfaces);
         }
      #endif
   #elif ANDROID
      #if 0
         Java
         WifiManager wifiManager = (WifiManager)getSystemService(WIFI_SERVICE);
         WifiInfo wifiInfo = wifiManager.getConnectionInfo();
         int ipAddress = wifiInfo.getIpAddress();
      #else
         Socket sock; if(sock.createUdp(true)) // prefer IPv6 because it's better
         {
            ifreq  buffer[32];
            ifconf ifc;
            ifc.ifc_len=SIZE(buffer);
            ifc.ifc_req=buffer;

            if(ioctl((SOCKET)sock._s, SIOCGIFCONF, &ifc)!=SOCKET_ERROR)
            {
               FREP(ifc.ifc_len/SIZE(ifreq))
               {
                  ifreq &ifr=buffer[i];
                  switch(ifr.ifr_addr.sa_family)
                  {
                     case AF_INET:
                     {
                      C sockaddr_in &addr=(sockaddr_in&)ifr.ifr_addr;
                        if(allow_local_host || addr.sin_addr.s_addr!=IP4_LOCAL_HOST){SockAddr &sa=addresses.New(); sa.v4()=addr; sa.port(port);}
                     }break;

                     case AF_INET6:
                     {
                      C sockaddr_in6 &addr=(sockaddr_in6&)ifr.ifr_addr;
                        if(allow_local_host || !IN6_IS_ADDR_LOOPBACK(&addr.sin6_addr)){SockAddr &sa=addresses.New(); sa.v6()=addr; sa.port(port);}
                     }break;
                  }
               }
            }
         }
      #endif
   #endif
   }

   if(!addresses.elms())
   {
      addrinfo hints; Zero(hints);
      hints.ai_family  =AF_UNSPEC;
      hints.ai_socktype=SOCK_STREAM;
      hints.ai_protocol=IPPROTO_TCP;

      addrinfo *result=null; if(!getaddrinfo(Str8(host), null, &hints, &result))
      {
         for(addrinfo *r=result; r; r=r->ai_next)switch(r->ai_family)
         {
            case AF_INET: if(r->ai_addr && r->ai_addrlen<=MEMBER_SIZE(SockAddr, _data))
            {
             C sockaddr_in &addr=*(sockaddr_in*)r->ai_addr;
               if(allow_local_host || addr.sin_addr.s_addr!=IP4_LOCAL_HOST){SockAddr &sa=addresses.New(); sa.v4()=addr; sa.port(port);}
            }break;

            case AF_INET6: if(r->ai_addr && r->ai_addrlen<=MEMBER_SIZE(SockAddr, _data))
            {
             C sockaddr_in6 &addr=*(sockaddr_in6*)r->ai_addr;
               if(allow_local_host || !IN6_IS_ADDR_LOOPBACK(&addr.sin6_addr)){SockAddr &sa=addresses.New(); sa.v6()=addr; sa.port(port);}
            }break;
         }
         freeaddrinfo(result);
      }
   }
#endif
   if(!addresses.elms() && Equal(host, ComputerName, true))addresses.New().setLocalFast(port);
}
/******************************************************************************/
void       GetGlobalIP(Bool refresh) {if(!GIPC.valid() || refresh)GIPC.start();}
Bool       HasGlobalIP(            ) {return GIPC.valid();}
Bool ObtainingGlobalIP(            ) {return GIPC.busy   ;}
/******************************************************************************/
static Bool InitSocketEx()
{
   if(LogInit)LogN("InitSocket");

#if WINDOWS
   if(WSAStartup(MAKEWORD(2, 2), &WsaData)==NO_ERROR) // init WinSock
   {
      // get mac address
      {
      #if WINDOWS_OLD
         Memt<Byte> addresses; ULONG size=0;
         if(GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_UNICAST|GAA_FLAG_SKIP_ANYCAST|GAA_FLAG_SKIP_MULTICAST|GAA_FLAG_SKIP_DNS_SERVER|GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER, null, (IP_ADAPTER_ADDRESSES*)addresses.data(), &size)==ERROR_BUFFER_OVERFLOW)
         {
            addresses.setNum(size);
            if(GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_UNICAST|GAA_FLAG_SKIP_ANYCAST|GAA_FLAG_SKIP_MULTICAST|GAA_FLAG_SKIP_DNS_SERVER|GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER, null, (IP_ADAPTER_ADDRESSES*)addresses.data(), &size)==ERROR_SUCCESS)
               for(IP_ADAPTER_ADDRESSES *adapter=(IP_ADAPTER_ADDRESSES*)addresses.data(); adapter; adapter=adapter->Next)
            {
   /*
   On a Windows Laptop there were following adapters found:
   1. MIB_IF_TYPE_ETHERNET "Qualcomm Atheros AR8151 PCI-E Gigabit Ethernet Controller (NDIS 6.30)"
   2. IF_TYPE_IEEE80211    "Microsoft Wi-Fi Direct Virtual Adapter"
   3. IF_TYPE_IEEE80211    "Intel(R) Centrino(R) Advanced-N 6235"
   */
               if(adapter->IfType==IF_TYPE_IEEE80211
               || adapter->IfType==MIB_IF_TYPE_ETHERNET)
                  CopyFast(&Mac, adapter->PhysicalAddress, Min(SIZEU(Mac), (UInt)adapter->PhysicalAddressLength));
            }
         }
      #else
         // TODO: WINDOWS_NEW get MAC address, it should support 'GetAdaptersAddresses', however there are compilation errors as of right now, so try #include <IPHlpApi.h> in the future again
      #endif
      }

      // get host name
      {
      #if 1 // better
         wchar_t host[MAX_COMPUTERNAME_LENGTH+1]; DWORD size=Elms(host);
         if(GetComputerNameW(host, &size))ComputerName=host;
      #else // doesn't work if "hosts" file is modified
         sockaddr_in addr; Zero(addr); // this also clears 'port'
         addr.sin_family     =AF_INET;
         addr.sin_addr.s_addr=IP4_LOCAL_HOST;
       //addr.sin_port       =htons(port); use zero port, we don't need to set this since 'Zero' is called above

         // 'GetNameInfoW' requires at least Windows XP with SP2
         wchar_t host[NI_MAXHOST], service[NI_MAXSERV]; if(!GetNameInfoW((sockaddr*)&addr, SIZE(addr), host, Elms(host), service, Elms(service), NI_NUMERICSERV))
         { // ok
            ComputerName=host;
         }
      #endif
      }
      return true;
   }
   return false;
#else
   signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE signal
   Char8 host[NI_MAXHOST]; if(!gethostname(host, Elms(host)))ComputerName=host;
#if MAC // Mac can append a suffix ".lan"
   if(Ends(ComputerName, ".lan"))ComputerName.removeLast(4);
#endif
   if(!ComputerName.is())ComputerName="localhost";
   #if LINUX
      Socket sock; if(sock.createUdp(true)) // we need this only for Mac Address, prefer IPv6 because it's better
      {
         ifreq ifr;
         // try ethernet first
         Set(ifr.ifr_name, "eth0"); if(ioctl((SOCKET)sock._s, SIOCGIFHWADDR, &ifr)!=SOCKET_ERROR)CopyFast(&Mac, ifr.ifr_hwaddr.sa_data, Min(SIZEU(Mac), 6));
         if(!Mac) // try wireless lan next
         {
            Set(ifr.ifr_name, "wlan0"); if(ioctl((SOCKET)sock._s, SIOCGIFHWADDR, &ifr)!=SOCKET_ERROR)CopyFast(&Mac, ifr.ifr_hwaddr.sa_data, Min(SIZEU(Mac), 6));
            if(!Mac) // if failed then try all available interfaces
            {
               ifreq  buffer[32];
               ifconf ifc;
               ifc.ifc_len=SIZE(buffer);
               ifc.ifc_req=buffer;

               if(ioctl((SOCKET)sock._s, SIOCGIFCONF, &ifc)!=SOCKET_ERROR)
                  FREP(ifc.ifc_len/SIZE(ifreq))
               {
                  ifreq &ifr=buffer[i];
                  if(ioctl((SOCKET)sock._s, SIOCGIFFLAGS, &ifr)!=SOCKET_ERROR)
                     if(!(ifr.ifr_flags&IFF_LOOPBACK)) // ignore loopback
                        if(ioctl((SOCKET)sock._s, SIOCGIFHWADDR, &ifr)!=SOCKET_ERROR)
                  {
                     CopyFast(&Mac, ifr.ifr_hwaddr.sa_data, Min(SIZEU(Mac), 6));
                     break; // stop on first found
                  }
               }
            }
         }
      }
   #elif MAC
      ifaddrs *interfaces=null;  
      if(!getifaddrs(&interfaces))
      {
         for(ifaddrs *r=interfaces; r; r=r->ifa_next)
            if(r->ifa_addr)switch(r->ifa_addr->sa_family)
         {
            case AF_LINK:
            {
               sockaddr_dl &addr=*(sockaddr_dl*)r->ifa_addr;
               if(addr.sdl_type==IFT_ETHER)
               {
                  CopyFast(&Mac, addr.sdl_data+addr.sdl_nlen, Min(SIZEU(Mac), addr.sdl_alen));
                  goto found; // stop on first found
               }
            }break;
         }
      found:
         freeifaddrs(interfaces);
      }
   #elif IOS // since iOS 7 this will always return "02:00:00:00:00:00"
      Socket sock; if(sock.createUdp(true)) // we need this only for Mac Address, prefer IPv6 because it's better
      {
         ifreq  buffer[32];
         ifconf ifc;
         ifc.ifc_len=SIZE(buffer);
         ifc.ifc_req=buffer;

         if(ioctl((SOCKET)sock._s, SIOCGIFCONF, &ifc)!=SOCKET_ERROR)
            FREP(ifc.ifc_len/SIZE(ifreq))
         {
            ifreq &ifr=buffer[i];
            switch(ifr.ifr_addr.sa_family)
            {
               case AF_LINK:
               {
                  sockaddr_dl &addr=(sockaddr_dl&)ifr.ifr_addr;
                  if(addr.sdl_type==IFT_ETHER)
                  {
                     CopyFast(&Mac, addr.sdl_data+addr.sdl_nlen, Min(SIZEU(Mac), addr.sdl_alen));
                     goto found; // stop on first found
                  }
               }break;
            }
         }
      found:;
      }
   #elif ANDROID
      if(!Mac)
      if(Jni && ActivityClass)
      if(JClass ContextClass="android/content/Context")
      if(JFieldID WIFI_SERVICEField=Jni->GetStaticFieldID(ContextClass, "WIFI_SERVICE", "Ljava/lang/String;"))
      if(JObject WIFI_SERVICE=Jni->GetStaticObjectField(ContextClass, WIFI_SERVICEField))
      if(JClass WifiManagerClass="android/net/wifi/WifiManager")
      if(JMethodID getSystemService=Jni->GetMethodID(ActivityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"))
      {
         JObject WifiManager=Jni->CallObjectMethod(Activity, getSystemService, WIFI_SERVICE());
         if(Jni->ExceptionCheck())
         {
         #if DEBUG
            Jni->ExceptionDescribe();
         #endif
            WifiManager.clear(); WifiManagerClass.clear(); Jni->ExceptionClear();
         }
         if(WifiManager)
         if(JClass WifiInfoClass="android/net/wifi/WifiInfo")
         if(JMethodID getConnectionInfo=Jni->GetMethodID(WifiManagerClass, "getConnectionInfo", "()Landroid/net/wifi/WifiInfo;"))
         if(JObject WifiInfo=Jni->CallObjectMethod(WifiManager, getConnectionInfo))
         if(JMethodID getMacAddress=Jni->GetMethodID(WifiInfoClass, "getMacAddress", "()Ljava/lang/String;"))
         if(JString MacAddress=Jni->CallObjectMethod(WifiInfo     ,  getMacAddress))
         {
            Memt<Str> mac; Split(mac, MacAddress.str(), ':');
            REP(Min(SIZE(Mac), Elms(mac)))((Byte*)&Mac)[i]=TextInt(S+"0x"+mac[i]);
         }
      }
   #endif
   return true;
#endif
}
/******************************************************************************/
// SECURE
/******************************************************************************/
#if SUPPORT_MBED_TLS
#include "_/Certificates.h"
static mbedtls_x509_crt         Certificates;
static mbedtls_entropy_context  entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_ssl_config       SecureConfig;
static SyncLock                 SecureSyncLock;

#if WINDOWS
extern "C" int mbedtls_platform_entropy_poll(Ptr data, Byte *output, size_t len, size_t *olen) // manually define this function so we can specify different bodies depending on WINDOWS_OLD/WINDOWS_NEW, without having to create separate libs for mbedTLS library
{
  *olen=0; Int r=MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
#if WINDOWS_OLD
   HCRYPTPROV provider; if(CryptAcquireContext(&provider, null, null, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
   {
      if(CryptGenRandom(provider, (DWORD)len, output)){*olen=len; r=0;}
      CryptReleaseContext(provider, 0);
   }
#else
   if(OK(BCryptGenRandom(null, output, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG))){*olen=len; r=0;}
#endif
   return r;
}
#endif

static int EntropyFunc(Ptr ctx, Byte *output, size_t len) {SyncLocker sl(SecureSyncLock); return mbedtls_entropy_func   (ctx, output, len);}
static int  RandomFunc(Ptr ctx, Byte *output, size_t len) {SyncLocker sl(SecureSyncLock); return mbedtls_ctr_drbg_random(ctx, output, len);}
#endif
/******************************************************************************/
static void DebugCallback(Ptr ctx, int level, CChar8 *file, int line, CChar8 *str)
{
   Log(str);
}
static void ShutSecure()
{
#if SUPPORT_MBED_TLS
   SyncLocker sl(SecureSyncLock);
   mbedtls_ssl_config_free(&SecureConfig);
   mbedtls_ctr_drbg_free  (&ctr_drbg);
   mbedtls_entropy_free   (&entropy);
   mbedtls_x509_crt_free  (&Certificates);
#endif
}
static void InitSecure()
{
#if SUPPORT_MBED_TLS
#define TEST_LOAD_SPEED (DEBUG && 0)
#if     TEST_LOAD_SPEED
   Dbl t=Time.curTime();
#endif
   if(!SecureConfig.f_dbg)
   {
      SyncLocker sl(SecureSyncLock); if(!SecureConfig.f_dbg)
      {
         {
            File cmpr, src; cmpr.readMem(CertificatesData, SIZE(CertificatesData));
            DYNAMIC_ASSERT(DecompressRaw(cmpr, src, COMPRESS_LZ4, cmpr.size(), CertificatesSize, true), "Can't decompress Certificates");
            DYNAMIC_ASSERT(src._type==FILE_MEM, "Src should be FILE_MEM");
            mbedtls_x509_crt *cert=&Certificates;
            mbedtls_x509_crt_init(cert);
            for(src.pos(0); !src.end(); )
            {
               Int size=src.getUShort(); DYNAMIC_ASSERT(size>0 && size<=src.left(), "Invalid Certificate size");
               DYNAMIC_ASSERT(mbedtls_x509_crt_parse_der(cert, (Byte*)src.memFast(), size)==0, "Can't load Certificate");
               if(cert->next)cert=cert->next; // advance to the last available to avoid iteration inside 'mbedtls_x509_crt_parse_der'
               src.skip(size);
            }
          //Int n=0; for(mbedtls_x509_crt *cert=&Certificates; cert; cert=cert->next)n++; Exit(n); display how many certificates were loaded
         }
      #if DEBUG && defined MBEDTLS_DEBUG_C
         mbedtls_debug_set_threshold(4); // enable reporting all messages, use 1 for errors only
      #endif
         mbedtls_ssl_config_init(&SecureConfig);
         mbedtls_ctr_drbg_init(&ctr_drbg);
         mbedtls_entropy_init(&entropy);
         UID custom; custom.randomize(); // using random values improves entropy
         DYNAMIC_ASSERT(!mbedtls_ctr_drbg_seed(&ctr_drbg, EntropyFunc, &entropy, custom.b, SIZE(custom)), "mbedtls_ctr_drbg_seed");
         DYNAMIC_ASSERT(!mbedtls_ssl_config_defaults(&SecureConfig, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT), "mbedtls_ssl_config_defaults");
         mbedtls_ssl_conf_read_timeout(&SecureConfig, DOWNLOAD_WAIT_TIME); ASSERT(DOWNLOAD_WAIT_TIME!=0); // for 'mbedtls_ssl_conf_read_timeout' 0 is actually unlimited and not instant
         mbedtls_ssl_conf_authmode(&SecureConfig, MBEDTLS_SSL_VERIFY_OPTIONAL); // make optional here and just check 'mbedtls_ssl_get_verify_result' manually so we can decide what to do later
         mbedtls_ssl_conf_ca_chain(&SecureConfig, &Certificates, null);
         mbedtls_ssl_conf_rng(&SecureConfig, RandomFunc, &ctr_drbg);

         // !! always set this and as the last thing, because this is used for checking if lib was initialized !!
         mbedtls_ssl_conf_dbg(&SecureConfig, DebugCallback, null);
         DEBUG_ASSERT(SecureConfig.f_dbg, "SecureConfig.f_dbg==null"); // make sure that 'SecureConfig.f_dbg' was set which is used for detecting if lib was initialized
      }
   }
#if TEST_LOAD_SPEED
   Exit(Flt(Time.curTime()-t)); // loads very fast, in 0.001s
#endif
#endif
}
#if UPDATE_CERTIFICATES
static void UpdateCertificates()
{
#if SUPPORT_MBED_TLS
   Mems<Byte> data;
   {
   #if 0 // from file
      File f; f.read(SystemPath(SP_DESKTOP).tailSlash(true)+"cacert.pem"); f.copyToAndDiscard(data); f.del();
   #else // from internet
      Download down; down.create("https://curl.haxx.se/ca/cacert.pem"); down.wait(); // this is from CURL based on Mozilla, some alternative resources:
      // https://hg.mozilla.org/mozilla-central/raw-file/tip/security/nss/lib/ckfw/builtins/certdata.txt
      // https://chromium.googlesource.com/external/github.com/dart-lang/root_certificates/+/master
      // https://chromium.googlesource.com/external/github.com/dart-lang/root_certificates/+/master/certdata.pem
      DYNAMIC_ASSERT(down.state()==DWNL_DONE && down.done()>0, "Can't download certificates");
      data.setNum(down.size()).copyFrom((Byte*)down.data());
   #endif
   }
   mbedtls_x509_crt certs; mbedtls_x509_crt_init(&certs);
   data.add(0); // add null character which is required by 'mbedtls_x509_crt_parse'
   Int error=mbedtls_x509_crt_parse(&certs, data.data(), data.elms());
   DYNAMIC_ASSERT(error==0, S+"Cert failed to load:"+error);
   Memc<Memc<Byte>> cert_data;
   for(mbedtls_x509_crt *cert=&certs; cert; cert=cert->next)
   {
      Int size=Int(cert->raw.len); DYNAMIC_ASSERT(size>0 && size<=USHORT_MAX, S+"Invalid CERT size:"+size);
      cert_data.New().setNum(size).copyFrom(cert->raw.p);
   }
   File f; f.writeMem();
   FREPA(cert_data)
   {
      f.putUShort(cert_data[i].elms());
      f.put      (cert_data[i].data(), cert_data[i].elms());
   }
   f.pos(0);
   File cmpr; DYNAMIC_ASSERT(CompressRaw(f, cmpr.writeMem(), COMPRESS_LZ4, 9), "failed to compress");
   {
      Str dest=GetPath(__FILE__).tailSlash(true)+"_/Certificates.h";
      FileText ft;
      ft.write(dest);
      ft.putLine(S+"static const Int  CertificatesSize="+f.size()+';');
      ft.putLine(  "static const Byte CertificatesData[]=");
      ft.putLine("{");
      for(cmpr.pos(0); !cmpr.end(); ){if(cmpr.pos()!=0){ft.putText(","); if(!(cmpr.pos()%64))ft.endLine();} ft.putText(cmpr.getByte());}
      ft.endLine();
      ft.putLine("};");
      Exit(S+"Certificates updated OK: "+ft.flushOK()+"\nCompressed Size: "+TextInt(cmpr.size(), -1, 3));
   }
#endif
}
#endif
/******************************************************************************/
void SecureSocket::del()
{
   unsecure(); // 'unsecure' first so connection can be notified of closing
   super::del();
}
void SecureSocket::unsecure()
{
#if SUPPORT_MBED_TLS
   if(_secure)
   {
      mbedtls_ssl_close_notify(_secure);
      mbedtls_ssl_free(_secure);
      Free(_secure);
   }
#endif
}
Bool SecureSocket::secure(CChar8 *host)
{
   unsecure();
   InitSecure(); // first make sure that lib was initialized
#if SUPPORT_MBED_TLS
   Alloc(_secure);
           mbedtls_ssl_init        (_secure);
   return !mbedtls_ssl_setup       (_secure, &SecureConfig)
       && !mbedtls_ssl_set_hostname(_secure, _GetStart(host));
#endif
   return false;
}
#if SUPPORT_MBED_TLS
static int SendFunc(Ptr ctx, C Byte *buf, size_t len)
{
   SecureSocket &s=*(SecureSocket*)ctx; Int r=s.Socket::send(buf, (Int)len); if(r<0)return Socket::WouldBlock() ? MBEDTLS_ERR_SSL_WANT_WRITE : MBEDTLS_ERR_NET_SEND_FAILED;
   return r;
}
static int ReceiveFunc(Ptr ctx, Byte *buf, size_t len)
{
   SecureSocket &s=*(SecureSocket*)ctx; Int r=s.Socket::receive(buf, (Int)len); if(r<0)return Socket::WouldBlock() ? MBEDTLS_ERR_SSL_WANT_READ : MBEDTLS_ERR_NET_RECV_FAILED;
   return r;
}
static int ReceiveFuncTimeout(Ptr ctx, Byte *buf, size_t len, uint32_t timeout)
{
   SecureSocket &s=*(SecureSocket*)ctx; if(!s.wait(timeout ? Min(timeout, INT_MAX) : INT_MAX))return MBEDTLS_ERR_SSL_TIMEOUT; // Min to INT_MAX because 'wait' expected Int, if 'timeout' is zero, then use INT_MAX because MBED TLS treats this as unlimited time
   return ReceiveFunc(ctx, buf, len);
}
static inline Int MbedTlsToResult(Int r)
{
   switch(r)
   {
      case MBEDTLS_ERR_SSL_WANT_READ :
      case MBEDTLS_ERR_SSL_TIMEOUT   : return SecureSocket::NEED_WAIT;

      case MBEDTLS_ERR_SSL_WANT_WRITE: return SecureSocket::NEED_FLUSH;

      default: return (r>=-1) ? r : SecureSocket::ERROR;
   }
}
#endif
void SecureSocket::setDefaultFunc()
{
#if SUPPORT_MBED_TLS
   mbedtls_ssl_set_bio(_secure, this, SendFunc, ReceiveFunc, ReceiveFuncTimeout);
#endif
}
Int SecureSocket::send(CPtr data, Int size)
{
#if SUPPORT_MBED_TLS
   if(_secure)return MbedTlsToResult(mbedtls_ssl_write(_secure, (Byte*)data, size));
#endif
   return super::send(data, size);
}
Int SecureSocket::receive(Ptr data, Int size)
{
#if SUPPORT_MBED_TLS
   if(_secure)return MbedTlsToResult(mbedtls_ssl_read(_secure, (Byte*)data, size));
#endif
   return super::receive(data, size);
}
SecureSocket::RESULT SecureSocket::handshake()
{
   if(!_secure)return OK;
#if SUPPORT_MBED_TLS
   switch(mbedtls_ssl_handshake(_secure))
   {
      case 0: return mbedtls_ssl_get_verify_result(_secure) ? BAD_CERT : OK;

      case MBEDTLS_ERR_SSL_WANT_READ :
      case MBEDTLS_ERR_SSL_TIMEOUT   : return NEED_WAIT;

      case MBEDTLS_ERR_SSL_WANT_WRITE: return NEED_FLUSH;
   }
#endif
   return ERROR;
}
/******************************************************************************/
Bool InitSocket()
{
   if(InitSocketEx())
   {
   #if UPDATE_CERTIFICATES
      #pragma message("!! Warning: Use this only once in many days !!")
      UpdateCertificates();
   #endif
      return true;
   }
   return false;
}
void ShutSocket()
{
   GIPC.del();
#if WINDOWS
   WSACleanup();
#endif
   ShutSecure();
}
/******************************************************************************/
}
/******************************************************************************/
