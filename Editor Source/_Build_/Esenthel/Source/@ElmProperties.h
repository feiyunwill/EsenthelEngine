/******************************************************************************/
/******************************************************************************/
class ElmProperties : ClosableWindow
{
   TextBlack ts, ts_small;
   Text    t_id, t_file, t_size, t_class, t_src;
   Text      id,   file,   size,   Class,   src;
   Button  b_id, b_file,                  b_src;
   UID     elm_id;

   static void CopyID  (ElmProperties &ep);
   static void CopyFile(ElmProperties &ep);
   static void Explore (ElmProperties &ep);

   void create();
   void activate(Elm &elm);

public:
   ElmProperties();
};
/******************************************************************************/
/******************************************************************************/
extern ElmProperties ElmProps;
/******************************************************************************/
