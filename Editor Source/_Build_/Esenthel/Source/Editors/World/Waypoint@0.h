/******************************************************************************/
/******************************************************************************/
void WaypointRol();
void WaypointRor();
void WaypointRev();
void WaypointPreChanged(C Property &prop);
void WaypointChanged   (C Property &prop);
void WaypointIDCopy(  WorldView &world             );
void WaypointID    (  WorldView &world, C Str &text);
Str  WaypointID    (C WorldView &world             );
void WaypointName  (  WorldView &world, C Str &text);
Str  WaypointName  (C WorldView &world             );
void WaypointLoop  (  WorldView &world, C Str &text);
Str  WaypointLoop  (C WorldView &world             );
/******************************************************************************/
