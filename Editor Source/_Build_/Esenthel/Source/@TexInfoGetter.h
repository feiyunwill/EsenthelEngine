/******************************************************************************/
/******************************************************************************/
class TexInfoGetter
{
   int       got_new_data;
   Str       tex_path;
   Memc<UID> tex_to_process;
   Memb<UID> tex_to_process1; // need to use 'Memb' to have const_mem_addr, because these can be added on the fly
   uintptr   thread_id;

  ~TexInfoGetter(); // stop processing before deleting other memebers

   void stop       ();
   void stopAndWait();

   static int ImageLoad(ImageHeader &header, C Str &name);
   static flt ImageSharpness(C Image &image);
   static void CalcTexSharpness(UID &tex_id, ptr user, int thread_index);

   void getTexSharpnessFromProject();
   void savedTex(C UID &tex_id);

public:
   TexInfoGetter();
};
/******************************************************************************/
/******************************************************************************/
extern TexInfoGetter TIG;
/******************************************************************************/
