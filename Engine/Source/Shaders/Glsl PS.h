#define AlphaTest(alpha) if(alpha+MaterialAlpha()<1.0)discard

PAR Image     Col, Col1, Col2, Col3,
              Nrm, Nrm1, Nrm2, Nrm3,
              Lum;
PAR HP Image  Depth;
PAR ImageCube Rfl, Rfl1, Rfl2, Rfl3;

PAR MP Flt AllowBackFlip;
#define BackFlip(dir) {if(gl_FrontFacing==false)dir*=AllowBackFlip;} // keep this as "gl_FrontFacing==false" instead of "!gl_FrontFacing" because Mac OpenGL drivers for Intel fail to compile this correctly, resulting in reversed lighting
MP Flt MultiMaterialWeight(MP Flt weight, MP Flt alpha) {return Max(weight+weight*(1.0-weight)*(alpha*10.0-5.0), weight/16.0);}
MP Vec UpdateVelocities_PS(MP Vec vel, HP Vec view_space_pos)
{
   vel/=view_space_pos.z;

//#if !SIGNED_VEL_RT
   vel=vel*0.5+0.5; // scale from signed to unsigned (-1..1 -> 0..1)
//#endif

   return vel;
}
