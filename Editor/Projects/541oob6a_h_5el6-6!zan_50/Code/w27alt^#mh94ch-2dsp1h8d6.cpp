/******************************************************************************

   'Map' is a container which stores elements of unique 'Keys' and custom 'Data'
      (there cannot be 2 elements in the same Map which have the same key)

   'Key' and 'Data' can be any kind of type.

   'Map' just like 'Memx' and 'Cache' containers preserve elements memory address on all operations.

   Accessing elements by 'Key' is fast because it's based on binary search.

/******************************************************************************/
class Key
{
   int key=0;

   Key(int key) {T.key=key;}
}

class Data
{
   Str name;
   int parameter=0;
}

int Compare(C Key &a, C Key &b) // compare 2 keys
{
   if(a.key<b.key)return -1;
   if(a.key>b.key)return +1;
                  return  0;
}

bool Create(Data &data, C Key &key, ptr user) // create element from 'key'
{
   data.name     =S+"Name made from key:"+key.key;
   data.parameter=Random(100); // some random value
   return true; // element has been created successfully, so return true
}

Map<Key, Data> map(Compare, Create, null);
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   Str name_of_key_12=map(Key(12)).name; // this will access the element of 'key=12', because this element was not yet created, 'Create' function will be called automatically

   map(Key(1)); // access element of 'key=1'

   map(Key(12)); // again access element of 'key=12', the element was already created earlier, so 'Create' will not be called, but only pointer returned

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   D.text (0, 0, S+"Number of elements in map: "+map.elms());
}
/******************************************************************************/
