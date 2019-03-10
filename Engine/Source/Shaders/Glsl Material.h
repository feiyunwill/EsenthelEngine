PAR LP Vec4 Highlight;

struct MaterialClass
{
   LP Vec4 _color;
   LP Vec4 _ambient_specular;
   MP Vec4 _sss_glow_rough_bump;
   HP Vec4 _texscale_detscale_detpower_reflect;
};

PAR MaterialClass Material;

LP Vec4 MaterialColor   () {return Material._color;}
LP Vec  MaterialColor3  () {return Material._color.rgb;}
LP Flt  MaterialAlpha   () {return Material._color.a;}
LP Vec  MaterialAmbient () {return Material._ambient_specular.xyz;}
MP Flt  MaterialGlow    () {return Material._sss_glow_rough_bump.y;}
HP Flt  MaterialTexScale() {return Material._texscale_detscale_detpower_reflect.x;}
LP Flt  MaterialReflect () {return Material._texscale_detscale_detpower_reflect.w;}
