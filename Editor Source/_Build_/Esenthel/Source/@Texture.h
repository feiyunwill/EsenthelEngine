/******************************************************************************/
/******************************************************************************/
class Texture
{
   UID id, // texture id
       src_tex_id; // if this is a dynamically generated texture, then 'src_tex_id' points to the original texture from which it was created
   bool  uses_alpha, // if uses alpha channel
            keep_hq, // if keep high quality and don't convert to low quality format
        mtrl_base_1, // if this is material base_1 texture
         regenerate; // if this texture needs to be regenerated
   byte        downsize; // blur mip map range

   Texture& downSize(int size);

   static int CompareTex(C Texture &tex, C UID &tex_id);

public:
   Texture();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
