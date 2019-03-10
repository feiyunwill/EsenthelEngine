/******************************************************************************/
/******************************************************************************/
class IconSettsEditor : PropWin
{
   class Change : Edit::_Undo::Change
   {
      IconSettings data;

      virtual void create(ptr user)override;
      virtual void apply(ptr user)override;
   };

   UID          elm_id;
   Elm         *elm;
   bool         changed;
   IconSettings edit;
   Button       undo, redo, locate;
   Edit::Undo<Change> undos;   void undoVis();

   ElmIconSetts* data()C;

   static cchar8 *IconSizes[]
;

   static void PreChanged(C Property &prop);
   static void    Changed(C Property &prop);

   static void Width        (  IconSettsEditor &ie, C Str &t);
   static Str  Width        (C IconSettsEditor &ie          );
   static void Height       (  IconSettsEditor &ie, C Str &t);
   static Str  Height       (C IconSettsEditor &ie          );
   static void Type         (  IconSettsEditor &ie, C Str &t);
   static Str  Type         (C IconSettsEditor &ie          );
   static void MipMaps      (  IconSettsEditor &ie, C Str &t);
   static Str  MipMaps      (C IconSettsEditor &ie          );
   static void AutoCenter   (  IconSettsEditor &ie, C Str &t);
   static Str  AutoCenter   (C IconSettsEditor &ie          );
   static void Scale        (  IconSettsEditor &ie, C Str &t);
   static Str  Scale        (C IconSettsEditor &ie          );
   static void CamYaw       (  IconSettsEditor &ie, C Str &t);
   static Str  CamYaw       (C IconSettsEditor &ie          );
   static void CamPitch     (  IconSettsEditor &ie, C Str &t);
   static Str  CamPitch     (C IconSettsEditor &ie          );
   static void CamRoll      (  IconSettsEditor &ie, C Str &t);
   static Str  CamRoll      (C IconSettsEditor &ie          );
   static void CamFocusX    (  IconSettsEditor &ie, C Str &t);
   static Str  CamFocusX    (C IconSettsEditor &ie          );
   static void CamFocusY    (  IconSettsEditor &ie, C Str &t);
   static Str  CamFocusY    (C IconSettsEditor &ie          );
   static void CamFocusZ    (  IconSettsEditor &ie, C Str &t);
   static Str  CamFocusZ    (C IconSettsEditor &ie          );
   static void BloomOriginal(  IconSettsEditor &ie, C Str &t);
   static Str  BloomOriginal(C IconSettsEditor &ie          );
   static void BloomScale   (  IconSettsEditor &ie, C Str &t);
   static Str  BloomScale   (C IconSettsEditor &ie          );
   static void BloomCut     (  IconSettsEditor &ie, C Str &t);
   static Str  BloomCut     (C IconSettsEditor &ie          );
   static void Light0Shadow (  IconSettsEditor &ie, C Str &t);
   static Str  Light0Shadow (C IconSettsEditor &ie          );
   static void Light1Shadow (  IconSettsEditor &ie, C Str &t);
   static Str  Light1Shadow (C IconSettsEditor &ie          );
   static void AmbientCol   (  IconSettsEditor &ie, C Str &t);
   static Str  AmbientCol   (C IconSettsEditor &ie          );
   static void AmbientOccl  (  IconSettsEditor &ie, C Str &t);
   static Str  AmbientOccl  (C IconSettsEditor &ie          );
   static void AmbientRange (  IconSettsEditor &ie, C Str &t);
   static Str  AmbientRange (C IconSettsEditor &ie          );
   static void Light0Col    (  IconSettsEditor &ie, C Str &t);
   static Str  Light0Col    (C IconSettsEditor &ie          );
   static void Light0Yaw    (  IconSettsEditor &ie, C Str &t);
   static Str  Light0Yaw    (C IconSettsEditor &ie          );
   static void Light0Pitch  (  IconSettsEditor &ie, C Str &t);
   static Str  Light0Pitch  (C IconSettsEditor &ie          );
   static void Light1Col    (  IconSettsEditor &ie, C Str &t);
   static Str  Light1Col    (C IconSettsEditor &ie          );
   static void Light1Yaw    (  IconSettsEditor &ie, C Str &t);
   static Str  Light1Yaw    (C IconSettsEditor &ie          );
   static void Light1Pitch  (  IconSettsEditor &ie, C Str &t);
   static Str  Light1Pitch  (C IconSettsEditor &ie          );
   static void Fov          (  IconSettsEditor &ie, C Str &t);
   static Str  Fov          (C IconSettsEditor &ie          );

   static void Undo  (IconSettsEditor &editor);
   static void Redo  (IconSettsEditor &editor);
   static void Locate(IconSettsEditor &editor);

   void create();

   virtual IconSettsEditor& hide()override;

   void flush();
   void setChanged();
   void set(Elm *elm);
   void activate(Elm *elm);       
   void toggle  (Elm *elm);       
   void erasing(C UID &elm_id);   
   void elmChanged(C UID &elm_id);

public:
   IconSettsEditor();
};
/******************************************************************************/
/******************************************************************************/
extern IconSettsEditor IconSettsEdit;
/******************************************************************************/
