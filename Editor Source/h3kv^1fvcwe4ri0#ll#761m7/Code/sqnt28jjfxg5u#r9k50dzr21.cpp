/******************************************************************************/
class User // can be accessed by multiple clients at the same time (for example 1 person working on Win+Mac)
{
   Str  email, name;
   UID  id, pass;
   uint change_pass_key=0;
   USER_ACCESS access=UA_NO_ACCESS;
   DateTime reg_date, login_date;
   int clients=0; // number of clients linked with this user, this should not be saved
   ImagePtr online; // this should not be saved, it is used only for displaying

   User() {id.randomize(); pass.randomize(); login_date.zero(); reg_date.getUTC();}

   bool canRead     ()C {return CanRead     (access);}
   bool canWrite    ()C {return CanWrite    (access);}
   bool canWriteCode()C {return CanWriteCode(access);}

   void changeClients(int delta)
   {
      clients+=delta;
      online  =((clients>0 && Gui.skin) ? Gui.skin->menu.check : ImagePtr());
   }

   bool save(File &f)C
   {
      f.cmpUIntV(3);
      f<<id<<pass<<access<<email<<name;
        reg_date.save(f);
      login_date.save(f);
      return f.ok();
   }
   bool load(File &f)
   {
      switch(f.decUIntV())
      {
         case 3:
         {
            f>>id>>pass>>access>>email>>name;
              reg_date.load(f);
            login_date.load(f);
            if(f.ok())return true;
         }break;

         case 2:
         {
            f>>id>>pass>>access; GetStr2(f, email); GetStr2(f, name);
              reg_date.load(f);
            login_date.load(f);
            if(f.ok())return true;
         }break;

         case 1:
         {
            GetStr(f, email); GetStr(f, name);
            f>>id>>pass>>access;
              reg_date.load(f);
            login_date.load(f);
            if(f.ok())return true;
         }break;

         case 0:
         {
            GetStr(f, email); GetStr(f, name);
            f>>id>>pass>>access;
            switch(access)
            {
               default: access=UA_NO_ACCESS; break;
               case  1: access=UA_READ_ONLY; break;
               case  2: access=UA_NORMAL   ; break;
               case  3: access=UA_ADMIN    ; break;
            }
              reg_date.load(f);
            login_date.load(f);
            if(f.ok())return true;
         }break;
      }
      return false;
   }
}
Memx<User> Users;
/******************************************************************************/
User* FindUser(C UID &id   ) {REPA(Users)if(Users[i].id   ==id   )return &Users[i]; return null;}
User* FindUser(C Str &email) {REPA(Users)if(Users[i].email==email)return &Users[i]; return null;}
/******************************************************************************/
REGISTER_RESULT RegisterUser(Str email, Str name, UID pass)
{
   if(!AllowUserRegistration())return REGISTER_REGISTRATION_UNAVAILABLE;
   if(!ValidEmail(email))return REGISTER_INVALID_EMAIL;
   if(  FindUser (email))return REGISTER_EMAIL_USED;
   User &user=Users.New();
   user.email=email;
   user.name =name;
   user.pass =pass;
   SaveSettings();
   UV.refresh();
   return REGISTER_SUCCESS;
}
bool CreateUser(Str email, Str name, Str pass, USER_ACCESS access)
{
   if(!ValidEmail(email)){Gui.msgBox(S, "Invalid email address"); return false;}
   if(!ValidPass (pass )){Gui.msgBox(S, "Invalid password"); return false;}
   if(  FindUser (email)){Gui.msgBox(S, "Email is already used"); return false;}
   User &user =Users.New();
   user.email =email;
   user.name  =name;
   user.pass  =PassToMD5(pass);
   user.access=access;
   SaveSettings();
   UV.refresh();
   return true;
}
FORGOT_PASS_RESULT ForgotPass(Str email)
{
   if(ValidEmail(email))if(User *user=FindUser(email))
   {
      user.change_pass_key=Random();
      return QueueMail(user.name, user.email, "Forgot Password", S+"Please use this key to reset your password:\r\n"+user.change_pass_key+"\r\n\r\nThis is an automated message - please do not reply!")
         ? FORGOT_PASS_KEY_SENT : FORGOT_PASS_NO_SEND_MAIL;
   }
   return FORGOT_PASS_EMAIL_NOT_FOUND;
}
CHANGE_PASS_RESULT ChangePassword(Str email, uint key, C UID &pass)
{
   if(ValidEmail(email))if(User *user=FindUser(email))
   {
      uint user_change_pass_key=user.change_pass_key; user.change_pass_key=0;
      if(user_change_pass_key && user_change_pass_key==key)
      {
         user.pass=pass;
         SaveSettings();
         return CHANGE_PASS_SUCCESS;
      }
      return CHANGE_PASS_INVALID_KEY;
   }
   return CHANGE_PASS_EMAIL_NOT_FOUND;
}
/******************************************************************************/
