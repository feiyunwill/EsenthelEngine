inline HP Flt  LinearizeDepth(HP Flt w, Bool perspective)
{
   HP Flt a=ProjMatrix[2][2], // ProjMatrix.z  .z !! for GL we need to swap [][] order !!
          b=ProjMatrix[2][3]; // ProjMatrix.pos.z !! for GL we need to swap [][] order !!
   w=w*2.0-1.0;
   return perspective ? b/(w-a) : (w-Flt(REVERSE_DEPTH))/a;
}
