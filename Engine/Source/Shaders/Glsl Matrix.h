#define Matrix3(m) Vec  m[3]
#define Matrix( m) Vec4 m[3]
#define Matrix4(m) Vec4 m[4]

MP Vec MatrixX  (MP Matrix(m)) {return Vec(m[0].x, m[1].x, m[2].x);}
MP Vec MatrixY  (MP Matrix(m)) {return Vec(m[0].y, m[1].y, m[2].y);}
MP Vec MatrixZ  (MP Matrix(m)) {return Vec(m[0].z, m[1].z, m[2].z);}
HP Vec MatrixPos(MP Matrix(m)) {return Vec(m[0].w, m[1].w, m[2].w);}

MP Vec Transform(MP Vec v, MP Matrix3(m))
{
   return Vec(Dot(v, m[0]),
              Dot(v, m[1]),
              Dot(v, m[2]));
}
MP Vec Transform3(MP Vec v, MP Matrix(m))
{
   return Vec(Dot(v, m[0].xyz),
              Dot(v, m[1].xyz),
              Dot(v, m[2].xyz));
}
HP Vec Transform(HP Vec v, HP Matrix(m))
{
   return Vec(Dot(v, m[0].xyz) + m[0].w,
              Dot(v, m[1].xyz) + m[1].w,
              Dot(v, m[2].xyz) + m[2].w);
}
HP Vec4 Transform(HP Vec v, HP Matrix4(m))
{
   return Vec4(Dot(v, m[0].xyz) + m[0].w,
               Dot(v, m[1].xyz) + m[1].w,
               Dot(v, m[2].xyz) + m[2].w,
               Dot(v, m[3].xyz) + m[3].w);
}
