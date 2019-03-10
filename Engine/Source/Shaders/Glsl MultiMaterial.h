struct MultiMaterialClass
{
   LP Vec4 _color, _normal_mul, _normal_add;
   MP Vec4 _bump_mul_add;
   HP Vec4 _texscale_detscale_detmul_detadd;
   MP Vec  _bump_macro_reflect;
};

PAR MultiMaterialClass MultiMaterial0;
PAR MultiMaterialClass MultiMaterial1;
PAR MultiMaterialClass MultiMaterial2;
PAR MultiMaterialClass MultiMaterial3;

LP Vec4 MultiMaterial0Color   () {return MultiMaterial0._color;}
LP Vec  MultiMaterial0Color3  () {return MultiMaterial0._color.rgb;}
HP Flt  MultiMaterial0TexScale() {return MultiMaterial0._texscale_detscale_detmul_detadd.x;}

LP Vec4 MultiMaterial1Color   () {return MultiMaterial1._color;}
LP Vec  MultiMaterial1Color3  () {return MultiMaterial1._color.rgb;}
HP Flt  MultiMaterial1TexScale() {return MultiMaterial1._texscale_detscale_detmul_detadd.x;}

LP Vec4 MultiMaterial2Color   () {return MultiMaterial2._color;}
LP Vec  MultiMaterial2Color3  () {return MultiMaterial2._color.rgb;}
HP Flt  MultiMaterial2TexScale() {return MultiMaterial2._texscale_detscale_detmul_detadd.x;}

LP Vec4 MultiMaterial3Color   () {return MultiMaterial3._color;}
LP Vec  MultiMaterial3Color3  () {return MultiMaterial3._color.rgb;}
HP Flt  MultiMaterial3TexScale() {return MultiMaterial3._texscale_detscale_detmul_detadd.x;}
