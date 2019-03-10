/******************************************************************************

   Mails are sent on secondary thread, because 'SendMail' can be slow.

/******************************************************************************/
class Mail
{
   Str to_name, to_email, subject, body;

   void set(C Str &to_name, C Str &to_email, C Str &subject, C Str &body) {T.to_name=to_name; T.to_email=to_email; T.subject=subject; T.body=body;}

   bool send() {return SendMail("Esenthel Server", "no-reply@esenthel_server.com", to_name, to_email, subject, body);}
}
/******************************************************************************/
Memc<Mail> Mails;
SyncLock   MailLock;
Thread     MailThread;
SyncEvent  MailEvent;
/******************************************************************************/
bool MailSupportedYes, MailSupportedKnown;
bool MailSupported()
{
   if(!MailSupportedKnown)
   {
      MailSupportedYes=SendMailSupported();
      MailSupportedKnown=true;
   }
   return MailSupportedYes;
}
/******************************************************************************/
bool QueueMail(C Str &to_name, C Str &to_email, C Str &subject, C Str &body)
{
   {
      SyncLocker locker(MailLock); Mails.New().set(to_name, to_email, subject, body);
   }
   MailEvent.on();
   return MailSupported();
}
/******************************************************************************/
bool MailFunc(Thread &thread)
{
   MailSupported(); // check this at start of the thread, so further calls to 'MailSupported' on the main thread will not stall
   MailEvent.wait();
   for(; Mails.elms(); )
   {
      SyncLockerEx locker(MailLock);
      if(Mails.elms())
      {
         Mail mail; Swap(mail, Mails.last()); Mails.removeLast(); locker.off();

         // send mail
         if(!mail.send())LogN(S+DateTime().getUTC().asText(true)+" Error sending mail To: \""+mail.to_email+"\" Subject: \""+mail.subject+"\" Body: \""+mail.body+"\"");
      }
   }
   return true;
}
void InitMailer()
{
   MailThread.create(MailFunc);
}
void ShutMailer()
{
   MailThread.stop(); MailEvent.on();
   MailThread.del();
}
/******************************************************************************/
