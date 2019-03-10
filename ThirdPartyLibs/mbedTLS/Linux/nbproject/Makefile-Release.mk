#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/bc0f7162/aes.o \
	${OBJECTDIR}/_ext/bc0f7162/aesni.o \
	${OBJECTDIR}/_ext/bc0f7162/arc4.o \
	${OBJECTDIR}/_ext/bc0f7162/aria.o \
	${OBJECTDIR}/_ext/bc0f7162/asn1parse.o \
	${OBJECTDIR}/_ext/bc0f7162/asn1write.o \
	${OBJECTDIR}/_ext/bc0f7162/base64.o \
	${OBJECTDIR}/_ext/bc0f7162/bignum.o \
	${OBJECTDIR}/_ext/bc0f7162/blowfish.o \
	${OBJECTDIR}/_ext/bc0f7162/camellia.o \
	${OBJECTDIR}/_ext/bc0f7162/ccm.o \
	${OBJECTDIR}/_ext/bc0f7162/certs.o \
	${OBJECTDIR}/_ext/bc0f7162/chacha20.o \
	${OBJECTDIR}/_ext/bc0f7162/chachapoly.o \
	${OBJECTDIR}/_ext/bc0f7162/cipher.o \
	${OBJECTDIR}/_ext/bc0f7162/cipher_wrap.o \
	${OBJECTDIR}/_ext/bc0f7162/cmac.o \
	${OBJECTDIR}/_ext/bc0f7162/ctr_drbg.o \
	${OBJECTDIR}/_ext/bc0f7162/debug.o \
	${OBJECTDIR}/_ext/bc0f7162/des.o \
	${OBJECTDIR}/_ext/bc0f7162/dhm.o \
	${OBJECTDIR}/_ext/bc0f7162/ecdh.o \
	${OBJECTDIR}/_ext/bc0f7162/ecdsa.o \
	${OBJECTDIR}/_ext/bc0f7162/ecjpake.o \
	${OBJECTDIR}/_ext/bc0f7162/ecp.o \
	${OBJECTDIR}/_ext/bc0f7162/ecp_curves.o \
	${OBJECTDIR}/_ext/bc0f7162/entropy.o \
	${OBJECTDIR}/_ext/bc0f7162/entropy_poll.o \
	${OBJECTDIR}/_ext/bc0f7162/error.o \
	${OBJECTDIR}/_ext/bc0f7162/gcm.o \
	${OBJECTDIR}/_ext/bc0f7162/havege.o \
	${OBJECTDIR}/_ext/bc0f7162/hkdf.o \
	${OBJECTDIR}/_ext/bc0f7162/hmac_drbg.o \
	${OBJECTDIR}/_ext/bc0f7162/md.o \
	${OBJECTDIR}/_ext/bc0f7162/md2.o \
	${OBJECTDIR}/_ext/bc0f7162/md4.o \
	${OBJECTDIR}/_ext/bc0f7162/md5.o \
	${OBJECTDIR}/_ext/bc0f7162/md_wrap.o \
	${OBJECTDIR}/_ext/bc0f7162/memory_buffer_alloc.o \
	${OBJECTDIR}/_ext/bc0f7162/net_sockets.o \
	${OBJECTDIR}/_ext/bc0f7162/nist_kw.o \
	${OBJECTDIR}/_ext/bc0f7162/oid.o \
	${OBJECTDIR}/_ext/bc0f7162/padlock.o \
	${OBJECTDIR}/_ext/bc0f7162/pem.o \
	${OBJECTDIR}/_ext/bc0f7162/pk.o \
	${OBJECTDIR}/_ext/bc0f7162/pk_wrap.o \
	${OBJECTDIR}/_ext/bc0f7162/pkcs11.o \
	${OBJECTDIR}/_ext/bc0f7162/pkcs12.o \
	${OBJECTDIR}/_ext/bc0f7162/pkcs5.o \
	${OBJECTDIR}/_ext/bc0f7162/pkparse.o \
	${OBJECTDIR}/_ext/bc0f7162/pkwrite.o \
	${OBJECTDIR}/_ext/bc0f7162/platform.o \
	${OBJECTDIR}/_ext/bc0f7162/platform_util.o \
	${OBJECTDIR}/_ext/bc0f7162/poly1305.o \
	${OBJECTDIR}/_ext/bc0f7162/ripemd160.o \
	${OBJECTDIR}/_ext/bc0f7162/rsa.o \
	${OBJECTDIR}/_ext/bc0f7162/rsa_internal.o \
	${OBJECTDIR}/_ext/bc0f7162/sha1.o \
	${OBJECTDIR}/_ext/bc0f7162/sha256.o \
	${OBJECTDIR}/_ext/bc0f7162/sha512.o \
	${OBJECTDIR}/_ext/bc0f7162/ssl_cache.o \
	${OBJECTDIR}/_ext/bc0f7162/ssl_ciphersuites.o \
	${OBJECTDIR}/_ext/bc0f7162/ssl_cli.o \
	${OBJECTDIR}/_ext/bc0f7162/ssl_cookie.o \
	${OBJECTDIR}/_ext/bc0f7162/ssl_srv.o \
	${OBJECTDIR}/_ext/bc0f7162/ssl_ticket.o \
	${OBJECTDIR}/_ext/bc0f7162/ssl_tls.o \
	${OBJECTDIR}/_ext/bc0f7162/threading.o \
	${OBJECTDIR}/_ext/bc0f7162/timing.o \
	${OBJECTDIR}/_ext/bc0f7162/version.o \
	${OBJECTDIR}/_ext/bc0f7162/version_features.o \
	${OBJECTDIR}/_ext/bc0f7162/x509.o \
	${OBJECTDIR}/_ext/bc0f7162/x509_create.o \
	${OBJECTDIR}/_ext/bc0f7162/x509_crl.o \
	${OBJECTDIR}/_ext/bc0f7162/x509_crt.o \
	${OBJECTDIR}/_ext/bc0f7162/x509_csr.o \
	${OBJECTDIR}/_ext/bc0f7162/x509write_crt.o \
	${OBJECTDIR}/_ext/bc0f7162/x509write_csr.o \
	${OBJECTDIR}/_ext/bc0f7162/xtea.o


# C Compiler Flags
CFLAGS=-m64

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblinux.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblinux.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblinux.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblinux.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblinux.a

${OBJECTDIR}/_ext/bc0f7162/aes.o: ../lib/library/aes.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/aes.o ../lib/library/aes.c

${OBJECTDIR}/_ext/bc0f7162/aesni.o: ../lib/library/aesni.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/aesni.o ../lib/library/aesni.c

${OBJECTDIR}/_ext/bc0f7162/arc4.o: ../lib/library/arc4.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/arc4.o ../lib/library/arc4.c

${OBJECTDIR}/_ext/bc0f7162/aria.o: ../lib/library/aria.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/aria.o ../lib/library/aria.c

${OBJECTDIR}/_ext/bc0f7162/asn1parse.o: ../lib/library/asn1parse.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/asn1parse.o ../lib/library/asn1parse.c

${OBJECTDIR}/_ext/bc0f7162/asn1write.o: ../lib/library/asn1write.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/asn1write.o ../lib/library/asn1write.c

${OBJECTDIR}/_ext/bc0f7162/base64.o: ../lib/library/base64.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/base64.o ../lib/library/base64.c

${OBJECTDIR}/_ext/bc0f7162/bignum.o: ../lib/library/bignum.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/bignum.o ../lib/library/bignum.c

${OBJECTDIR}/_ext/bc0f7162/blowfish.o: ../lib/library/blowfish.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/blowfish.o ../lib/library/blowfish.c

${OBJECTDIR}/_ext/bc0f7162/camellia.o: ../lib/library/camellia.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/camellia.o ../lib/library/camellia.c

${OBJECTDIR}/_ext/bc0f7162/ccm.o: ../lib/library/ccm.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ccm.o ../lib/library/ccm.c

${OBJECTDIR}/_ext/bc0f7162/certs.o: ../lib/library/certs.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/certs.o ../lib/library/certs.c

${OBJECTDIR}/_ext/bc0f7162/chacha20.o: ../lib/library/chacha20.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/chacha20.o ../lib/library/chacha20.c

${OBJECTDIR}/_ext/bc0f7162/chachapoly.o: ../lib/library/chachapoly.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/chachapoly.o ../lib/library/chachapoly.c

${OBJECTDIR}/_ext/bc0f7162/cipher.o: ../lib/library/cipher.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/cipher.o ../lib/library/cipher.c

${OBJECTDIR}/_ext/bc0f7162/cipher_wrap.o: ../lib/library/cipher_wrap.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/cipher_wrap.o ../lib/library/cipher_wrap.c

${OBJECTDIR}/_ext/bc0f7162/cmac.o: ../lib/library/cmac.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/cmac.o ../lib/library/cmac.c

${OBJECTDIR}/_ext/bc0f7162/ctr_drbg.o: ../lib/library/ctr_drbg.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ctr_drbg.o ../lib/library/ctr_drbg.c

${OBJECTDIR}/_ext/bc0f7162/debug.o: ../lib/library/debug.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/debug.o ../lib/library/debug.c

${OBJECTDIR}/_ext/bc0f7162/des.o: ../lib/library/des.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/des.o ../lib/library/des.c

${OBJECTDIR}/_ext/bc0f7162/dhm.o: ../lib/library/dhm.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/dhm.o ../lib/library/dhm.c

${OBJECTDIR}/_ext/bc0f7162/ecdh.o: ../lib/library/ecdh.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ecdh.o ../lib/library/ecdh.c

${OBJECTDIR}/_ext/bc0f7162/ecdsa.o: ../lib/library/ecdsa.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ecdsa.o ../lib/library/ecdsa.c

${OBJECTDIR}/_ext/bc0f7162/ecjpake.o: ../lib/library/ecjpake.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ecjpake.o ../lib/library/ecjpake.c

${OBJECTDIR}/_ext/bc0f7162/ecp.o: ../lib/library/ecp.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ecp.o ../lib/library/ecp.c

${OBJECTDIR}/_ext/bc0f7162/ecp_curves.o: ../lib/library/ecp_curves.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ecp_curves.o ../lib/library/ecp_curves.c

${OBJECTDIR}/_ext/bc0f7162/entropy.o: ../lib/library/entropy.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/entropy.o ../lib/library/entropy.c

${OBJECTDIR}/_ext/bc0f7162/entropy_poll.o: ../lib/library/entropy_poll.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/entropy_poll.o ../lib/library/entropy_poll.c

${OBJECTDIR}/_ext/bc0f7162/error.o: ../lib/library/error.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/error.o ../lib/library/error.c

${OBJECTDIR}/_ext/bc0f7162/gcm.o: ../lib/library/gcm.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/gcm.o ../lib/library/gcm.c

${OBJECTDIR}/_ext/bc0f7162/havege.o: ../lib/library/havege.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/havege.o ../lib/library/havege.c

${OBJECTDIR}/_ext/bc0f7162/hkdf.o: ../lib/library/hkdf.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/hkdf.o ../lib/library/hkdf.c

${OBJECTDIR}/_ext/bc0f7162/hmac_drbg.o: ../lib/library/hmac_drbg.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/hmac_drbg.o ../lib/library/hmac_drbg.c

${OBJECTDIR}/_ext/bc0f7162/md.o: ../lib/library/md.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/md.o ../lib/library/md.c

${OBJECTDIR}/_ext/bc0f7162/md2.o: ../lib/library/md2.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/md2.o ../lib/library/md2.c

${OBJECTDIR}/_ext/bc0f7162/md4.o: ../lib/library/md4.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/md4.o ../lib/library/md4.c

${OBJECTDIR}/_ext/bc0f7162/md5.o: ../lib/library/md5.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/md5.o ../lib/library/md5.c

${OBJECTDIR}/_ext/bc0f7162/md_wrap.o: ../lib/library/md_wrap.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/md_wrap.o ../lib/library/md_wrap.c

${OBJECTDIR}/_ext/bc0f7162/memory_buffer_alloc.o: ../lib/library/memory_buffer_alloc.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/memory_buffer_alloc.o ../lib/library/memory_buffer_alloc.c

${OBJECTDIR}/_ext/bc0f7162/net_sockets.o: ../lib/library/net_sockets.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/net_sockets.o ../lib/library/net_sockets.c

${OBJECTDIR}/_ext/bc0f7162/nist_kw.o: ../lib/library/nist_kw.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/nist_kw.o ../lib/library/nist_kw.c

${OBJECTDIR}/_ext/bc0f7162/oid.o: ../lib/library/oid.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/oid.o ../lib/library/oid.c

${OBJECTDIR}/_ext/bc0f7162/padlock.o: ../lib/library/padlock.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/padlock.o ../lib/library/padlock.c

${OBJECTDIR}/_ext/bc0f7162/pem.o: ../lib/library/pem.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/pem.o ../lib/library/pem.c

${OBJECTDIR}/_ext/bc0f7162/pk.o: ../lib/library/pk.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/pk.o ../lib/library/pk.c

${OBJECTDIR}/_ext/bc0f7162/pk_wrap.o: ../lib/library/pk_wrap.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/pk_wrap.o ../lib/library/pk_wrap.c

${OBJECTDIR}/_ext/bc0f7162/pkcs11.o: ../lib/library/pkcs11.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/pkcs11.o ../lib/library/pkcs11.c

${OBJECTDIR}/_ext/bc0f7162/pkcs12.o: ../lib/library/pkcs12.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/pkcs12.o ../lib/library/pkcs12.c

${OBJECTDIR}/_ext/bc0f7162/pkcs5.o: ../lib/library/pkcs5.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/pkcs5.o ../lib/library/pkcs5.c

${OBJECTDIR}/_ext/bc0f7162/pkparse.o: ../lib/library/pkparse.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/pkparse.o ../lib/library/pkparse.c

${OBJECTDIR}/_ext/bc0f7162/pkwrite.o: ../lib/library/pkwrite.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/pkwrite.o ../lib/library/pkwrite.c

${OBJECTDIR}/_ext/bc0f7162/platform.o: ../lib/library/platform.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/platform.o ../lib/library/platform.c

${OBJECTDIR}/_ext/bc0f7162/platform_util.o: ../lib/library/platform_util.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/platform_util.o ../lib/library/platform_util.c

${OBJECTDIR}/_ext/bc0f7162/poly1305.o: ../lib/library/poly1305.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/poly1305.o ../lib/library/poly1305.c

${OBJECTDIR}/_ext/bc0f7162/ripemd160.o: ../lib/library/ripemd160.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ripemd160.o ../lib/library/ripemd160.c

${OBJECTDIR}/_ext/bc0f7162/rsa.o: ../lib/library/rsa.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/rsa.o ../lib/library/rsa.c

${OBJECTDIR}/_ext/bc0f7162/rsa_internal.o: ../lib/library/rsa_internal.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/rsa_internal.o ../lib/library/rsa_internal.c

${OBJECTDIR}/_ext/bc0f7162/sha1.o: ../lib/library/sha1.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/sha1.o ../lib/library/sha1.c

${OBJECTDIR}/_ext/bc0f7162/sha256.o: ../lib/library/sha256.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/sha256.o ../lib/library/sha256.c

${OBJECTDIR}/_ext/bc0f7162/sha512.o: ../lib/library/sha512.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/sha512.o ../lib/library/sha512.c

${OBJECTDIR}/_ext/bc0f7162/ssl_cache.o: ../lib/library/ssl_cache.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ssl_cache.o ../lib/library/ssl_cache.c

${OBJECTDIR}/_ext/bc0f7162/ssl_ciphersuites.o: ../lib/library/ssl_ciphersuites.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ssl_ciphersuites.o ../lib/library/ssl_ciphersuites.c

${OBJECTDIR}/_ext/bc0f7162/ssl_cli.o: ../lib/library/ssl_cli.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ssl_cli.o ../lib/library/ssl_cli.c

${OBJECTDIR}/_ext/bc0f7162/ssl_cookie.o: ../lib/library/ssl_cookie.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ssl_cookie.o ../lib/library/ssl_cookie.c

${OBJECTDIR}/_ext/bc0f7162/ssl_srv.o: ../lib/library/ssl_srv.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ssl_srv.o ../lib/library/ssl_srv.c

${OBJECTDIR}/_ext/bc0f7162/ssl_ticket.o: ../lib/library/ssl_ticket.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ssl_ticket.o ../lib/library/ssl_ticket.c

${OBJECTDIR}/_ext/bc0f7162/ssl_tls.o: ../lib/library/ssl_tls.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/ssl_tls.o ../lib/library/ssl_tls.c

${OBJECTDIR}/_ext/bc0f7162/threading.o: ../lib/library/threading.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/threading.o ../lib/library/threading.c

${OBJECTDIR}/_ext/bc0f7162/timing.o: ../lib/library/timing.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/timing.o ../lib/library/timing.c

${OBJECTDIR}/_ext/bc0f7162/version.o: ../lib/library/version.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/version.o ../lib/library/version.c

${OBJECTDIR}/_ext/bc0f7162/version_features.o: ../lib/library/version_features.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/version_features.o ../lib/library/version_features.c

${OBJECTDIR}/_ext/bc0f7162/x509.o: ../lib/library/x509.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/x509.o ../lib/library/x509.c

${OBJECTDIR}/_ext/bc0f7162/x509_create.o: ../lib/library/x509_create.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/x509_create.o ../lib/library/x509_create.c

${OBJECTDIR}/_ext/bc0f7162/x509_crl.o: ../lib/library/x509_crl.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/x509_crl.o ../lib/library/x509_crl.c

${OBJECTDIR}/_ext/bc0f7162/x509_crt.o: ../lib/library/x509_crt.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/x509_crt.o ../lib/library/x509_crt.c

${OBJECTDIR}/_ext/bc0f7162/x509_csr.o: ../lib/library/x509_csr.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/x509_csr.o ../lib/library/x509_csr.c

${OBJECTDIR}/_ext/bc0f7162/x509write_crt.o: ../lib/library/x509write_crt.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/x509write_crt.o ../lib/library/x509write_crt.c

${OBJECTDIR}/_ext/bc0f7162/x509write_csr.o: ../lib/library/x509write_csr.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/x509write_csr.o ../lib/library/x509write_csr.c

${OBJECTDIR}/_ext/bc0f7162/xtea.o: ../lib/library/xtea.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bc0f7162
	${RM} "$@.d"
	$(COMPILE.c) -O3 -s -I../lib/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bc0f7162/xtea.o ../lib/library/xtea.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
