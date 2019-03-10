/******************************************************************************

   Undo supports two modes:
      -Full
      -Partial

   For "Full" mode each Change must contain the entire copy of the data.

   For "Partial" mode each Change contains only information about the data that was changed,
      it is the user's responsibility to design the "Change" class to properly store that information.

   Full mode may be easier to implement, however it may use a lot more memory for big amounts of data.

/******************************************************************************/
namespace Edit{
/******************************************************************************/
struct _Undo // Undo (do not use this class, use 'Undo' below)
{
   struct Change
   {
      virtual UInt memUsage()C {return 0;} // override this method and return the memory usage of this Undo Change

      // methods used in "Full" mode
      virtual void create(Ptr user) {} // this is called when current state was requested to be saved     , inside this method you should copy entire state of data into this class, 'user'=user data with its value taken from 'Undo.user'
      virtual void apply (Ptr user) {} // this is called when this Undo Change was requested to be applied, inside this method you should copy entire state from this class to data, 'user'=user data with its value taken from 'Undo.user'

      // methods used in "Partial" mode
      virtual void swap(Ptr user) {} // this is called when this Undo Change was requested to be applied, inside this method you should swap the changes between this class and data, 'user'=user data with its value taken from 'Undo.user'
   };

   Ptr user; // custom user data that will be passed to 'Change' methods
   Flt time; // max time between multiple changes to consider them as one

   // apply Undo/Redo
   Bool undo        (); // perform Undo by calling 'Change.apply', false on fail (if there are no Undos)
   Bool redo        (); // perform Redo by calling 'Change.apply', false on fail (if there are no Redos)
   Bool undoAsChange(); // perform Undo by calling 'Change.apply', false on fail (if there are no Undos), this method applies the Undo as if it is a new change to the data

   // get
   Bool undosAvailable ()C {return                 undos()>0;} // if  any Undos are avaialble
   Bool redosAvailable ()C {return                 redos()>0;} // if  any Redos are avaialble
    Int undos          ()C {return                    _undos;} // get number of available Undos
    Int redos          ()C {return          changes()-_undos;} // get number of available Redos
   CPtr lastChangeType ()C {return              _change_type;} // get last type of change that was applied in CPtr format
    Int lastChangeTypeI()C {return (Int)(IntPtr)_change_type;} // get last type of change that was applied in  Int format

   Change* getNextUndo(); // get next undo in line, null is returned if there's no undo
   Change* getNextRedo(); // get next redo in line, null is returned if there's no redo

   Change& operator[](  Int     i     ) {return _changes           [i     ];} // get i-th change
   Change* addr      (  Int     i     ) {return _changes.addr      (i     );} // get i-th change address, null is returned if index is out of range
   Int     index     (C Change *change)C{return _changes.validIndex(change);} // get index of change, -1 on fail

   // operations
  _Undo& del                (); // reset to default empty state, this removes all Undo Changes but keeps max allowed limit settings
  _Undo& forceCreateNextUndo(); // always allow the next 'set' call to create a new Undo Change

   // max allowed limit settings
   Long memUsage()C;  _Undo& maxMemUsage(Long limit); // get current memory  usage / set max allowed memory usage for all Undo Changes (-1=unlimited, default=-1)
   Int  changes ()C;  _Undo& maxChanges (Int  limit); // get current changes count / set max allowed number of        all Undo Changes (-1=unlimited, default=-1)

private:
   Bool         _full;
   Int          _undos, _max_changes;
   UInt         _change_frame;
   Long         _max_mem_usage;
   Dbl          _change_time;
   CPtr         _change_type;
   Memx<Change> _changes;

   Change* set(CPtr change_type, Bool force_create, Flt extra_time);
   Change* set( Int change_type, Bool force_create, Flt extra_time);
#if EE_PRIVATE
   void clean(Bool test_mem_usage);
#endif

   explicit _Undo(Bool full, Ptr user, Flt time);
   T1(TYPE) friend struct Undo;
};
/******************************************************************************/
T1(TYPE) struct Undo : _Undo // 'TYPE' must be based on '_Undo.Change'
{
   explicit Undo(Bool full, Ptr user=null, Flt time=1.0f); // 'full'=if use "Full" or "Partial" mode, 'user'=custom user data that will be passed to 'Change' methods, 'time'=max time between multiple changes to consider them as one

   TYPE* getNextUndo(); // get next undo in line, null is returned if there's no undo
   TYPE* getNextRedo(); // get next redo in line, null is returned if there's no redo

   TYPE& operator[](Int i); // get i-th change
   TYPE* addr      (Int i); // get i-th change address, null is returned if index is out of range

   // create Undo Change
   TYPE* set(CPtr change_type=null, Bool force_create=false, Flt extra_time=0); // create a new Undo Change, call this if data associated with this Undo is going to be changed with a specific type, 'change_type'=unique pointer allowing to differentiate between types (it can point  to anything as that data is not accessed but only the memory address is, recommended way is to pass C++ strings like "move", "rotate"), by default new Undo Changes will not be created if in a short while ago (based on 'Undo.time') there was a change with a similar 'change_type', however setting 'force_create' to true will always force creation of a new Undo Change, this method will return the pointer to a new Undo Change if it was created and null if it wasn't. 'extra_time'=extra time for this particular change that would allow it to be considered the same as the last one. In Undo "Partial" mode this method will always return a Change object, in case the new change is of the same type as the last one and was requested within a short amount of time, then the last Change object will be returned.
   TYPE* set( Int change_type     , Bool force_create=false, Flt extra_time=0); // create a new Undo Change, call this if data associated with this Undo is going to be changed with a specific type, 'change_type'=unique value   allowing to differentiate between types (it can be set to anything                                                            , recommended way is to use  C++ custom enum value            ), by default new Undo Changes will not be created if in a short while ago (based on 'Undo.time') there was a change with a similar 'change_type', however setting 'force_create' to true will always force creation of a new Undo Change, this method will return the pointer to a new Undo Change if it was created and null if it wasn't. 'extra_time'=extra time for this particular change that would allow it to be considered the same as the last one. In Undo "Partial" mode this method will always return a Change object, in case the new change is of the same type as the last one and was requested within a short amount of time, then the last Change object will be returned.

   T1(CHANGE) Undo& replaceClass();
};
/******************************************************************************/
} // namespace
/******************************************************************************/
inline Int Elms(C Edit::_Undo &undo) {return undo.changes();}
/******************************************************************************/
