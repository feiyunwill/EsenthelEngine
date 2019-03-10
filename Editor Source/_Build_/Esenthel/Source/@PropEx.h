/******************************************************************************/
/******************************************************************************/
class PropEx : Property
{
   static ObjPtrs<PropEx> props;

   static void Clear(PropEx &prop);

   ELM_TYPE elm_type;
   Str     _desc;

   bool compatible(ELM_TYPE elm_type)C;

   void    setDesc(                        );
   PropEx& desc   (C Str &desc             );
   PropEx& setEnum(                        );
   PropEx& setEnum(cchar8 *data[], int elms);
   PropEx& setEnum(cchar  *data[], int elms);
   PropEx& elmType(ELM_TYPE elm_type       );
   void    clear  (                        );

   T1(TYPE) PropEx& create(C Str &name,   TYPE       &member         );
            PropEx& create(C Str &name, C MemberDesc &md=MemberDesc());

   void dragStart(bool elm_types[ELM_NUM_ANY]); // enable textline only if the source has the same element type as this property
   void dragEnd  (                           );

   PropEx();
  ~PropEx();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
