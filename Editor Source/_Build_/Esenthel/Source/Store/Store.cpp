/******************************************************************************/
#include "stdafx.h"
/******************************************************************************/
StoreClass AppStore;
/******************************************************************************/

/******************************************************************************/
   cchar8 *StoreClass::categories_t[]=
   {
      "License",       // 0
      "Source Code",   // 1
      "Games",         // 2
      "Assets",        // 3
      "Contributions", // 4
   };
   cchar8 *StoreClass::source_code_cats[]=
   {
      "Games",
      "Tools",
   };
   cchar8 *StoreClass::game_cats[]=
   {
      "Shooter",
      "RPG",
      "Strategy",
      "Platform",
      "Racing",
      "Casual",
   };
   cchar8 *StoreClass::asset_cats[]=
   {
      "2D",
      "3D",
      "Audio",
   };
   cchar8 *StoreClass::item_t[]=
   {
      "Images",
      "Download",
    //"Extra",
   };
/******************************************************************************/
   Str StoreClass::ItemPath(int item_id) {return S+"http://www.esenthel.com/store/items/"+item_id+"/";}
   Str StoreClass::ItemRes(int item_id, C Str &file, C Str &name)
   {
      Str path=file;
      if( path.is())
      {
         if(!GetPath(path).is())path=ItemPath(item_id)+name+file;else
         if(!StartsPath(path, "http://") && !StartsPath(path, "https://"))path=S+"http://www.esenthel.com/"+path;
      }
      return path;
   }
         void StoreClass::Item::Temp::reset() {T=Temp();}
         StoreClass::Item::Temp::Temp(C TextNode &node) : ready(false), cat(-1), sub_cat(-1), price(-1), computer_id(-1), subscription(-1)
         {
            if(C TextNode *n=node.findNode("Ready"    ))ready  =n->asBool();
            if(C TextNode *n=node.findNode("Cat"      ))cat    =n->asInt ();
            if(C TextNode *n=node.findNode("Sub"      ))sub_cat=n->asInt ();
            if(C TextNode *n=node.findNode("CID"      ))computer_id=n->asInt();
            if(C TextNode *n=node.findNode("Price"    ))price  =n->asInt ();
            if(C TextNode *n=node.findNode("Subscr"   ))subscription=n->asInt();
            if(C TextNode *n=node.findNode("Name"     ))name   =n->asText();
            if(C TextNode *n=node.findNode("Video"    ))video  =n->asText();
            if(C TextNode *n=node.findNode("Icon"     ))icon   =n->asText();
            if(C TextNode *n=node.findNode("Files"    ))SetFiles(files, *n);
            if(C TextNode *n=node.findNode("Desc"     ))desc   =DecodeText(n->asText());
            if(C TextNode *n=node.findNode("DescShort"))desc_short=n->asText();

            if(C TextNode *n=node.findNode("Image_0"))image[0]=n->asText();
            if(C TextNode *n=node.findNode("Image_1"))image[1]=n->asText();
            if(C TextNode *n=node.findNode("Image_2"))image[2]=n->asText();
         }
      void StoreClass::Item::SetFiles(MemPtr<ItemFile> files, C TextNode &node)
      {
         FREPA(node.nodes)
         {
          C TextNode &src =node.nodes[i];
            ItemFile &file=files.New();
            if(C TextNode *n=src.findNode("name"))file.name=n->asText();
            if(C TextNode *n=src.findNode("file"))file.file=n->asText();
            if(C TextNode *n=src.findNode("free"))file.free=n->asBool();
         }
      }
      bool StoreClass::Item::XmlChar(char c) {return CharType(c)==CHART_CHAR || c=='-' || c==':';}
      Str StoreClass::Item::DecodeText(C Str &str)
      {
         Str out, tag;
         FREPA(str)
         {
            char c=str[i];
            if(c!='\n' && c!='\r')
            {
               if(c=='<')
               {
                  for(i++; ; i++)
                  {
                     c=str[i]; if(!c || c=='>')break;
                     tag+=c;
                  }
                  tag=SkipWhiteChars(tag);
                  FREPA(tag)if(!XmlChar(tag[i]) && !(i==0 && tag[i]=='/')){tag.clip(i); break;}
                  if(tag=="/h1")out+="\n";else
                  if(tag=="br" )out+="\n";else
                  if(tag=="li" )out+="- ";else 
                  if(tag=="/li")out.line();else
                  if(tag=="ul" )out.line();
                  tag.clear();
               }else
               if(c=='\t')out+="   ";else
               if(c==u'•' )out+='-'  ;else
                  out+=c;
            }
         }
         if(out.last()=='\n')out.removeLast();
         out=Replace(out, "&nbsp;", " ", true, true);
         return DecodeXmlString(out);
      }
      StoreClass::Item::Item() : requested_details(false), has_details(false), like(false), uses_computer_id(false), uses_github_account(false), has_temp(false), id(-1), price(0), category(0), sub_category(0), purchased(0), popularity(0), author(-1), likes(0), sold(0), developer_discount(0), subscription(0) {date_added.zero(); developer_date_joined.zero();}
      Str StoreClass::Item::buyText(bool include_price)C {Str t=((price>0 || price==-1) ? "Buy" : (price<0) ? "Contribute" : ""); if(include_price && price>0)t+=S+" ("+price_text+")"; return t;}
      bool StoreClass::Item::isNew() // if haven't been published yet
      {
         return price==0 && !icon_path.is() && !files.elms() && !likes && !sold && !video.is();
      }
      void StoreClass::Item::getDetails(bool force)
      {
         if(!requested_details || force)
         {
            requested_details=true;
            AppStore.sendCommand("get_item_details", "i", S+id);
         }
      }
      ::StoreClass::Item::Temp& StoreClass::Item::setTemp()
      {
         if(!has_temp)
         {
            has_temp=true;
            temp.desc=desc;
            temp.desc_short=desc_short;
            temp.video=video;
            if(id==AppStore.cur_item)AppStore.item_publish.show();
         }
         return temp;
      }
      void StoreClass::Item::setIcon(bool force)
      {
         if(!icon || force)
         {
            if(temp.icon=="keep" || !temp.icon.is())
            {
               if(icon_path)icon=IC.getImage(icon_path);
            }else
            {
               if(temp.icon!="null")icon=IC.getImage(ItemRes(id, temp.icon, "icon_temp."));
            }
            if(AppStore.cur_item==id)AppStore.item_icon.set(icon);
         }
      }
      void StoreClass::Item::setPrice(int price)
      {
         T.price=price;
         price_text=((price>0) ? TextReal(price/100.0f, 2)+" USD" : (price==0) ? S+"Free" : (price==-1) ? S+"Negotiable" : S);
         if(subscription>0)
         {
            price_text+=" / ";
            if(subscription==30 )price_text+="Month";else
            if(subscription==365)price_text+="Year" ;else 
                                 price_text+=S+subscription+" Day"+CountS(subscription);
         }
      }
      StoreClass::Item::Item(C TextNode &node) : requested_details(false), has_details(false), like(false), uses_computer_id(false), uses_github_account(false), has_temp(false), id(-1), price(0), category(0), sub_category(0), purchased(0), popularity(0), author(-1), likes(0), sold(0), developer_discount(0), subscription(0)
      {
         int p=0;
         if(C TextNode *n=node.findNode("id"))id=n->asInt();
         if(C TextNode *n=node.findNode("author"))author=n->asInt();
         if(C TextNode *n=node.findNode("pop"))popularity=Sqrt(n->asInt()/100);
         if(C TextNode *n=node.findNode("likes"))likes=n->asInt();
         if(C TextNode *n=node.findNode("subscr"))subscription=n->asInt(); // set before price
         if(C TextNode *n=node.findNode("price"))p=n->asInt(); if(C TextNode *n=node.findNode("cost"))p=n->asInt();
         if(C TextNode *n=node.findNode("cat"))category=n->asInt(); if(C TextNode *n=node.findNode("category"))category=n->asInt();
         if(C TextNode *n=node.findNode("sub"))sub_category=n->asInt(); if(C TextNode *n=node.findNode("subcategory"))sub_category=n->asInt();
         if(C TextNode *n=node.findNode("name"))name=n->asText();
         if(C TextNode *n=node.findNode("desc"))desc_short=n->asText();
         if(C TextNode *n=node.findNode("date"))date_added.fromText(n->asText());
         if(C TextNode *n=node.findNode("icon"))icon_path=ItemRes(id, n->asText(), "icon.");

         setPrice(p);
         time_ago=TimeAgo(date_added);
      }
      void StoreClass::Item::setDetails(C TextData &data)
      {
         has_details=true;
         files.clear();
         if(C TextNode *n=data.findNode("name" ))name=n->asText();
         if(C TextNode *n=data.findNode("subscr"))subscription=n->asInt(); // set before price
         if(C TextNode *n=data.findNode("price"))setPrice(n->asInt());
         if(C TextNode *n=data.findNode("cat"  ))category=n->asInt();
         if(C TextNode *n=data.findNode("sub"  ))sub_category=n->asInt();
         if(C TextNode *n=data.findNode("cid"  ))uses_computer_id=n->asBool();
         if(C TextNode *n=data.findNode("gha"  ))uses_github_account=n->asBool();
         if(C TextNode *n=data.findNode("sold" ))sold =n->asInt();
         if(C TextNode *n=data.findNode("likes"))likes=n->asInt();
         if(C TextNode *n=data.findNode("files"))SetFiles(files, *n);
         if(C TextNode *n=data.findNode("desc" ))desc=DecodeText(n->asText());
         if(C TextNode *n=data.findNode("DescShort"))desc_short=n->asText();
         if(C TextNode *n=data.findNode("video"))video=n->asText();
         if(C TextNode *n=data.findNode("icon"   ))icon_path    =ItemRes(id, n->asText(), "icon."   );
         if(C TextNode *n=data.findNode("image_0"))image_path[0]=ItemRes(id, n->asText(), "image_0.");
         if(C TextNode *n=data.findNode("image_1"))image_path[1]=ItemRes(id, n->asText(), "image_1.");
         if(C TextNode *n=data.findNode("image_2"))image_path[2]=ItemRes(id, n->asText(), "image_2.");
         if(C TextNode *n=data.findNode("DevName"    ))developer_name    =n->asText();
         if(C TextNode *n=data.findNode("DevPayPal"  ))developer_paypal  =n->asText();
         if(C TextNode *n=data.findNode("DevSupport" ))developer_support =n->asText();
         if(C TextNode *n=data.findNode("DevDiscount"))developer_discount=n->asInt ();
         if(C TextNode *n=data.findNode("DevDate"    ))developer_date_joined.fromText(n->asText());

         // purchases
         like=false;
         purchased=0;
         license_keys.clear();
         if(C TextNode *n=data.findNode("Purchases"))
         {
            purchased=n->nodes.elms();
            FREPA(n->nodes)
            {
             C TextNode &purchase=n->nodes[i];
               if(C TextNode *lk  =purchase.findNode("LicenseKey"))license_keys.line()+=lk->asText();
               if(C TextNode *like=purchase.findNode("Like"      ))if(like->asBool())T.like=true;
            }
         }

         // temp
         has_temp=false; temp.reset();
         if(C TextNode *n=data.findNode("Temp")){has_temp=true; temp=*n;}
         setIcon(true);

         // update gui
         if(AppStore.cur_item==id)AppStore.itemGotDetails(T);
      }
      void StoreClass::NewItem::Create(NewItem &new_item)
      {
         if(ValidText(new_item.name(), 1, 128))
         {
            AppStore.sendCommand("new_item", "item_name", new_item.name());
            new_item.hide();
         }else Gui.msgBox(S, "Invalid item name.");
      }
      ClosableWindow& StoreClass::NewItem::create()
      {
         ts.reset().size*=0.75f; ts.align.set(1, 0);
         ::EE::Window::create("New Item").hide(); button[2].show(); FlagDisable(flag, WIN_MOVABLE);
         T+=terms.create(
            "1. You certify that you have full rights to sell the items that you're submitting to Esenthel Store (you're the creator of the item or you have acquired all legal rights in order to sell it)\n"
            "2. Once item is submitted, it cannot be deleted (users that bought it, can access it forever)\n"
            "3. You will not encourage to purchase items from your own website instead of Esenthel Store\n"
            "4. Item price in Esenthel Store will not be higher than on your website\n"
            "5. Sales are done through PayPal, you need to have a PayPal account in order to get paid, the income will be decreased by PayPal fees\n"
            "6. 70% of sales will be sold through your PayPal account, 30% of sales will be sold through Esenthel PayPal account (which means that you get 70% income, and 30% goes to Esenthel Store)\n"
            "7. You will not include files or descriptions related to other game engines than Esenthel Engine\n"
            "8. After submitting items, they will be verified by Esenthel Staff before they will become available for sale, this process may take up to few days or more, depending on unforeseen circumstances\n"
            "9. Esenthel Store operators reserve the right to refuse acceptance of items to the store, or remove them afterwards at any time without any specific reason, especially if they consider the items to be offensive or in any way inappropriate\n"
            "10. Esenthel Store operators reserve the right to perform minor modifications to descriptions of the items that you have submitted\n"
            "11. You hold full responsibility for providing support to the customers which purchased your items\n"
            "12. You give permission to Esenthel Store owners to use the items (for making promotional images/videos/demos of the Esenthel Engine, including commercial games) as if they would've bought the item themselves according to the item's Terms of Use\n"
            "13. These terms may change from time to time, failure to accept the new changes will result in your items being no longer available for sale to new customers (old customers can still access their purchased items according to the old terms)\n"
            "14. Esenthel Store operators offer no warranty in regards to the Store itself (for example if the Store becomes unavailable; your items disappear from the Store listing; by some technical error they would get accessible for free or at a different price that you've specified - the issues will try to be fixed ASAP, however you're not entitled to any refund/compensation and you will not take any legal action against the Esenthel Staff)\n"
            "15. Items are by default sold to Users with the Terms listed on Esenthel Store Login Screen, if you wish however to specify your own Terms of Use for the items that you submit, you need to include them in the Item Description visible on the Item Page in Esenthel Store, so the users can read the Terms before they make the purchase."
            , &ts); terms.auto_line=AUTO_LINE_SPACE_SPLIT;
         T+=t_name.create("Item Name"); T+=name.create().maxLength(128);
         T+=b_create.create("Create").func(Create, T);
         return T;
      }
      ClosableWindow& StoreClass::NewItem::rect(C Rect &rect)
{
         ::EE::Window::rect(rect);
         t_name.rect(Vec2(clientWidth()/2-0.4f, -clientHeight()+0.06f)); name.rect(Rect_L(t_name.pos()+Vec2(0.13f, 0), 0.45f, 0.05f));
         b_create.rect(Rect_L(name.rect().right()+Vec2(0.02f, 0), 0.25f, 0.06f));
         terms.rect(Rect(0, name.rect().max.y, clientWidth(), 0).extend(-0.05f));
         return T;
      }
      ClosableWindow& StoreClass::NewItem::show()
{
         name.kbSet();
         ::EE::Window::show();
         return T;
      }
      void StoreClass::NewItem::update(C GuiPC &gpc)
{
         ::EE::ClosableWindow::update(gpc);
         if(Kb.k(KB_ENTER) && contains(Gui.kb()))b_create.push();
      }
      void StoreClass::Login::LoginDo(Login &login) {AppStore.loginDo(1, login.email(), login.pass());}
      void StoreClass::Login::ForgotPass(Login &login) {AppStore.loginDo(2, login.email(), login.pass());}
      void StoreClass::Login::Cancel(Login &login) {AppStore.b_login.set(false);}
      void StoreClass::Login::ShowTerms(Login &login)
      {
         Gui.msgBox("Esenthel Store Terms of Use",
         "1. All purchased items are non-refundable.\n"
         "2. Any purchased items and its license key may not be shared/distributed/sold, in full or in part, publicly or to any third party.\n"
         "3. Items can be distributed as part of an application provided that they're encrypted and not normally accessible (for example you can buy a '3D model' item, and include it in your 'game' application, however you need to keep the item file encrypted so only your game can access it, but other people can't).\n"
         "4. Providing access to your account to other people is forbidden.\n"
         "5. You can use the items worldwide without any additional fees or royalties.\n"
         "6. You can use the items for making as many products (games/videos/images/..) as you wish.\n"
         "7. If Esenthel Store or its items become unavailable, temporarily or permanently, you're not entitled to any refund of any kind (please keep a backup of the files).\n"
         "8. All items are provided without warranty or support of any kind.\n"
         "9. In no event will the operators of Esenthel Store be held liable for any damages arising from the use of the Store or its Products.\n"
         "\n"
         "Not complying with Esenthel Store Terms may result in termination of your account."
         );
      }
      ClosableWindow& StoreClass::Login::create()
      {
            ::EE::Window::create("Log In").hide().barVisible(false); FlagDisable(flag, WIN_MOVABLE); button[2].func(Cancel, T);
         T+=t_email    .create("E-mail"           ); T+=email.create();
         T+=t_pass     .create("Password"         ); T+=pass .create().password(true);
         T+=login      .create("Log In / Register").func(LoginDo   , T);
         T+=forgot_pass.create("Forgot Password"  ).func(ForgotPass, T).desc("Request password to be sent to your email");
         T+=terms      .create("Esenthel Store Terms of Use").func(ShowTerms, T);
         return T;
      }
      ClosableWindow& StoreClass::Login::rect(C Rect &rect)
{
         ::EE::Window::rect(rect);
         t_email    .rect(Vec2(0.07f, -0.05f)); email.rect(Rect_L(0.22f, -0.05f, 0.45f, 0.05f));
         t_pass     .rect(Vec2(0.11f, -0.11f)); pass .rect(Rect_L(0.22f, -0.11f, 0.45f, 0.05f));
         login      .rect(Rect_C(clientWidth()*1/4-0.00f, -clientHeight()+0.105f, 0.30f, 0.05f));
         forgot_pass.rect(Rect_C(clientWidth()*3/4-0.01f, -clientHeight()+0.105f, 0.31f, 0.05f));
         terms      .rect(Rect_C(clientWidth()*2/4     , -clientHeight()+0.040f, 0.62f, 0.05f));
         return T;
      }
      ClosableWindow& StoreClass::Login::show()
{
         if(email().is())pass.kbSet();else email.kbSet();
         ::EE::Window::show();
         return T;
      }
      void StoreClass::Login::update(C GuiPC &gpc)
{
         ::EE::ClosableWindow::update(gpc);
         if(Kb.k(KB_ENTER) && contains(Gui.kb()))login.push();
      }
      void StoreClass::Register::Yes(Register &reg) {AppStore.loginDo(0, AppStore.login_email, AppStore.login_pass);}
      void StoreClass::Register::Cancel(Register &reg) {reg.hide();}
      ::StoreClass::Register& StoreClass::Register::create(C Vec2 &pos)
      {
            ::EE::Window::create(Rect_C(pos, 0.9f, 0.4f), "Log In").hide(); button[2].show();
         T+=yes   .create(Rect_C(clientWidth()*1/4, -clientHeight()+0.08f, 0.25f, 0.055f), "Yes"   ).func(Yes   , T);
         T+=cancel.create(Rect_C(clientWidth()*3/4, -clientHeight()+0.08f, 0.25f, 0.055f), "Cancel").func(Cancel, T);
         T+=text  .create(Rect(0, yes.rect().max.y, clientWidth(), 0).extend(-0.05f)); text.auto_line=AUTO_LINE_SPACE_SPLIT;
         return T;
      }
      ::StoreClass::Register& StoreClass::Register::activate()
      {
         ::EE::GuiObj::activate();
         text.set(S+"User "+AppStore.login_email+" was not found.\nWould you like to register?");
         return T;
      }
      void StoreClass::ConfirmPayPal::Confirm(ConfirmPayPal &cp)
      {
         if(cp.paypal().is())
         {
            if(cp.paypal()==AppStore.e_dev_paypal())
            {
               if(AppStore.cur_dev==AppStore.user_id)AppStore.user_paypal=cp.paypal(); AppStore.cur_dev_paypal=cp.paypal(); AppStore.refreshDev(); AppStore.sendCommand("set_user_paypal", "user_id", S+AppStore.cur_dev, "user_paypal", cp.paypal());
               cp.hide();
            }else
            {
               Gui.msgBox(S, S+"You've typed a different PayPal than before:\n"+cp.paypal()+"\n"+AppStore.e_dev_paypal());
            }
         }
      }
      ::StoreClass::ConfirmPayPal& StoreClass::ConfirmPayPal::create()
      {
         ts.reset().size=0.039f;
            ::EE::Window::create(Rect_LU(0, 0, 0.9f, 0.22f), "Edit PayPal").barVisible(false).hide(); button[2].show(); FlagDisable(flag, WIN_MOVABLE);
         T+=paypal .create(Rect_L(0.03f, -clientHeight()+0.05f, 0.53f, 0.055f));
         T+=confirm.create(Rect  (paypal.rect().max.x+0.01f, paypal.rect().min.y, clientWidth()-0.03f, paypal.rect().max.y), "Confirm").func(Confirm, T);
         T+=text   .create(Rect(0, confirm.rect().max.y, clientWidth(), 0).extend(-0.03f), "Please confirm your PayPal email address.\nIf you type it wrong you will not be able to receive payments !!", &ts); text.auto_line=AUTO_LINE_SPACE_SPLIT;
         return T;
      }
      void StoreClass::ConfirmPayPal::setPos()
      {
         move(AppStore.e_dev_paypal.rect().up()+Vec2(0, 0.06f)-rect().down());
      }
      ::StoreClass::ConfirmPayPal& StoreClass::ConfirmPayPal::activate()
{
         ::EE::GuiObj::activate();
         paypal.clear().kbSet();
         setPos();
         return T;
      }
      void StoreClass::ConfirmPayPal::update(C GuiPC &gpc)
{
         ::EE::ClosableWindow::update(gpc);
         if(gpc.visible && visible())
         {
            setPos();
            if(Kb.k(KB_ENTER) && contains(Gui.kb()))confirm.push();
         }
      }
      void StoreClass::BecomeSeller::EngineLicense(BecomeSeller &bs) {bs.hide(); AppStore.openItem(-1); AppStore.categories.set(0);}
      ::StoreClass::BecomeSeller& StoreClass::BecomeSeller::create(C Vec2 &pos)
      {
         ts.reset().size=0.04f; ts.align.set(1, 0);
         ::EE::Window::create(Rect_C(pos, 1.32f, 0.30f), "Seller").barVisible(false).hide(); button[2].show();
         T+=engine_license.create(Rect_C(clientWidth()/2, -clientHeight()+0.06f, 0.5f, 0.055f), "Buy Engine License").func(EngineLicense, T);
         T+=text          .create(Rect(0, engine_license.rect().max.y, clientWidth(), 0).extend(-0.05f), "In order to sell your own items in Esenthel Store you need to be a Licensed Developer (have an Esenthel Engine License).\nOnce you're a Licensed Developer, you will be able to submit your own items to Esenthel Store and gain income from their sales.", &ts); text.auto_line=AUTO_LINE_SPACE_SPLIT;
         return T;
      }
      void StoreClass::Contribute::Buy(Contribute &contr)
      {
         dbl cd=TextDbl(contr.amount());
         int c =Round(cd*100);
         if( c>=10*100)
         {
            contr.hide();
            Explore(EsenthelStoreURL+"?cmd=buy_item&i="+AppStore.cur_item+"&u="+AppStore.user_id+"&contribute="+c);
         }else Gui.msgBox(S, "Minimum amount to contribute is 10 $");
      }
      ClosableWindow& StoreClass::Contribute::create()
      {
         ::EE::Window::create(Rect_LU(0, 0, 0.7f, 0.23f), "Contribute").barVisible(false).hide(); button[2].show(); FlagDisable(flag, WIN_MOVABLE);
         ts.reset().size=0.045f;
         T+=amount    .create(Rect_C(clientWidth()*1/4, -clientHeight()+0.05f, 0.25f, 0.055f)).maxLength(32);
         T+=contribute.create(Rect_C(clientWidth()*3/4, -clientHeight()+0.05f, 0.25f, 0.055f), "Contribute").func(Buy, T);
         T+=text.create(Rect(0, amount.pos().y, clientWidth(), 0).extend(-0.05f), "Please enter the amount of USD which you'd like to contribute", &ts); text.auto_line=AUTO_LINE_SPACE_SPLIT;
         return T;
      }
      void StoreClass::Contribute::update(C GuiPC &gpc)
{
         ::EE::ClosableWindow::update(gpc);
         if(Kb.k(KB_ENTER) && contains(Gui.kb()))contribute.push();
      }
      void StoreClass::ItemList::draw(C GuiPC &gpc)
{
         if(visible() && gpc.visible)
         {
            Rect  clip=gpc.clip; if(!columnsHidden())clip.max.y-=columnHeight();
            VecI2 visible_elms=visibleElmsOnScreen(&gpc);

            for(int i=visible_elms.x  ; i<=visible_elms.y  ; i++)if(Item *item=T(i))item->setIcon(); // first set those seen
            for(int i=visible_elms.x-5; i<=visible_elms.y+5; i++)if(Item *item=T(i))item->setIcon(); // then  set those nearby

            // draw back images
            if(drawMode()==LDM_RECTS)
            {
               D.clip(clip);
               for(int i=visible_elms.x; i<=visible_elms.y; i++)
               {
                  Rect rect=visToScreenRect(i); flt w=0.02f; rect.extend(-w*0.7f);
                  D.drawShadow(16, rect, w*1.5f);
                  rect.extend(w*0.4f).draw(Gui.borderColor(), false);
               }
            }

            // draw images and selection rectangles
            ::EE::_List::draw(gpc);

            // draw names and descriptions
            if(drawMode()==LDM_RECTS)
            {
               Rect padd=imageSizePadding()*zoom();
               TextStyleParams ts(true); ts.size=0.038f*Min(1.2f, zoom()); ts.align.set(0, -1); ts.color.a=220;
               TextStyleParams ts_small=ts; ts_small.align.set(-1, 1); ts_small.color.a=128; ts_small.size*=0.91f;
               for(int i=visible_elms.x; i<=visible_elms.y; i++)
               {
                  if(Item *item=T(i))
                  {
                     Rect rect=visToScreenRect(i);
                     rect.min+=padd.max;
                     rect.max+=padd.min; rect.max.y=rect.min.y-padd.min.y-padd.max.y;
                     D.clip(clip&rect);
                     rect.extendX(zoom()*-0.006f);
                                        D.text(ts      , rect, item->name      , AUTO_LINE_SPACE_SPLIT);
                     if(!item->purchased)D.text(ts_small, rect, item->price_text, AUTO_LINE_SPACE_SPLIT);else AppStore.ok_image->drawFit(Rect_RD(rect.rd(), 0.08f*zoom()));
                  }
               }
            }
         }
      }
      void StoreClass::Purchase::Copy(Purchase &p) {ClipSet(p.license_key);}
      void StoreClass::Purchase::create(GuiObj &parent, C Str &license_key, bool computer_id, bool github_account)
      {
                                   T.  license_key=license_key;
                           parent+=T.t_license_key.create(S+"License Key: "+license_key, &AppStore.ts_desc);
                           parent+=T.copy.create("Copy").func(Copy, T).desc("Copy License Key to system clipboard");
         if(computer_id   )parent+=T.set_computer_id   .create("Set Computer ID for this Key"   ).func(ItemWebsite, AppStore);else T.set_computer_id   .del();
         if(github_account)parent+=T.set_github_account.create("Set GitHub account for this Key").func(ItemWebsite, AppStore);else T.set_github_account.del();
      }
         void StoreClass::DownloadFile::WindowIOEx::Hidden(WindowIOEx &w) {AppStore.clearDownloadFiles();}
         WindowIO& StoreClass::DownloadFile::WindowIOEx::hide()
{
            ::EE::WindowIO::hide();
            Gui.addFuncCall(Hidden, T);
            return T;
         }
         GuiObj* StoreClass::DownloadFile::ProgressNoTest::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel){return null;}
      void StoreClass::DownloadFile::Open(DownloadFile &df) {Explore(df.dest, true   ); Cancel(df);}
      void StoreClass::DownloadFile::Import(DownloadFile &df) {CopyElms.display(df.dest); Cancel(df);}
      void StoreClass::DownloadFile::Cancel(DownloadFile &df) {AppStore.download_files.removeData(&df);}
      void StoreClass::DownloadFile::Retry(DownloadFile &df) {df.download(true);}
             void StoreClass::DownloadFile::download(bool resume)
      {
         retry.hide();
         if(resume)
         {
            if(down.done())
            {
               file_done.put(down.data(), down.done()); // remember what was written
            }
         }else
         {
            file_size       =-1;
            file_modify_time.zero();
            file_done       .writeMem();
         }

         if(ValidURL(file))down.create(file, null, null, -1, file_done.size());else
         {
            Memt<HTTPParam> params;
            AppStore.setUserParams(params, "get_item_file");
            params.New().set("i"   , S+item_id);
            params.New().set("file",   file+(temp ? "@new" : ""));
            down.create(EsenthelStoreURL, params, null, -1, file_done.size());
         }
      }
      void StoreClass::DownloadFile::Select(C Str &name, DownloadFile &df) {df.select(name);}
             void StoreClass::DownloadFile::select(C Str &name)
      {
         dest=name;
      #if 0
         dest.tailSlash(true)+=file;
      #endif
         download(false);
         Gui+=::EE::Window::create(Rect_C(0, 0, 0.8f, 0.21f), S+"Downloading \""+GetBase(file)+'"');
         T+=progress.create(Rect_LU(0, 0, clientWidth(), 0.09f).extend(-0.02f));
         ts.reset(); ts.size=0.045f;
         T+=text.create(progress.rect().center(), S, &ts);
         T+=open  .create(Rect_C(clientWidth()*1/6, -clientHeight()+0.04f, 0.21f, 0.055f), "Open"  ).func(Open  , T).hide();
         T+=import.create(Rect_C(clientWidth()*3/6, -clientHeight()+0.04f, 0.21f, 0.055f), "Import").func(Import, T).hide();
         T+=retry .create(Rect_C(clientWidth()*1/6, -clientHeight()+0.04f, 0.21f, 0.055f), "Retry" ).func(Retry , T).hide();
         T+=cancel.create(Rect_C(clientWidth()*5/6, -clientHeight()+0.04f, 0.21f, 0.055f), "Cancel").func(Cancel, T);
      }
      bool StoreClass::DownloadFile::valid()C {return win_io.visible() || visible();}
      void StoreClass::DownloadFile::create(int item_id, C Str &file, bool temp)
      {
         T.item_id=item_id;
         T.file   =file;
         T.temp   =temp;
      #if 1
         win_io.create(GetExt(file), S, SystemPath(SP_DESKTOP)).io(Select, Select, T).save(); win_io.textline.set(GetBase(file));
      #else
         win_io.create(S, SystemPath(SP_DESKTOP)).io(Select, Select, T).modeDirSelect().activate();
      #endif
      }
      void StoreClass::DownloadFile::update(C GuiPC &gpc)
{
         ::EE::Window::update(gpc);

         if(down.totalSize()>=0) // if size is known
         {
            if(file_size<0)file_size=down.totalSize();
            if(file_size!=down.totalSize())download(false); // restart
         }
         if(down.modifyTimeUTC().valid()) // if modify time is known
         {
            if(!file_modify_time.valid())file_modify_time=down.modifyTimeUTC();
            if( file_modify_time!=down.modifyTimeUTC())download(false); // restart
         }

         switch(down.state())
         {
            case DWNL_DONE:
            {
               if(!saved)
               {
                  saved=true;
                  if(file_done.size()){file_done.put(down.data(), down.done()); file_done.pos(0);}
                  File temp(down.data(), down.done()), &src=(file_done.size() ? file_done : temp);
                  if(SafeOverwrite(src, dest))
                  {
                     open.show(); cancel.text="Close"; if(GetExt(file)==EsenthelProjectExt)import.show();
                     text.set(S+"Done ("+FileSize(down.totalSize())+")");
                  }else text.set("Failed to save file");
                  progress.set(1);
                //flash();
               }
            }break;
            case DWNL_DOWNLOAD: {long done=file_done.size()+down.done(); text.set(FileSize(done)+" / "+(down.totalSize()>=0 ? FileSize(down.totalSize()) : S+"?")); progress.set(done, down.totalSize());} break;
            case DWNL_ERROR   : if(down.done())retry.show(); text.set("Error occured"); break; // if downloaded anything
            default           : text.set("Connecting"); break;
         }
      }
      bool StoreClass::ItemFile::ItemDownloadable()
      {
         if(Item *item=AppStore.findItem(AppStore.cur_item))
            return AppStore.isAdmin() || (AppStore.user_id>=0 && item->author==AppStore.user_id) || item->purchased || item->price==0; // admin or author or purchased or free
         return false;
      }
      void StoreClass::ItemFile::Download(ItemFile &df)
      {
         if(ItemDownloadable() || df.is_free)
         {
            if(StartsPath(df.file, "http://") || StartsPath(df.file, "https://")) // check if this is a link to item in App Stores
            {
               Str s=GetStart(SkipStart(TextPos(df.file, ':')+3, "www."));
               if(Contains(s, "steampowered.com", false, true)
               || Contains(s,    "microsoft.com", false, true)
               || Contains(s,       "google.com", false, true)
               || Contains(s,        "apple.com", false, true)
               || Contains(s,       "amazon.com", false, true)
               || Contains(s,      "samsung.com", false, true)){Explore(df.file); return;}
            }
         #if 0
            if(ValidURL(df.file))Explore(df.file);else // open through web browser
         #endif
            if(ValidURL(df.file) || CleanFileName(df.file)==df.file)AppStore.download_files.New().create(AppStore.cur_item, df.file, df.temp);
         }else Gui.msgBox(S, "You need to buy the item to download this file");
      }
      void StoreClass::ItemFile::Rename(ItemFile &df) {AppStore.rename_file.activate(df);}
      void StoreClass::ItemFile::Remove(ItemFile &df) {AppStore.remove_file.activate(df);}
      void StoreClass::ItemFile::SetFree(ItemFile &df) {AppStore.sendCommand("set_item_file_access", "i", S+AppStore.cur_item, "item_file_file", df.file, "item_file_free", S+!df.is_free);}
      void StoreClass::ItemFile::create(GuiObj &parent, C Item::ItemFile &file, bool temp)
      {
         T.temp=temp;
         T.name=file.name;
         T.file=file.file;
         T.is_free=file.free;
         bool downloadable=(ItemDownloadable() || is_free);
         Str desc=S+"File: \""+file.file+"\""; if(!ItemDownloadable() && is_free)desc.line()+="This file is accessible for free"; if(downloadable)desc.line()+="Click to download";else desc.line()+="You need to buy the item to download this file";
                 parent+=download.create(name    ).func(Download, T).setImage(downloadable ? AppStore.ok_image : AppStore.fail_image).desc(desc); download.skin=(downloadable ? null : &HalfTransparentSkin);
         if(temp)parent+=rename  .create("Rename").func(Rename  , T).desc("Rename this file");else rename.del();
         if(temp)parent+=remove  .create("Remove").func(Remove  , T).desc("Delete this file");else remove.del();
         if(temp)parent+=free    .create(is_free ? "Set paid access" : "Set access for free").func(SetFree, T).desc("Make this file to be accessible for free.\nNormally files are accessible only after the item is purchased.\nHowever you may want to include some files to preview for free, before purchasing.\nAfter clicking this button, the file will become accessible for free.");else free.del();
      }
      void StoreClass::RenameFile::OK(RenameFile &rf) {rf.ok();}
             void StoreClass::RenameFile::ok()
      {
         if(ValidText(new_name(), 1, 256))
         {
            if(!Equal(new_name(), name, true))AppStore.sendCommand("rename_item_file", "i", S+AppStore.cur_item, "item_file_file", file, "item_file_name", new_name());
            hide();
         }else Gui.msgBox(S, "Invalid name");
      }
      ::StoreClass::RenameFile& StoreClass::RenameFile::create(C Vec2 &pos)
      {
         Vec2 size(0.78f, 0.05f), padd=defaultInnerPaddingSize()+0.02f;
         ::EE::Window::create(Rect_LU(pos, size+padd)); button[2].show();
         T+=new_name.create(Rect_LU(0.01f, -0.01f, size.x, size.y)).maxLength(256);
         return T;
      }
      ::StoreClass::RenameFile& StoreClass::RenameFile::activate(ItemFile &df)
      {
         T.file=df.file;
         T.name=df.name;
         setTitle(S+"Rename \""+name+"\"");
         ::EE::GuiObj::activate();
         new_name.set(df.name).selectAll().kbSet();
         return T;
      }
      void StoreClass::RenameFile::update(C GuiPC &gpc)
{
         ::EE::ClosableWindow::update(gpc);
         if(Gui.window()==this && Kb.k(KB_ENTER)){ok(); Kb.eatKey();}
      }
      void StoreClass::RemoveFile::OK(RemoveFile &fr) {fr.hide(); AppStore.sendCommand("remove_item_file", "i", S+AppStore.cur_item, "item_file_file", fr.file);}
      void StoreClass::RemoveFile::activate(ItemFile &df)
      {
         T.file=df.file;
         setTitle(S+"Remove File \""+df.name+"\"");
         ::EE::GuiObj::activate();
      }
      ::StoreClass::RemoveFile& StoreClass::RemoveFile::create(C Vec2 &pos)
      {
             ::EE::Window::create(Rect_C(pos, 0.72f, 0.32f), "Remove File").hide(); button[2].show();
         T+=t_name.create(Rect_C(clientWidth()/2, -0.09f, 0.7f, 0.0f), "Are you sure you wish to remove selected file?"); t_name.auto_line=AUTO_LINE_SPACE_SPLIT;
         T+=    ok.create(Rect_C(clientWidth()/2, -0.20f, 0.3f, 0.06f), "OK").func(OK, T);
         return T;
      }
      void StoreClass::PublishConfirm::OK(PublishConfirm &pc)
      {
         pc.hide();
         if(Item *item=AppStore.findItem(AppStore.cur_item)){item->setTemp().ready=true; AppStore.item_publish.set(item->temp.ready, QUIET); AppStore.filter();}
         AppStore.sendCommand("set_item_ready", "i", S+AppStore.cur_item, "item_ready", "1");
      }
      void StoreClass::PublishConfirm::publish()
      {
         if(Item *item=AppStore.findItem(AppStore.cur_item))
         {
            int price=((item->temp.price>=0) ? item->temp.price : item->price);
            if(!price && item->isNew()){AppStore.item_publish.set(item->temp.ready, QUIET); activate();}else OK(T); // haven't specified a price and is new -> ask for confirmation
         }
      }
      ::StoreClass::PublishConfirm& StoreClass::PublishConfirm::create(C Vec2 &pos)
      {
           ::EE::Window::create(Rect_C(pos, 1.00f, 0.35f), "Confirm Publishing").hide(); button[2].show();
         T+=ok  .create(Rect_C(clientWidth()/2, -clientHeight()+0.07f, 0.3f, 0.06f), "OK").func(OK, T);
         T+=text.create(Rect(0, ok.rect().max.y, clientWidth(), 0).extend(-0.05f), "You haven't specified the price for your item.\nAre you sure you wish to publish it for free?"); text.auto_line=AUTO_LINE_SPACE_SPLIT;
         return T;
      }
      void StoreClass::Upload::Cancel(Upload &up) {AppStore.store_uploads.removeData(&up);}
      void StoreClass::Upload::create(GuiObj &parent, int item_id, C Str &name, C Str &display)
      {
         T.item_id=item_id;
         T.display=display; T.display.space();
         T.name=name;
         ts.reset(); ts.size=0.038f;
                         parent+=progress.create();
         if(display.is())parent+=text    .create(S, &ts);
      }
      void StoreClass::Upload::rect(C Rect &rect)
      {
         progress.rect(rect);
         text    .rect(rect);
      }
      void StoreClass::Upload::pos(C Vec2 &pos)
      {
         rect(Rect_LU(pos, 1.0f, 0.045f)); cancel.rect(Rect_LU(progress.rect().ru()+Vec2(0.01f, 0), 0.17f, 0.045f));
      }
      bool StoreClass::Upload::checkChunked()
      {
         if(chunked)
         {
            sent+=down.sent();
            if(!src.end()){transfer(); return false;} // proceed to next chunk
         }
         return true;
      }
      void StoreClass::Upload::update()
      {
         long sent=T.sent+down.sent(), size=src.size();
         text.set(display+"("+FileSize(sent)+" / "+FileSize(size)+")");
         progress.set(sent, size);
         visible(item_id==AppStore.cur_item);
      }
      void StoreClass::Upload::transfer()
      {
         Memt<HTTPParam> params; params=T.params;
         if(chunked)
         {
            int mode;
            if(src.pos ()==0)mode=1;else // first
            if(src.left()<=EsenthelStoreFileChunkSize)mode=3;else // last
               mode=2; // middle
            params.New().set("item_file_mode", S+mode);
         }
         down.create(EsenthelStoreURL, params, &src, chunked ? Min(EsenthelStoreFileChunkSize, src.left()) : -1);
      }
      bool StoreClass::Upload::visible()C {return progress.visible();}
      void StoreClass::Upload::visible(bool on)
      {
         progress.visible(on);
         text    .visible(on);
         cancel  .visible(on);
      }
      void StoreClass::ItemImage::setEditable(bool editable)
      {
         T.editable=editable;
         rect_color.a=(editable ? 32 : 0);
         desc(editable ? "Drag and drop an image here" : "");
      }
      ::StoreClass::ItemImage& StoreClass::ItemImage::create(bool hoverable)
      {
         ::EE::GuiImage::create();
         T.hoverable=hoverable;
         fit=true;
         return T;
      }
      GuiObj* StoreClass::ItemImage::test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel){return (editable || hoverable) ? ::EE::GuiObj::test(gpc, pos, mouse_wheel) : null;}
      void StoreClass::ItemImage::draw(C GuiPC &gpc)
{
         if(gpc.visible && visible())
         {
            D.clip(gpc.clip);
            Rect r=rect()+gpc.offset;
            if(image)image->drawFilter(image->fit(r));
            if(editable && !image)
            {
               TextStyleParams ts(true); ts.size=0.04f; ts.color.a=128;
               D.text(ts, rect()+gpc.offset, "Drag and drop an image here", AUTO_LINE_SPACE_SPLIT);
            }
            if(rect_color.a)r.draw(rect_color, false);
         }
      }
      void StoreClass::Line::draw(C GuiPC &gpc)
{
         D.clip(gpc.clip);
         Rect rect=T.rect()+gpc.offset;
         D.line(Gui.borderColor(), rect.min, rect.max);
      }
   void StoreClass::ItemWebsite(StoreClass &store)
   {
      if(store.cur_item>=0)Explore(S+"http://www.esenthel.com/?id=store&item="+store.cur_item);
   }
   void StoreClass::ItemVideo(StoreClass &store)
   {
      if(Item *item=store.findItem(store.cur_item))
         if(StartsPath(item->video, "http://www.youtube.com") || StartsPath(item->video, "https://www.youtube.com"))
            Explore(YouTubeEmbedToFull(item->video));
   }
   void StoreClass::ItemCID(StoreClass &store)
   {
      if(Item *item=store.findItem(store.cur_item))
      {
         item->setTemp().computer_id=store.item_cid();
         store.sendCommand("set_item_computer_id", "i", S+item->id, "item_computer_id", S+store.item_cid());
      }
   }
   void StoreClass::ItemLike(StoreClass &store)
   {
      if(Item *item=store.findItem(store.cur_item))
         if(item->purchased)
            store.sendCommand("like_item", "i", S+item->id, "like", S+!item->like);
   }
   void StoreClass::ItemRefresh(StoreClass &store)
   {
      if(Item *item=store.findItem(store.cur_item))
         item->getDetails(true);
   }
   void StoreClass::ItemPublish(StoreClass &store)
   {
      store.publish_confirm.publish();
   }
   void StoreClass::ItemCategory(StoreClass &store)
   {
      int cat, sub; store.getCategory(cat, sub);
      if(Item *item=store.findItem(store.cur_item))
      {
         item->setTemp();
         item->temp.cat=cat;
         item->temp.sub_cat=sub;
      }
      store.sendCommand("set_item_category", "i", S+store.cur_item, "item_category", S+cat, "item_sub_category", S+sub);
   }
   void StoreClass::Buy(StoreClass &store)
   {
      if(Item *item=store.findItem(store.cur_item))
      {
         if(item->price==-1) // negotiable
         {
            Gui.msgBox(S, "Please contact \"esenthel@esenthel.com\"");
         }else
         if(!store.loggedIn())
         {
            store.b_login.set(true);
            Gui.msgBox(S, S+"Please log in to "+item->buyText(false));
         }else
         if(store.cur_item<=0)
         {
            Gui.msgBox(S, "Invalid item");
         }else
         if(item && item->price==-2)
         {
            store.contribute.activate();
         }else
         {
            Gui.msgBox(S, "After making the purchase please push the \"Refresh\" button.\nThis will show your License Key and activate the downloads.");
            Explore(EsenthelStoreURL+"?cmd=buy_item&i="+store.cur_item+"&u="+store.user_id);
         }
      }
   }
   void StoreClass::DeveloperPage(StoreClass &store) {if(Item *item=store.findItem(store.cur_item))store.openDeveloper(item->author, item->developer_name, item->developer_support, item->developer_paypal, item->developer_date_joined, item->developer_discount);}
   void StoreClass::Back(StoreClass &store) {store.openItem(-1);}
   void StoreClass::Filter(StoreClass &store) {store.filter();}
   void StoreClass::Readies(StoreClass &store) {if(store.readies())store.categories.set(-1); store.filter();}
   bool StoreClass::Check(Download &down)
   {
      if(down.state()==DWNL_ERROR)
      {
         Gui.msgBox("Esenthel Store", "Can't connect to the server.\nPlease try again in a moment.");
         down.del();
      }
      return down.state()==DWNL_DONE;
   }
   Str StoreClass::MD5Text(C Str8 &text)
   {
      UID id=MD5Mem(text(), text.length()); return TextHexMem(&id, SIZE(id));
   }
   void StoreClass::SetUserParams(MemPtr<HTTPParam> params, C Str &user, C Str &password, int key, C Str &cmd)
   {
      uint request_id=Random();
      DateTime utc, epoch; utc.getUTC(); epoch.zero(); epoch.year=1970; epoch.month=1; epoch.day=1; int time=((utc-epoch)/60/5); // 5 mins
      params.New().set("u", user);
      params.New().set("k", MD5Text(CaseDown(password+key+request_id+time)));
      params.New().set("r", S+request_id);
      if(cmd.is())params.New().set("cmd", cmd);
   }
   void StoreClass::OpenSupport(C Str &support) {if(ValidEmail(support)){ClipSet(support); Gui.msgBox(S, S+"Support email\n\""+support+"\"\nhas been copied to clipboard.");}else if(ValidURL(support))Explore(support);}
   void StoreClass::ItemSupport(StoreClass &store) {if(Item *item=store.findItem(store.cur_item))OpenSupport(item->developer_support);}
   void StoreClass::DevSupport(StoreClass &store) {OpenSupport(store.cur_dev_support);}
   void StoreClass::CloseDevPage(StoreClass &store) {store.openDeveloper(-1);}
   void StoreClass::Account(StoreClass &store) {if(store.cur_dev==store.user_id && store.mode()==0)store.openDeveloper(-1);else store.openDeveloper(store.user_id, store.user_name, store.user_support, store.user_paypal, store.user_date, store.user_discount);}
   void StoreClass::NewItemDo(StoreClass &store)
   {
      if(!store.loggedIn())
      {
         store.b_login.set(true);
         Gui.msgBox(S, "Please log in to create new items in the store.");
      }else
      if(!store.user_name.is() || !store.user_paypal.is() || !store.user_support.is())
      {
         if(store.cur_dev!=store.user_id)Account(store);
         Str msg;
         if(!store.user_name   .is())msg.line()+="Please set your Developer Name";
         if(!store.user_paypal .is())msg.line()+="Please set your Developer PayPal email address";
         if(!store.user_support.is())msg.line()+="Please set your Developer Support Contact (website or email)";
         Gui.msgBox(S, msg);
      }else
      if(!store.isSeller())
      {
         store.become_seller.activate();
      }else
         store.new_item.activate();
   }
   void StoreClass::LoginToggle(StoreClass &store)
   {
      store.login.visibleActivate(store.b_login());
   }
   void StoreClass::Logout(StoreClass &store) {store.logout();}
   ::StoreClass::Item* StoreClass::findItem(int id) {if(id>0)REPA(items)if(items[i].id==id)return &items[i]; return null;}
   bool StoreClass::loggedIn()C {return user_id>=0;}
   bool StoreClass::isAdmin()C {return user_id==1;}
   bool StoreClass::isPurchased(int id) {if(Item *item=findItem(id))return item->purchased>0; return false;}
   bool StoreClass::isSeller()C {return loggedIn() && user_seller;}
   void StoreClass::clearDownloadFiles() {REPA(download_files)if(!download_files[i].valid())download_files.removeValid(i);}
   void StoreClass::setUserParams(MemPtr<HTTPParam> params, C Str &cmd) {SetUserParams(params, S+user_id, user_pass, user_key, cmd);}
   void StoreClass::sendCommand(C Str &cmd, C Str &name, C Str &value, C Str &name2, C Str &value2, C Str &name3, C Str &value3)
   {
      Memt<HTTPParam> params;
      setUserParams(params, cmd);
      if(name .is())params.New().set(name , value );
      if(name2.is())params.New().set(name2, value2);
      if(name3.is())params.New().set(name3, value3);
      store_commands.New().create(EsenthelStoreURL, params);
   }
   void StoreClass::sendCommandPost(C Str &cmd, C Str &name, C Str &value, C Str &post_name, C Str &post_value)
   {
      Memt<HTTPParam> params;
      setUserParams(params, cmd);
      if(     name.is())params.New().set(     name,      value);
      if(post_name.is())params.New().set(post_name, post_value, HTTP_POST);
      store_commands.New().create(EsenthelStoreURL, params);
   }
   void StoreClass::setLogin()
   {
      if(loggedIn())
      {
         hello_user.set(S+"Hello "+(user_name.is() ? user_name : user_email)+"!");
         b_login.set(false);
         account.show();
        _register.hide();
      }else
      {
         hello_user.set("Hello Guest!");
         account.hide();
         become_seller.hide();
         contribute.hide();
         rename_file.hide();
         remove_file.hide();
         publish_confirm.hide();
         confirm_paypal.hide();
      }
      b_login .visible(!loggedIn());
      b_logout.visible( loggedIn());
      readies .visible(  isAdmin()); // only admin sees this
      if(readies.hidden())readies.set(false, QUIET); // quietly disable if hidden
   }
   void StoreClass::loginDo(int mode, C Str &email, C Str &pass)
   {
     _register.hide();
      if(mode==2 || ValidPass(pass))
      {
         Memt<HTTPParam> params;
         SetUserParams(params, email, pass, 0, mode==0 ? "register" : mode==1 ? "log_in" : "forgot_pass");
         if(mode==0)params.New().set("p", pass, HTTP_POST);
         store_commands.New().create(EsenthelStoreURL, params);
         login_email=email;
         login_pass =pass ;
      }else Gui.msgBox(S, "Password length must be in range of 4-16 characters and may not contain unicode characters.");
   }
   void StoreClass::logout()
   {
       user_id =-1;
       user_key= 0;
       user_discount=0;
       user_seller=false;
       user_name   .clear();
       user_email  .clear();
       user_pass   .clear();
       user_support.clear();
       user_paypal .clear();
       user_date   .zero ();
      login_email  .clear();
      login_pass   .clear();
      login.pass   .clear();
      setLogin();
      openItem(-1);
      refreshDev();
      downloadItems(); // redownload items after logging out
   }
   void StoreClass::setCategory(int cat, int sub)
   {
      item_category.set(cat, QUIET);
      if(InRange(cat, categories_node.children))
      {
         Memx<Node<MenuElm> > &subs=categories_node.children[cat].children;
         if(InRange(sub, subs))item_category.text=subs[sub].name;
      }
   }
   void StoreClass::getCategory(int &cat, int &sub)
   {
      cat=item_category();
      sub=0;
      if(InRange(cat, categories_node.children))
      {
         Memx<Node<MenuElm> > &subs=categories_node.children[cat].children;
         REPA(subs)if(subs[i].name==item_category.text){sub=i; break;}
      }
   }
   void StoreClass::create()
   {
      // categories
      FREPA(categories_t    )categories_node+=categories_t[i];
      FREPA(source_code_cats)categories_node.children[1]+=source_code_cats[i];
      FREPA(       game_cats)categories_node.children[2]+=       game_cats[i];
      FREPA(      asset_cats)categories_node.children[3]+=      asset_cats[i];

      // all header
      ListColumn lc[]=
      {
         ListColumn(MEMBER(Item, icon), 0.29f, "Toggle Details"), // 0
         ListColumn(MEMBER(Item, popularity), 0.35f, "Sort by Popularity"), // 1
         ListColumn(MEMBER(Item, time_ago), 0.27f, "Sort by Date"), // 2
         ListColumn(MEMBER(Item, name), 0.29f, "Sort by Name"), // 3
         ListColumn(MEMBER(Item, price_text), 0.28f, "Sort by Price"), // 4
       //ListColumn(MEMBER(Item, likes), 0.28, "Sort by Likes"), // 5
      }; lc[2].sort=&date_sort.set(MEMBER(Item, date_added)); lc[4].sort=&price_sort.set(MEMBER(Item, price));

      DYNAMIC_ASSERT(Elms(sub_categories)==Elms(categories_t), "categories elms");
      Gui+=::EE::Window::create("Esenthel Store").hide(); button[1].show(); button[2].show(); flag|=WIN_RESIZABLE;
      T+=mode.create((cchar**)null, 2).set(0); REPA(mode)mode.tab(i).hide();
      T+=hello_user.create();
      T+=line.create();

      // item list page
      mode.tab(0)+=filter_name.create().func(Filter, T); filter_name.kbClear(); filter_name.reset.show(); filter_name.show_find=true; filter_name.hint="Find";
      mode.tab(0)+=categories.create(categories_t, Elms(categories_t)).set(0).func(Filter, T);
      categories.tab(1)+=sub_categories[1].create(source_code_cats, Elms(source_code_cats)).func(Filter, T);
      categories.tab(2)+=sub_categories[2].create(       game_cats, Elms(       game_cats)).func(Filter, T);
      categories.tab(3)+=sub_categories[3].create(      asset_cats, Elms(      asset_cats)).func(Filter, T);
      mode.tab(0)+=purchased.create("Bought").func(Filter, T).desc("Display only the items which you have purchased"); purchased.mode=BUTTON_TOGGLE;
      mode.tab(0)+=readies.create("Readies").func(Readies, T).desc("Display items ready for publishing").hide(); readies.mode=BUTTON_TOGGLE;
      mode.tab(0)+=items_region.create(); items_region.kb_lit=false;
      flt padd=0.01f;
      items_region+=items_list.create(lc, Elms(lc)).columnHeight(0.048f).elmHeight(0.1f).textSize(0.04f, 0).drawMode(LDM_RECTS);
      items_list.cur_mode=LCM_MOUSE; items_list.flag|=LIST_SCALABLE;
      items_list.imageSize(0.286f, 0, Rect(-padd, -padd-0.15f, padd, padd)).columnVisible(0, false);
      items_list.sort_column[0]=1; items_list.sort_swap[0]=true;
      items_list.sort_column[1]=2; items_list.sort_swap[1]=true;
      items_list.sort_column[2]=3; items_list.sort_swap[2]=true;
      mode.tab(0)+=close_dev_page .create().func(CloseDevPage, T).hide().desc("Close Developer Page"); close_dev_page.image="Gui/close.img";
      mode.tab(0)+=t_dev_name     .create(S, &ts_dev_name).hide();
      mode.tab(0)+=t_dev_date     .create(S, &ts_stats).hide();
      mode.tab(0)+=t_dev_support  .create("Support:", &ts_stats).hide();
      mode.tab(0)+=  dev_support  .create().func(DevSupport, T).hide();
      mode.tab(0)+= t_dev_paypal  .create(S, &ts_stats).hide();
      mode.tab(0)+= t_dev_discount.create(S, &ts_stats).hide();

      mode.tab(0)+=e_dev_name    .create().maxLength( 32).desc("Set new Developer Name and press Enter to save changes");
      mode.tab(0)+=e_dev_paypal  .create().maxLength(128).desc("Set new Developer PayPal Email and press Enter to save changes");
      mode.tab(0)+=e_dev_support .create().maxLength(256).desc("Set new Developer Support Contact (Website or Email) and press Enter to save changes");
      mode.tab(0)+=e_dev_discount.create().maxLength(  2).desc("Set new Global Discount for all of your items in the store (0..99%), 0=no discount, 99=big discount");

      // item details page
      ts_name    .reset().align.set(0, -1);
      ts_desc    .reset().align.set(1, -1); ts_desc.size=0.041f;
      ts_stats   .reset().align.set(1,  0); ts_stats.size=0.037f;
      ts_dev_name.reset().align.set(1,  0); ts_dev_name.size=0.05f;
      mode.tab(1)+=back.create("< Back").func(Back, T);
      mode.tab(1)+=  item_icon.create();
      mode.tab(1)+=t_item_name.create(S, &ts_name);
      mode.tab(1)+=t_item_id  .create(S, &ts_stats);
      mode.tab(1)+=  item_www.create("WWW").func(ItemWebsite, T).desc("Open item page in a web browser");
      mode.tab(1)+=t_item_video.create("Video:", &ts_stats);
      mode.tab(1)+=  item_video.create("See on YouTube").func(ItemVideo, T);
      mode.tab(1)+=t_item_developer.create("Developer:", &ts_stats);
      mode.tab(1)+=  item_developer.create().func(DeveloperPage, T).desc("Show Developer page");
      mode.tab(1)+=t_item_date.create(S, &ts_stats);
      mode.tab(1)+=t_item_support.create("Support:", &ts_stats);
      mode.tab(1)+=  item_support.create().func(ItemSupport, T).hide();
      mode.tab(1)+=t_item_sold .create(S, &ts_stats);
      mode.tab(1)+=t_item_likes.create(S, &ts_stats);
      mode.tab(1)+=  item_like .create().func(ItemLike, T);
      mode.tab(1)+=t_item_price.create("Price: Free");
      mode.tab(1)+=t_item_cid  .create("ComputerID:", &ts_stats);
      mode.tab(1)+=  item_cid  .create().func(ItemCID, T).desc("Toggle if users are allowed to set Computer ID for their license keys of this item");
      mode.tab(1)+=  item_publish.create("PUBLISH").func(ItemPublish, T).desc("Any changes that you've made are just temporary, and they are not yet visible publicly.\nAfter you click this button, the item will be marked as ready for publishing.\nIt will be verified by Esenthel Staff within a few days.\nIf everything will be okay, it will automatically become available for sale with all the changes that you've made."); item_publish.mode=BUTTON_TOGGLE;
      mode.tab(1)+=  item_desc_region.create();
      item_desc_region+=t_item_desc.create(Vec2(0.01f, -0.00f), S, &ts_desc); t_item_desc.auto_line=AUTO_LINE_SPACE_SPLIT;
      mode.tab(1)+=item_tabs.create(item_t, Elms(item_t)).valid(true).set(0);
      FREPA(item_images)item_tabs.tab(0)+=item_images[i].create(true);
      mode.tab(1)+=buy.create().func(Buy, T);
      mode.tab(1)+=item_refresh.create("Refresh").func(ItemRefresh, T);
      item_tabs.tab(1)+=item_files_region.create();
      item_files_region+=t_new_files.create(S+"New Files: (Drag and Drop "+EsenthelProjectExt+"/7Z/RAR/ZIP/EXE files here to add them, they will replace all old files)", &ts_desc).hide();

      mode.tab(1)+=e_item_name .create().maxLength(128).desc("Set new Item Name and press Enter to save changes");
      mode.tab(1)+=e_item_video.create().maxLength(256).desc("Set new Item YouTube Video URL and press Enter to save changes");
      mode.tab(1)+=e_item_desc .create().maxLength(10*1024).desc("Set new Item Description and press Enter to save changes");
      mode.tab(1)+=e_item_price.create().maxLength(16).desc("Set new Item Price and press Enter to save changes");
      mode.tab(1)+=item_category.create(categories_node).func(ItemCategory, T).desc("Set new Item Category");

      T+=b_login.create("Log In").func(LoginToggle, T); b_login.mode=BUTTON_TOGGLE;
      T+=b_logout.create("Log Out").func(Logout, T).desc("Log out from Esenthel Store");
      T+=b_new_item.create("New Item").func(NewItemDo, T).desc("Create a new item in Esenthel Store and earn money by selling it to other people");
      T+=account.create("Account").func(Account, T).desc("Edit your account settings");
      T+=login.create();
      T+=new_item.create();
        ok_image="Gui/Misc/ok.img";
      fail_image="Gui/Misc/fail.img";
      setLogin();
      rect(Rect_C(0, 0, 2, 1.5f));
      mode.tab(1)+=rename_file.create(Vec2(clientWidth()/2, clientHeight()/-2));
      mode.tab(1)+=remove_file.create(Vec2(clientWidth()/2, clientHeight()/-2));
      mode.tab(1)+=publish_confirm.create(Vec2(clientWidth()/2, clientHeight()/-2));
      T+=_register.create(Vec2(clientWidth()/2, clientHeight()/-2));
      T+=confirm_paypal.create();
      T+=become_seller.create(Vec2(clientWidth()/2, clientHeight()/-2));
      T+=contribute.create();
   }
   Rect StoreClass::sizeLimit()C 
{
      Rect rect=::EE::Window::sizeLimit();
      MAX(rect.min.x, 2);
      MAX(rect.min.y, 1.5f);
      return rect;
   }
   void StoreClass::filter()
   {
      Memt<bool> visible; visible.setNumZero(items.elms());
      FREPA(items)
      {
         Item &item=items[i];
         Tabs *sub_cat=(InRange(categories(), sub_categories) ? &sub_categories[categories()] : null); if(sub_cat && !sub_cat->is())sub_cat=null;
         if(categories()<0 || item.category==categories())
         if(cur_dev<0 || item.author==cur_dev)
         if(!sub_cat || (*sub_cat)()<0 || item.sub_category==(*sub_cat)())
         if(!purchased() || item.purchased)
         if(!readies() || item.temp.ready)
         if(!filter_name().is() || ContainsAll(item.name, filter_name()))visible[i]=true;
      }
      items_list.setData(items, visible);
   }
   void StoreClass::setFilesRects()
   {
      flt y=0, h=0.05f;
      FREPA(item_purchases)
      {
         Purchase &p=item_purchases[i];
         p.t_license_key.rect(Vec2(0.01f, y));
         p.copy.rect(Rect_LU(0.87f, y, 0.16f, h));
         Vec2 pos=p.copy.rect().ru();
         if(p.set_computer_id   .is()){p.set_computer_id   .rect(Rect_LU(pos+Vec2(0.01f, 0), 0.55f, h)); pos=p.set_computer_id   .rect().ru();}
         if(p.set_github_account.is()){p.set_github_account.rect(Rect_LU(pos+Vec2(0.01f, 0), 0.55f, h)); pos=p.set_github_account.rect().ru();}
         y-=h;
      }
      y-=0.01f;
      h=0.045f;
      bool has_temp=false;
      FREPA(item_files)
      {
         ItemFile &f=item_files[i];
         if(f.temp && !has_temp)
         {
            has_temp=true;
            t_new_files.pos(Vec2(0.01f, y));
            y-=h;
         }
         flt w=f.download.textWidth(&h)+0.10f;
         f.download.rect(Rect_LU(0.01f, y, w, h));
         f.rename  .rect(Rect_LU(f.download.rect().ru()+Vec2(h   , 0), 0.17f, h));
         f.remove  .rect(Rect_LU(f.rename  .rect().ru()+Vec2(0.02f, 0), 0.17f, h));
         f.free    .rect(Rect_LU(f.remove  .rect().ru()+Vec2(0.02f, 0), 0.34f, h));
         y-=h+0.01f;
      }
      if(!has_temp){t_new_files.rect(Rect_LU(0.01f, y, 0, h)); y-=h;}
      item_files_region_height=y;
   }
   void StoreClass::setDescSize()
   {
      if(TextStyle *ts=t_item_desc.getTextStyle())
      {
         Vec2 size=item_desc_region.size()-t_item_desc.pos()*Vec2(1, -1);
         flt  h   =ts->textLines(t_item_desc(), size.x, t_item_desc.auto_line)*ts->lineHeight();
         if(h>size.y)
         {
            size.x-=item_desc_region.slidebarSize();
            size.y =ts->textLines(t_item_desc(), size.x, t_item_desc.auto_line)*ts->lineHeight();
         }else size.y=h;
         t_item_desc.size(size);
      }
   }
   Window& StoreClass::rect(C Rect &rect)
{
      ::EE::Window::rect(rect);
      hello_user.pos(Vec2(clientWidth()/2, -0.05f));
      {flt y=hello_user.pos().y-0.05f, b=0.3f; line.rect(Rect(b, y, clientWidth()-b, y));}
      filter_name.rect(Rect_LU(0.05f, -0.025f, 0.35f, 0.05f));
      {
         flt x=clientWidth()/2, y=-clientHeight()+0.05f;
         t_dev_discount.rect(Rect_L (Avg(x, clientWidth()), y, 0.33f, 0.05f));
         t_dev_date    .rect(Vec2   (x, y));
         t_dev_support .rect(Rect_L (0.05f, y, 0.8f, 0.05f)); y+=0.05f;
         t_dev_name    .rect(Rect_L (0.05f, y, 0.8f, 0.05f));
         t_dev_paypal  .rect(Rect_L (   x, y, 0.8f, 0.05f));
           dev_support .rect(Rect_L (t_dev_support .rect().left()+Vec2(0.13f, 0), 0.30f, 0.045f));
         e_dev_name    .rect(Rect_L (t_dev_name    .rect().left()+Vec2(0.21f, 0), 0.72f, 0.048f));
         e_dev_paypal  .rect(Rect_L (t_dev_paypal  .rect().left()+Vec2(0.10f, 0), 0.66f, 0.045f));
         e_dev_support .rect(Rect_LU(  dev_support .rect().lerp(dev_support.visible(), 1), 0.5f+dev_support.size().x*!dev_support.visible(), 0.045f));
         e_dev_discount.rect(Rect_L (t_dev_discount.rect().left()+Vec2(0.23f, 0), 0.1f , 0.045f));
         close_dev_page.rect(Rect_R (clientWidth()-0.05f, t_dev_name.rect().centerY(), 0.05f, 0.05f));
      }

      items_region.rect(Rect(0, t_dev_name.visible() ? t_dev_name.rect().centerY() : -clientHeight(), clientWidth(), hello_user.rect().min.y-0.16f).extend(-0.05f));
      Rect_LD sub_cat_rect(items_region.rect().lu()+Vec2(0, 0.01f), items_region.rect().w(), 0.05f);
      REPAO(sub_categories).rect(sub_cat_rect);
      purchased.rect(Rect_RD(sub_cat_rect.ru()+Vec2(0, 0.01f), 0.16f, 0.06f));
      readies.rect(Rect_RD(purchased.rect().ru(), 0.2f, 0.055f));
      categories.rect(Rect(sub_cat_rect.lu()+Vec2(0, 0.01f), purchased.rect().lu()-Vec2(0.06f, 0)));

      back.rect(Rect_L(0.05f, -0.05f, 0.20f, 0.055f));
      Rect r(0, -clientHeight(), clientWidth(), back.rect().min.y-0.01f); r.extend(-0.04f);
      flt cx=r.lerpX(0.25f),
          cy=r.lerpY(0.35f);
      item_icon.rect(Rect(r.min.x, cy, cx, r.max.y));
      cx+=0.02f;
      flt y=r.max.y, xx=Avg(cx, r.max.x);
      item_publish.rect(Rect_LU(cx, y, 0.2f, 0.05f)); t_item_name.rect(Rect_U(xx, y, 1.0f, 0.05f)); item_www.rect(Rect_RU(r.max.x, y, 0.12f, 0.045f)); y-=0.09f;
      t_item_developer.rect(Vec2(cx, y)); item_developer.rect(Rect_L(t_item_developer.rect().right()+Vec2(0.16f, 0), 0.5f, 0.045f));
      t_item_video.rect(Rect_L(xx, y, 0.7f, 0.05f)); item_video.rect(Rect_L(t_item_video.rect().left()+Vec2(0.13f, 0), 0.3f, 0.045f));
      t_item_id.rect(item_video.rect().right()+Vec2(0.06f, 0));
      y-=0.045f;
      t_item_date.rect(Vec2(cx, y));
      t_item_support.rect(Vec2(xx, y)); item_support.rect(Rect_L(t_item_support.pos()+Vec2(0.13f, 0), 0.3f, 0.045f));
      item_category.rect(Rect_LU(item_support.rect().ru()+Vec2(0.05f, 0), 0.25f, 0.045f));
      y-=0.045f;
      t_item_sold.rect(Vec2(cx, y));
      t_item_likes.rect(Vec2(xx, y)); item_like.rect(Rect_R(t_item_likes.rect().right()+Vec2(0.43f, 0), 0.15f, 0.045f));
      t_item_cid.rect(Vec2(item_like.rect().right()+Vec2(0.05f, 0))); item_cid.rect(Rect_L(t_item_cid.pos()+Vec2(0.181f, 0), 0.045f, 0.045f));
      y-=0.03f;
      item_desc_region.rect(Rect(cx, cy, r.max.x, y));
      setDescSize();
      cy-=0.02f;
      item_tabs.rect(Rect_LU(r.min.x, cy, 0.65f, 0.055f));
      buy.rect(Rect_RU(r.max.x, cy, 0.48f, 0.055f));
      t_item_price.rect(buy.rect());
      item_refresh.rect(Rect_RU(buy.rect().lu()-Vec2(0.02f, 0), 0.3f, 0.055f));
      contribute.move(item_desc_region.rect().rd()-contribute.rect().rd());
      y=item_tabs.rect().min.y-0.02f;
      REPAO(item_images).rect(Rect(r.lerpX(i/3.0f), r.min.y, r.lerpX((i+1)/3.0f), y).extendX(-0.005f));
      item_files_region.rect(Rect(r.min.x, r.min.y, r.max.x, y));
      setFilesRects();

      e_item_name .rect(Rect_U(t_item_name.rect().up(), 1.0f, 0.05f));
      e_item_video.rect(Rect_LU(item_video.rect().lerp(item_video.visible(), 1), 0.28f+item_video.size().x*!item_video.visible(), 0.045f));
      e_item_desc .rect(Rect_LD(item_desc_region.rect().ld(), item_desc_region.size().x-item_desc_region.slidebarSize(), 0.05f));
      e_item_price.rect(Rect_LD(buy.rect().lerp(0, buy.visible()), buy.size()));

      b_login.rect(Rect_R(items_region.rect().max.x, hello_user.pos().y, 0.17f, 0.05f));
      b_logout.rect(b_login.rect());
      b_new_item.rect(Rect_R(b_login.rect().min.x-0.01f, hello_user.pos().y, 0.22f, 0.05f));
      account.rect(Rect_R(b_new_item.rect().min.x-0.01f, hello_user.pos().y, 0.17f, 0.05f));
      login.rect(Rect_RU(b_login.rect().rd()-Vec2(0, 0.02f), 0.70f, 0.285f));
      new_item.rect(Rect_C(clientWidth()/2, clientHeight()/-2, 1.5f, 1.35f));
      return T;
   }
   void StoreClass::downloadItems() {sendCommand("get_items");}
   Window& StoreClass::show()
{
      if(!downloaded)
      {
         downloaded=true;
         downloadItems();
      }
      return ::EE::Window::show();
   }
   void StoreClass::setItemImages(Item &item)
   {
      REPA(item_images)
      {
         if(item.temp.image[i]=="keep" || !item.temp.image[i].is())
         {
            item_images[i].set(IC.getImage(item.image_path[i]));
         }else
         {
            if(item.temp.image[i]!="null")item_images[i].set(IC.getImage(ItemRes(item.id, item.temp.image[i], S+"image_"+i+"_temp.")));
         }
      }
   }
   void StoreClass::setItemDetails(Item &item)
   {
      setItemImages(item);
      item_editable=(item.has_details && (isAdmin() || (user_id>=0 && user_id==item.author))); // admin or author
      item_publish.visible(item_editable && item.has_temp);
      item_publish.set(item.temp.ready, QUIET);
      item_publish.skin=&RedSkin; // admin publishes, others request for publishing

            item_icon   .setEditable(item_editable);
      REPAO(item_images).setEditable(item_editable);
        item_icon .set(item.icon);
      t_item_name .set(item.name);
      t_item_desc .set(item.desc);
      t_item_sold .set(S+"Sold: "+(item.has_details ? S+item.sold : S)).visible(item.price>0);
      t_item_likes.set(S+"Likes: "+(item.has_details ? S+item.likes : S)).visible(item.price>0 && 0); // hide likes because not many people use this functionality
      t_item_id   .set(S+"ItemID: "+item.id).visible(item_editable);
      t_item_date .set(S+"Added: "+item.time_ago);

      t_item_cid.visible(item_editable);
        item_cid.set((item.temp.computer_id>=0) ? item.temp.computer_id!=0 : item.uses_computer_id, QUIET).visible(item_editable);

      item_support.text=(ValidEmail(item.developer_support) ? "Email" : ValidURL(item.developer_support) ? "Open Website" : ""); item_support.visible(item_support.text.is());
      item_like.visible(item.has_details && item.purchased>0 && t_item_likes.visible());
      item_like.text=(item.like ? "Unlike" : "Like");
      item_video.visible(item.video.is());
      item_developer.text=(item.developer_name.is() ? item.developer_name : item.developer_paypal);
      item_developer.visible(item.has_details);
      t_item_price.visible(item_editable);
      buy.text=item.buyText(true); buy.visible(item.price!=0 && item.has_details);
      item_refresh.visible((buy.visible() && loggedIn()) || item_editable);
      Memt<Str> license_keys; Split(license_keys, item.license_keys, '\n'); if(license_keys.elms() && !license_keys.last().is())license_keys.removeLast();
      item_purchases.setNum(license_keys.elms()); FREPAO(item_purchases).create(item_files_region, license_keys[i], item.uses_computer_id, item.uses_github_account);
      if(!license_keys.elms() && item.has_details && item.price>0)item_files_region+=item_purchases.New().t_license_key.create("License Key: You need to buy this item in order to get a license key", &ts_desc);
      item_files.setNum(item.files.elms()); FREPAO(item_files).create(item_files_region, item.files[i]);
      FREPA(item.temp.files)item_files.New().create(item_files_region, item.temp.files[i], true);
      t_new_files.visible(item_editable);

      e_item_name .set(item.temp.name.is() ? item.temp.name : item.name);
      e_item_price.set(TextReal(((item.temp.price>=0) ? item.temp.price : item.price)/100.0f, 2)); e_item_price.pos(buy.pos()+Vec2(0, buy.visible()*e_item_price.size().y));
      e_item_desc .set(item.has_temp ? item.temp.desc : item.desc);
      e_item_video.set((item.has_temp && item.temp.video!="keep") ? item.temp.video : item.video); Rect rect=e_item_video.rect(); rect.min.x=item_video.rect().lerpX(item_video.visible()); e_item_video.rect(rect);
      item_category.visible(item_editable);
      setCategory((item.temp.cat>=0) ? item.temp.cat : item.category, (item.temp.sub_cat>=0) ? item.temp.sub_cat : item.sub_category);

      setFilesRects();
      setDescSize();
   }
   void StoreClass::refreshDev()
   {
      dev_editable=(isAdmin() || (cur_dev>=0 && user_id==cur_dev)); // admin or author
      t_dev_name    .set(S+"Developer: "      +cur_dev_name        ).visible(cur_dev>=0);
      t_dev_paypal  .set(S+"PayPal: "         +cur_dev_paypal      ).visible(cur_dev>=0 && dev_editable);
      t_dev_discount.set(S+"Global Discount: "+cur_dev_discount+'%').visible(cur_dev>=0 && dev_editable);
        dev_support.text=(ValidEmail(cur_dev_support) ? "Email" : ValidURL(cur_dev_support) ? "Open Website" : ""); dev_support.visible(dev_support.text.is());
      rect(::EE::Window::rect());
   }
   void StoreClass::openDeveloper(int id, C Str &name, C Str &support, C Str &paypal, C DateTime &date, int discount)
   {
      cur_dev=id;
      cur_dev_name    =name;
      cur_dev_support =support;
      cur_dev_paypal  =paypal;
      cur_dev_discount=discount;
      D.setFade(0.05f, true);
      t_dev_date.set(S+"Joined: "+TimeAgo(date)).visible(id>=0);
      close_dev_page.visible(id>=0);
      t_dev_support .visible(id>=0);
      e_dev_name    .set(name);
      e_dev_support .set(support);
      e_dev_paypal  .set(paypal);
      e_dev_discount.set(S+discount);
      refreshDev();
      mode.set(0);
      filter();
   }
   void StoreClass::openItem(int id)
   {
      D.setFade(0.05f, true);
      rename_file.hide();
      remove_file.hide();
      publish_confirm.hide();
      contribute.hide();
      if(Item *item=findItem(id))
      {
         cur_item=id;
         mode.set(1);
         item->getDetails();
         setItemDetails(*item);
      }else
      {
         cur_item=-1;
         mode.set(0);
      }
   }
   void StoreClass::itemGotDetails(Item &item)
   {
      if(item.id==cur_item)setItemDetails(item);
   }
   bool StoreClass::itemCanUploadFile(Item &item, C Str &file)
   {
      Str ext=GetExt(file);
      if( ext==EsenthelProjectExt || ext=="7z" || ext=="rar" || ext=="zip" || ext=="exe")
      {
         if(ValidFileNameForUpload(file))
         {
            REPA(item.temp.files       )if(EqualPath(item.temp.files       [i].     file, file)                                              ){Gui.msgBox(S, S+"Item already has a file \""+file+"\""               ); return false;}
            REPA(AppStore.store_uploads)if(EqualPath(AppStore.store_uploads[i].item_file, file) && AppStore.store_uploads[i].item_id==item.id){Gui.msgBox(S, S+"Item already has a file \""+file+"\" being uploaded"); return false;}
            
            int item_files=item.temp.files.elms(); REPA(AppStore.store_uploads)if(AppStore.store_uploads[i].item_id==item.id)item_files++;
            if( item_files<EsenthelStoreMaxFileNum)
            {
               return true;
            }else Gui.msgBox(S, S+"Can't upload more than "+EsenthelStoreMaxFileNum+" files per item");
         }else Gui.msgBox(S, S+"Invalid file name \""+file+"\"");
      }else Gui.msgBox(S, S+"Only "+EsenthelProjectExt+"/7Z/RAR/ZIP/EXE extensions are supported");
      return false;
   }
   void StoreClass::drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos)
   {
      if(item_editable)
      {
         if(item_files_region.contains(obj))
         {
            if(Item *item=findItem(cur_item))FREPA(names)
            {
               Str name=names[i], file_file=GetBase(name);
               if(itemCanUploadFile(*item, file_file))
               {
                  names.remove(i--, true);
                  File src; if(src.readStdTry(name))
                  {
                     if(src.size()<=EsenthelStoreMaxFileSize)
                     {
                        Str file_name=Replace(GetExtNot(file_file), '_', ' ');
                        Upload &upload=store_uploads.New();
                        upload.create(item_files_region, cur_item, file_name, file_name);
                        upload.item_file=file_file;
                        upload.chunked=(src.size()>EsenthelStoreFileChunked);
                        item_files_region+=upload.cancel.create("Cancel").func(Upload::Cancel, upload);
                        Swap(src, upload.src);
                        setUserParams(upload.params, "add_item_file");
                                      upload.params.New().set("i"             , S+cur_item);
                                      upload.params.New().set("item_file_file", file_file);
                                      upload.params.New().set("item_file_name", file_name);
                        upload.transfer();
                     }else Gui.msgBox(S, S+"File size "+FileSize(src.size())+" exceeds "+FileSize(EsenthelStoreMaxFileSize)+" limit");
                  }else Gui.msgBox(S, S+"Can't read file \""+name+"\"");
               }
            }
         }else
         if(obj==&item_icon)
         {
            FREPA(names)
            {
               Str name=names[i], ext=GetExt(name);
               if(ExtType(ext)==EXT_IMAGE)
               {
                  Image temp; if(temp.ImportTry(name))
                  {
                     File src;
                     if(ext=="jpeg")ext="jpg";
                     if(ext=="jpg" || ext=="png")src.readStdTry(name);else
                     {
                        src.writeMem(); if(HasAlpha(temp)){temp.ExportPNG(src); ext="png";}else{temp.ExportJPG(src, 0.8f); ext="jpg";}
                        src.pos(0);
                     }
                     if(src.size()<=EsenthelStoreMaxIconSize)
                     {
                        Upload &upload=store_uploads.New();
                        upload.create(mode.tab(1), cur_item, "Icon");
                        upload.dest=&item_icon;
                        upload.set =ext;
                        Swap(src, upload.src);
                        Memt<HTTPParam> params; setUserParams(params, "set_item_icon"); params.New().set("i", S+cur_item); params.New().set("item_icon", ext);
                        upload.down.create(EsenthelStoreURL, params, &upload.src);
                     }else Gui.msgBox(S, S+"File size "+FileSize(src.size())+" exceeds "+FileSize(EsenthelStoreMaxIconSize)+" limit");
                  }else Gui.msgBox(S, S+"Couldn't import image \""+name+"\"");
                  break;
               }
            }
         }else
         REPA(item_images)if(obj==&item_images[i])
         {
            int image_index=i;
            FREPA(names)
            {
               Str name=names[i], ext=GetExt(name);
               if(ExtType(ext)==EXT_IMAGE)
               {
                  Image temp; if(temp.ImportTry(name, IMAGE_R8G8B8)) // ignore alpha channel
                  {
                     File src;
                     if(ext=="jpeg")ext="jpg";
                     if(ext=="jpg")src.readStdTry(name);else
                     {
                                  src.writeMem(); temp.ExportJPG(src, 0.8f); ext="jpg";
                        File png; png.writeMem(); temp.ExportPNG(png); if(png.size()<src.size()){Swap(png, src); ext="png";}
                        src.pos(0);
                     }
                     if(src.size()<=EsenthelStoreMaxImageSize)
                     {
                        Upload &upload=store_uploads.New();
                        upload.create(item_tabs.tab(0), cur_item, "Image");
                        upload.index=image_index;
                        upload.dest =&item_images[image_index];
                        upload.set  =ext;
                        Swap(src, upload.src);
                        Memt<HTTPParam> params; setUserParams(params, "set_item_image"); params.New().set("i", S+cur_item); params.New().set("item_image_index", S+image_index); params.New().set("item_image_status", ext);
                        upload.down.create(EsenthelStoreURL, params, &upload.src);
                     }else Gui.msgBox(S, S+"File size "+FileSize(src.size())+" exceeds "+FileSize(EsenthelStoreMaxImageSize)+" limit");
                  }else Gui.msgBox(S, S+"Couldn't import image \""+name+"\"");
                  break;
               }
            }
            break;
         }
      }
   }
   void StoreClass::update(C GuiPC &gpc)
{
      ::EE::ClosableWindow::update(gpc);
      if(visible() && gpc.visible)
      {
         if((Ms.tapped(1) && Gui.window()==this) || (Ms.bp(4) && Gui.window()==this && !(Gui.kb() && Gui.kb()->type()==GO_TEXTLINE)))back.push();
         REPA(MT)if(MT.tapped(i) && MT.guiObj(i)==&items_list)if(Item *item=items_list())openItem(item->id);

         bool preview=false;
         REPAD(mt, MT)REPA(item_images)if(MT.guiObj(mt)==&item_images[i] && item_images[i].image && item_images[i].image->is())
         {
            image_preview=item_images[i].image;
            preview=true;
            goto found;
         }
      found:
         AdjustValTime(image_preview_step, preview, 0.0001f);

         e_dev_name    .visible(dev_editable && (e_dev_name    .contains(Gui.kb()) || e_dev_name    .contains(Gui.ms()) || t_dev_name.contains(Gui.ms())));
         e_dev_paypal  .visible(dev_editable && (e_dev_paypal  .contains(Gui.kb()) || e_dev_paypal  .contains(Gui.ms()) || t_dev_paypal.contains(Gui.ms())));
         e_dev_support .visible(dev_editable && (e_dev_support .contains(Gui.kb()) || e_dev_support .contains(Gui.ms()) || dev_support.contains(Gui.ms()) || t_dev_support.contains(Gui.ms())));
         e_dev_discount.visible(dev_editable && (e_dev_discount.contains(Gui.kb()) || e_dev_discount.contains(Gui.ms()) || t_dev_discount.contains(Gui.ms())));

         e_item_name .visible(item_editable && (e_item_name .contains(Gui.kb()) || e_item_name .contains(Gui.ms()) || t_item_name .contains(Gui.ms())));
         e_item_video.visible(item_editable && (e_item_video.contains(Gui.kb()) || e_item_video.contains(Gui.ms()) || t_item_video.contains(Gui.ms()) || item_video.contains(Gui.ms())));
         e_item_desc .visible(item_editable && (e_item_desc .contains(Gui.kb()) || e_item_desc .contains(Gui.ms()) || item_desc_region.contains(Gui.ms())));
         e_item_price.visible(item_editable && (e_item_price.contains(Gui.kb()) || e_item_price.contains(Gui.ms()) || buy.contains(Gui.ms()) || t_item_price.contains(Gui.ms())));

         if((Kb.k(KB_ENTER) || Kb.k(KB_NPENTER)) && Kb.k.first() && Gui.kb() && Gui.kb()->type()==GO_TEXTLINE)
         {
            if(Gui.kb()==&e_dev_name)
            {
               if(e_dev_name().is())
               {
                  if(cur_dev==user_id)user_name=e_dev_name(); cur_dev_name=e_dev_name(); refreshDev(); sendCommand("set_user_name", "user_id", S+cur_dev, "user_name", e_dev_name());
               }else Gui.msgBox(S, "Invalid User Name");
            }else
            if(Gui.kb()==&e_dev_paypal)
            {
               if(ValidEmail(e_dev_paypal()))
               {
                  confirm_paypal.activate();
               }else Gui.msgBox(S, "Invalid User PayPal email");
            }else
            if(Gui.kb()==&e_dev_support)
            {
               if(ValidSupport(e_dev_support()))
               {
                  if(cur_dev==user_id)user_support=e_dev_support(); cur_dev_support=e_dev_support(); refreshDev(); sendCommand("set_user_support", "user_id", S+cur_dev, "user_support", Replace(e_dev_support(), '/', '\\')); // server will fail for some reason if parameter contains "http://" (instead use '\')
               }else Gui.msgBox(S, "Invalid User Support Contact.\nIt must be either a website link or an email address.");
            }else
            if(Gui.kb()==&e_dev_discount)
            {
               int discount=TextInt(e_dev_discount());
               if( discount>=0 && discount<100)
               {
                  if(cur_dev==user_id)user_discount=discount; cur_dev_discount=discount; refreshDev(); sendCommand("set_user_discount", "user_id", S+cur_dev, "user_discount", S+discount);
               }else Gui.msgBox(S, "Invalid discount.\nIt must be in range 0 .. 99 %");
            }else
            if(Gui.kb()==&e_item_name)
            {
               if(e_item_name().is())
               {
                  if(Item *item=findItem(cur_item))item->setTemp().name=e_item_name();
                  sendCommand("set_item_name", "i", S+cur_item, "item_name", e_item_name());
               }else Gui.msgBox(S, "Item name must not be empty");
            }else
            if(Gui.kb()==&e_item_price)
            {
               dbl priced=TextDbl(e_item_price());
               int price =Round(priced*100);
               if( price>=0)
               {
                  if(Item *item=findItem(cur_item))item->setTemp().price=price;
                  sendCommand("set_item_price", "i", S+cur_item, "item_price", S+price);
               }else Gui.msgBox(S, "Invalid item price");
            }else
            if(Gui.kb()==&e_item_desc)
            {
               if(Item *item=findItem(cur_item))item->setTemp().desc=e_item_desc();
               sendCommandPost("set_item_desc", "i", S+cur_item, "item_desc", e_item_desc());
            }else
            if(Gui.kb()==&e_item_video)
            {
               Str video=YouTubeFullToEmbed(e_item_video());
               if(ValidVideo(video))
               {
                  if(Item *item=findItem(cur_item))item->setTemp().video=video;
                  sendCommand("set_item_video", "i", S+cur_item, "item_video", Replace(video, '/', '\\')); // server will fail for some reason if parameter contains "http://" (instead use '\')
               }else Gui.msgBox(S, "Invalid item video link.\nIt must start with \"http://www.youtube.com/embed/\"");
            }
         }

         // check for commands from the server
         REPA(store_commands) // process from end because they will get removed
         {
            Download &down=store_commands[i];
            if(Check(down))
            {
               TextData data; if(data.load(FileText().readMem(down.data(), down.size(), UTF_8_NAKED)))if(C TextNode *cmd=data.findNode("cmd"))
               {
                C TextNode *ok=data.findNode("OK");
                  Str msg; for(int i=0; C TextNode *n=data.findNode("msg", i); i++)msg.line()+=n->asText();
                  if(cmd->asText()=="log_in" || cmd->asText()=="register" || cmd->asText()=="forgot_pass")
                  {
                     user_id=-1;
                   C TextNode *result=data.findNode("result");
                     if(ok && ok->asBool())
                     {
                        if(C TextNode *user_id      =data.findNode("u"       ))T.user_id      =user_id      ->asInt ();
                        if(C TextNode *user_key     =data.findNode("k"       ))T.user_key     =user_key     ->asInt ();
                        if(C TextNode *user_name    =data.findNode("n"       ))T.user_name    =user_name    ->asText();
                        if(C TextNode *user_support =data.findNode("s"       ))T.user_support =user_support ->asText();
                        if(C TextNode *user_paypal  =data.findNode("p"       ))T.user_paypal  =user_paypal  ->asText();
                        if(C TextNode *user_date    =data.findNode("d"       ))T.user_date    .fromText(user_date->asText());
                        if(C TextNode *user_seller  =data.findNode("seller"  ))T.user_seller  =user_seller  ->asBool();
                        if(C TextNode *user_discount=data.findNode("discount"))T.user_discount=user_discount->asInt ();
                                                                               T.user_email   =login_email;
                                                                               T.user_pass    =login_pass ;
                        downloadItems(); // redownload items after logging in
                     }
                     if(user_id==-1 && msg=="User not found"){_register.activate(); msg.clear();}
                     setLogin();
                     refreshDev();
                  }else
                  if(cmd->asText()=="get_items")
                  {
                     if(ok && ok->asBool())
                     {
                        items.clear();
                        if(C TextNode *node=data.findNode("Items"    ))FREPA(node->nodes)items.New()=node->nodes[i];
                        if(C TextNode *node=data.findNode("Purchased"))FREPA(node->nodes)if(Item *item=findItem(node->nodes[i].asInt()))item->purchased++;
                        if(C TextNode *node=data.findNode("Ready"    ))FREPA(node->nodes)if(Item *item=findItem(node->nodes[i].asInt()))item->temp.ready=true;
                        if(Item *item=findItem(cur_item))item->getDetails();
                        filter();
                     }
                  }else 
                  if(cmd->asText()=="new_item")
                  {
                     if(ok && ok->asBool())
                     {
                        new_item.name.clear();
                        int item_id=-1; if(C TextNode *n=data.findNode("ItemID"))item_id=n->asInt();
                        if( item_id>=0)
                        {
                           Item &item=items.New();
                           item.id=item_id;
                           item.author=user_id;
                           if(C TextNode *n=data.findNode("ItemName"))item.name=n->asText();
                           filter();
                        }
                        openItem(item_id);
                     }
                  }else
                  if(cmd->asText()=="like_item")
                  {
                     if(ok && ok->asBool())
                     {
                        if(C TextNode *id=data.findNode("id"))if(Item *item=findItem(id->asInt()))
                        {
                           if(C TextNode *n=data.findNode("like" ))item->like =n->asBool();
                           if(C TextNode *n=data.findNode("likes"))item->likes=n->asInt ();
                           if(item->id==cur_item)setItemDetails(*item);
                        }
                     }
                  }else
                  if(cmd->asText()=="get_item_details")
                  {
                     if(C TextNode *id=data.findNode("id"))if(Item *item=findItem(id->asInt()))item->setDetails(data);
                  }else
                  if(cmd->asText()=="set_item_ready")
                  {
                     if(isAdmin()) // admin has published an item
                     {
                        if(Item *item=findItem(cur_item))item->getDetails(true);
                     }
                  }else
                  if(cmd->asText()=="remove_item_file")
                  {
                     if(ok && ok->asBool())if(Item *item=findItem(cur_item))item->getDetails(true);
                  }else
                  if(cmd->asText()=="rename_item_file")
                  {
                     if(ok && ok->asBool())if(Item *item=findItem(cur_item))item->getDetails(true);
                  }else
                  if(cmd->asText()=="set_item_file_access")
                  {
                     if(ok && ok->asBool())if(Item *item=findItem(cur_item))item->getDetails(true);
                  }else
                  if(cmd->asText()=="set_user_discount")
                  {
                     if(ok && ok->asBool())downloadItems(); // redownload items to get the new prices
                  }
                  if(msg.is())Gui.msgBox("Esenthel Store", msg);
               }
               down.del();
            }
            if(!down.state())store_commands.removeValid(i);
         }
         flt  y=item_files_region_height;
         bool have_file_uploads=false;
         REPA(store_uploads) // process from end because they will get removed
         {
              Upload &upload=store_uploads[i];
            Download &down=upload.down;
            upload.update();
            if(upload.dest){Rect r=upload.dest->rect(); upload.rect(Rect_C(r.center(), r.w(), 0.04f));}else
                           {if(upload.visible()){upload.pos(Vec2(0.01f, y)); y-=0.045f; have_file_uploads=true;}} // file upload
            if(down.state()==DWNL_ERROR && down.sent() && down.sent()==down.toSend()) // finished uploading, but didn't receive confirmation
            {
               if(Item *item=findItem(upload.item_id))item->getDetails(true);
               Gui.msgBox("Esenthel Store", S+"\""+upload.name+"\" has finished uploading, however confirmation was not received.\nPlease verify if the upload succeeded.");
               down.del();
            }else
            if(Check(down) && upload.checkChunked())
            {
               TextData data; if(data.load(FileText().readMem(down.data(), down.size())))if(C TextNode *cmd=data.findNode("cmd"))
               {
                C TextNode *ok=data.findNode("OK");
                  Str msg; for(int i=0; C TextNode *n=data.findNode("msg", i); i++)msg.line()+=n->asText();
                  if(cmd->asText()=="set_item_icon")
                  {
                     if(ok && ok->asBool())
                     {
                        if(Item *item=findItem(upload.item_id)){item->setTemp().icon=upload.set; item->setIcon(true);}
                        IC.changed(ItemRes(upload.item_id, upload.set, "icon_temp."));
                     }
                  }else
                  if(cmd->asText()=="set_item_image")
                  {
                     if(ok && ok->asBool())
                     {
                        if(Item *item=findItem(upload.item_id)){item->setTemp().image[upload.index]=upload.set; if(upload.item_id==cur_item)setItemImages(*item);}
                        IC.changed(ItemRes(upload.item_id, upload.set, S+"image_"+upload.index+"_temp."));
                     }
                  }else
                  if(cmd->asText()=="add_item_file")
                  {
                     if(ok && ok->asBool())
                     {
                        if(Item *item=findItem(upload.item_id)){item->setTemp(); item->getDetails(true);}
                     }
                  }
                  if(msg.is())Gui.msgBox("Esenthel Store", msg);
               }
               down.del();
            }
            if(!down.state())store_uploads.removeValid(i, true);
         }
      }
   }
   void StoreClass::draw(C GuiPC &gpc)
{
      ::EE::Window::draw(gpc);
      if(visible() && gpc.visible)
      {
         if(image_preview && image_preview_step>EPS_COL)
         {
            D.clip();
            D.fxBegin();
            ALPHA_MODE alpha=D.alpha(ALPHA_NONE); Rect(-D.w(), -D.h(), D.w(), D.h()).draw(ColorAlpha(BLACK, 0.7f)); D.alpha(alpha);
            Rect rect=image_preview->fit(Gui.desktop()->rect())*0.9f;
            D.drawShadow(128, rect, 0.07f);
            image_preview->drawFilter(rect);
            D.fxEnd()->drawFs(ColorAlpha(image_preview_step));
         }else if(image_preview_step<=EPS)image_preview=null;
      }
   }
StoreClass::StoreClass() : downloaded(false), dev_editable(false), item_editable(false), user_seller(false), user_id(-1), user_key(0), user_discount(0), cur_item(-1), cur_dev(-1), cur_dev_discount(0), item_files_region_height(0), image_preview_step(0) {}

StoreClass::Item::ItemFile::ItemFile() : free(false) {}

StoreClass::Item::Temp::Temp() : ready(false), cat(-1), sub_cat(-1), price(-1), computer_id(-1), subscription(-1) {}

StoreClass::DownloadFile::DownloadFile() : saved(false), temp(false), item_id(-1), file_size(-1) {}

StoreClass::ItemFile::ItemFile() : temp(false), is_free(false) {}

StoreClass::Upload::Upload() : chunked(false), item_id(-1), index(0), sent(0), dest(null) {}

StoreClass::ItemImage::ItemImage() : editable(false), hoverable(false) {}

/******************************************************************************/
