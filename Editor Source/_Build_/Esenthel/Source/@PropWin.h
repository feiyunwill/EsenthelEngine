/******************************************************************************/
class PropWin : ClosableWindow
{
   Memx<PropEx> props;
   TextBlack    ts;

   Rect create(C Str &name, C Vec2 &lu=Vec2(0.02f, -0.02f), C Vec2 &text_scale=0.036f, flt property_height=0.043f, flt value_width=0.3f);
   PropEx &     add (C Str &name=S, C MemberDesc &md=MemberDesc());                                      
   PropWin&   toGui (         );                                        
   PropWin&   toGui (cptr data);                                        
   PropWin& autoData( ptr data);                                           ptr autoData()C;         
   PropWin& changed (void (*changed)(C Property &prop), void (*pre_changed)(C Property &prop)=null);

   virtual PropWin& hide()override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
