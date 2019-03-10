/******************************************************************************/
/******************************************************************************/
class StoreClass : ClosableWindow
{
   static Str ItemPath(int item_id);                        
   static Str ItemRes (int item_id, C Str&file, C Str&name);

   class Item
   {
      class ItemFile
      {
         bool free;
         Str  name, file;

public:
   ItemFile();
      };
      class Temp
      {
         bool ready;
         int  cat, sub_cat, price, computer_id,
              subscription; // number of days, 30=month, 365=year
         Str  name, video, icon, image[3], desc_short, desc;
         Memc<ItemFile> files;

         void reset();

         Temp(C TextNode &node);

public:
   Temp();
      };

      static void SetFiles(MemPtr<ItemFile>files, C TextNode&node);
      static bool XmlChar(char c);                               
      static Str DecodeText(C Str &str);

      bool           requested_details, has_details, like, uses_computer_id, uses_github_account, has_temp;
      int            id, price, category, sub_category, purchased, popularity, author, likes, sold, developer_discount, subscription;
      Str            name, desc_short, desc, video, price_text, icon_path, time_ago, image_path[3], developer_name, developer_paypal, developer_support, license_keys;
      DateTime       date_added, developer_date_joined;
      ImagePtr       icon;
      Memc<ItemFile> files;
      Temp           temp;

      Item();

      Str buyText(bool include_price)C;
      bool isNew(); // if haven't been published yet
      void getDetails(bool force=false);
      Temp& setTemp();
      void setIcon(bool force=false);
      void setPrice(int price);

      Item(C TextNode &node);
      void setDetails(C TextData &data);
   };
   class NewItem : ClosableWindow
   {
      Text    t_name, terms;
      TextLine  name;
      Button    b_create;
      TextBlack ts;

      static void Create(NewItem &new_item);

      ClosableWindow& create();
      virtual ClosableWindow& rect(C Rect &rect)override;
      virtual ClosableWindow& show()override;
      virtual void update(C GuiPC &gpc)override;
   };
   class Login : ClosableWindow
   {
      Text t_email, t_pass;
      TextLine email, pass;
      Button   login, forgot_pass, terms;

      static void LoginDo   (Login &login);
      static void ForgotPass(Login &login);
      static void Cancel    (Login &login);
      static void ShowTerms (Login &login);

      ClosableWindow& create();
      virtual ClosableWindow& rect(C Rect &rect)override;
      virtual ClosableWindow& show()override;
      virtual void update(C GuiPC &gpc)override;
   };
   class Register : ClosableWindow
   {
      Text   text;
      Button yes, cancel;

      static void Yes   (Register &reg);
      static void Cancel(Register &reg);

      Register& create(C Vec2 &pos);
      Register& activate();
   };
   class ConfirmPayPal : ClosableWindow
   {
      Text      text;
      TextLine  paypal;
      TextBlack ts;
      Button    confirm;

      static void Confirm(ConfirmPayPal &cp);

      ConfirmPayPal& create();
      void setPos();
      ConfirmPayPal& activate()override;
      virtual void update(C GuiPC &gpc)override;
   };
   class BecomeSeller : ClosableWindow
   {
      TextNoTest text;
      TextBlack  ts;
      Button     engine_license;

      static void EngineLicense(BecomeSeller &bs);

      BecomeSeller& create(C Vec2 &pos);
   };
   class Contribute : ClosableWindow
   {
      Text      text;
      TextBlack ts;
      TextLine  amount;
      Button    contribute;

      static void Buy(Contribute &contr);

      ClosableWindow& create();
      virtual void update(C GuiPC &gpc)override;
   };
   class ItemList : List<Item>
   {
      virtual void draw(C GuiPC &gpc)override;
   };
   const_mem_addr class Purchase
   {
      Str      license_key;
      Text   t_license_key;
      Button copy, set_computer_id, set_github_account;

      static void Copy(Purchase &p);

      void create(GuiObj &parent, C Str &license_key, bool computer_id, bool github_account);
   };
   const_mem_addr class DownloadFile : Window
   {
      class WindowIOEx : WindowIO
      {
         static void Hidden(WindowIOEx &w);

         virtual WindowIO& hide()override;
      };
      class ProgressNoTest : Progress
      {
         virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;
      };

      bool       saved, temp;
      int        item_id;
      Str        file, dest;
      WindowIOEx win_io;
      Download   down;
      long       file_size;
      DateTime   file_modify_time;
      File       file_done;

      ProgressNoTest progress;
      Button         open, import, retry, cancel;
      Text           text;
      TextBlack      ts;

      static void Open  (DownloadFile &df); // explore file  and delete window
      static void Import(DownloadFile &df); // open CopyElms and delete window
      static void Cancel(DownloadFile &df); // delete

      static void Retry(DownloadFile &df);
             void download(bool resume);

      static void Select(C Str &name, DownloadFile &df);
             void select(C Str &name);
      bool valid()C;                                   
      void create(int item_id, C Str &file, bool temp);
      virtual void update(C GuiPC &gpc)override;

public:
   DownloadFile();
   };
   const_mem_addr class ItemFile
   {
      Str    name, file;
      bool   temp, is_free;
      Button download, rename, remove, free;

      static bool ItemDownloadable();

      static void Download(ItemFile &df);
      static void Rename (ItemFile &df); 
      static void Remove (ItemFile &df); 
      static void SetFree(ItemFile &df); 

      void create(GuiObj &parent, C Item::ItemFile &file, bool temp=false);

public:
   ItemFile();
   };
   class RenameFile : ClosableWindow
   {
      TextLine new_name;
      Str      file, name;

      static void OK(RenameFile &rf);
             void ok();
      RenameFile& create(C Vec2 &pos);
      RenameFile& activate(ItemFile &df);
      virtual void update(C GuiPC &gpc)override;
   };
   class RemoveFile : ClosableWindow
   {
      Text   t_name;
      Button     ok;
      Str      file;

      static void OK(RemoveFile &fr);

      void activate(ItemFile &df);
      RemoveFile& create(C Vec2 &pos);
   };
   class PublishConfirm : ClosableWindow
   {
      Text text;
      Button ok;

      static void OK(PublishConfirm &pc);

      void publish();
      PublishConfirm& create(C Vec2 &pos);
   };
   const_mem_addr class Upload
   {
      bool      chunked;
      Download  down;
      File      src;
      Progress  progress;
      TextBlack ts;
      Text      text;
      Button    cancel;
      int       item_id, index;
      long      sent;
      GuiObj   *dest;
      Str       set, display, item_file, name;
      Memc<HTTPParam> params;

      static void Cancel(Upload &up);
      
      void create(GuiObj &parent, int item_id, C Str &name, C Str &display=S);
      void rect(C Rect &rect);
      void pos(C Vec2 &pos);
      bool checkChunked();
      void update();
      void transfer();
      bool visible()C;      
      void visible(bool on);

public:
   Upload();
   };
   class ItemImage : GuiImage
   {
      bool editable, hoverable;

      void setEditable(bool editable);

      ItemImage& create(bool hoverable=false);

      virtual GuiObj* test(C GuiPC &gpc, C Vec2 &pos, GuiObj* &mouse_wheel)override;

      virtual void draw(C GuiPC &gpc)override;

public:
   ItemImage();
   };
   class Line : GuiCustom
   {
      virtual void draw(C GuiPC &gpc)override;
   };

   static void ItemWebsite(StoreClass &store);
   static void ItemVideo(StoreClass &store);
   static void ItemCID(StoreClass &store);
   static void ItemLike(StoreClass &store);
   static void ItemRefresh(StoreClass &store);
   static void ItemPublish(StoreClass &store);
   static void ItemCategory(StoreClass &store);
   static void Buy(StoreClass &store);
   static void DeveloperPage(StoreClass &store);
   static void Back(StoreClass &store);       
   static void Filter(StoreClass &store);     
   static void Readies(StoreClass &store);    
   static bool Check(Download &down);
   static Str MD5Text(C Str8 &text);
   static void SetUserParams(MemPtr<HTTPParam> params, C Str&user, C Str&password, int key, C Str&cmd=S);
   static void OpenSupport(C Str &support);                                                            
   static void ItemSupport(StoreClass &store);                                                         
   static void DevSupport(StoreClass &store);                                   
   static void CloseDevPage(StoreClass &store);                         
   static void Account(StoreClass &store);   
   static void NewItemDo(StoreClass &store);
   static void LoginToggle(StoreClass &store);
   static void Logout(StoreClass &store);   

   static cchar8 *categories_t[]
;
   static cchar8 *source_code_cats[]
;
   static cchar8 *game_cats[]
;
   static cchar8 *asset_cats[]
;
   static cchar8 *item_t[]
;

   Button         purchased, readies, back;
   Tabs           categories, sub_categories[5],
                  mode, // item_list, item_details (this is invisible)
                  item_tabs; // images, files
   Memc<Item>     items;
   ItemList       items_list;
   Region         items_region, item_files_region, item_desc_region;
   Memx<Purchase> item_purchases;
   Memx<ItemFile> item_files;
   Memx<DownloadFile> download_files;
   bool           downloaded, dev_editable, item_editable;
   Memx<Download> store_commands;
   Memx<Upload>   store_uploads;
   bool           user_seller;
   int            user_id, user_key, user_discount, cur_item, cur_dev, cur_dev_discount;
   flt            item_files_region_height;
   Str            user_name, cur_dev_name, user_email, login_email, user_pass, login_pass, user_support, cur_dev_support, user_paypal, cur_dev_paypal;
   DateTime       user_date;
   Text           hello_user, t_item_name, t_item_developer, t_item_sold, t_item_likes, t_item_video, t_item_desc, t_item_id, t_item_date, t_item_support, t_item_price, t_item_cid, t_dev_name, t_dev_paypal, t_dev_support, t_dev_date, t_dev_discount, t_new_files;
   TextLine       filter_name, e_dev_name, e_dev_paypal, e_dev_support, e_dev_discount, e_item_name, e_item_video, e_item_desc, e_item_price;
   CheckBox       item_cid;
   ComboBox       item_category;
   TextBlack      ts_name, ts_dev_name, ts_desc, ts_stats;
   ItemImage      item_icon, item_images[3];
   Button         b_new_item, b_login, b_logout, account, item_www, item_developer, item_like, item_video, buy, item_refresh, item_support, item_publish, dev_support, close_dev_page;
   Line           line;
   ImagePtr       ok_image, fail_image;
   Login          login;
   Register      _register;
   BecomeSeller   become_seller;
   ConfirmPayPal  confirm_paypal;
   Contribute     contribute;
   RenameFile     rename_file;
   RemoveFile     remove_file;
   PublishConfirm publish_confirm;
   NewItem        new_item;
   MemberDesc     price_sort, date_sort;
   ImagePtr       image_preview;
   flt            image_preview_step;
   Node<MenuElm>  categories_node;

   Item* findItem(int id);             
   bool loggedIn()C;                   
   bool isAdmin()C;                    
   bool isPurchased(int id);                           
   bool isSeller()C;                                   
   void clearDownloadFiles();                                                                                
   void setUserParams(MemPtr<HTTPParam> params, C Str &cmd);                                                          
   void sendCommand(C Str &cmd, C Str &name=S, C Str &value=S, C Str&name2=S, C Str &value2=S, C Str&name3=S, C Str&value3=S);
   void sendCommandPost(C Str &cmd, C Str &name, C Str &value, C Str&post_name, C Str &post_value);
   void setLogin();
   void loginDo(int mode, C Str &email, C Str &pass);
   void logout();
   void setCategory(int cat, int sub);
   void getCategory(int &cat, int &sub);
   void create();
   virtual Rect sizeLimit()C override;
   void filter();
   void setFilesRects();
   void setDescSize();
   virtual Window& rect(C Rect &rect)override;
   void downloadItems();            
   virtual Window& show()override;
   void setItemImages(Item &item);
   void setItemDetails(Item &item);
   void refreshDev();
   void openDeveloper(int id, C Str &name=S, C Str &support=S, C Str &paypal=S, C DateTime &date=DateTime().zero(), int discount=0);
   void openItem(int id);
   void itemGotDetails(Item &item);
   bool itemCanUploadFile(Item &item, C Str &file);
   void drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos);
   virtual void update(C GuiPC &gpc)override;
   virtual void draw(C GuiPC &gpc)override;

public:
   StoreClass();
};
/******************************************************************************/
/******************************************************************************/
extern StoreClass AppStore;
/******************************************************************************/
