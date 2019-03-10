/******************************************************************************/
/******************************************************************************/
class PhysPath // Phys Body Settings for Path Generation
{
   OBJ_PATH    mode;
   PhysBodyPtr body;

   operator bool()C;

   bool operator==(C PhysPath &pp)C;
   bool operator!=(C PhysPath &pp)C;

   void set      (OBJ_PATH mode, C PhysBodyPtr &body);
   void operator=(C Object  &obj                    );
   void operator=(C ObjData &obj                    );

   PhysPath(OBJ_PATH mode, C PhysBodyPtr &body);
   PhysPath(C Object  &obj                    );
   PhysPath(C ObjData &obj                    );

public:
   PhysPath();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
