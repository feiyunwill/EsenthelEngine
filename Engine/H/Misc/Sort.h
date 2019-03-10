/******************************************************************************

   Use 'Sort' functions to sort custom data.

   Use 'BinarySearch' functions to perform binary search on custom data.

/******************************************************************************/
// Basic Compare Functions
inline Int Compare(C SByte  &a, C SByte  &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Byte   &a, C Byte   &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Short  &a, C Short  &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C UShort &a, C UShort &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Int    &a, C Int    &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C UInt   &a, C UInt   &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Long   &a, C Long   &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C ULong  &a, C ULong  &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Half   &a, C Half   &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Flt    &a, C Flt    &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Dbl    &a, C Dbl    &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1
inline Int Compare(C Ptr    &a, C Ptr    &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1

T1(TYPE) ENABLE_IF_ENUM(TYPE, Int) Compare(C TYPE &a, C TYPE &b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' enum values and return -1, 0, +1

inline Int ComparePtr(CPtr a, CPtr b) {if(a<b)return -1; if(a>b)return +1; return 0;} // compare 'a' 'b' values and return -1, 0, +1

inline Int CompareEps(C Flt &a, C Flt &b) {return SignEps(a-b);}

inline Int Compare(C IndexWeight &a, C IndexWeight &b) {return Compare(b.weight, a.weight);} // compare in reversed order because we're sorting from most to least important

// Sort Data
         void Sort(Int  *data, Int elms                                           ); // sort Int    array
         void Sort(Flt  *data, Int elms                                           ); // sort Flt    array
         void Sort(Dbl  *data, Int elms                                           ); // sort Dbl    array
T1(TYPE) void Sort(TYPE *data, Int elms, Int compare(C TYPE &a, C TYPE &b)=Compare); // sort custom array using custom comparing function

// Binary Search, search sorted 'data' array for presence of 'value' and return if it was found in the array, 'index'=if the function returned true then this index points to the location where the 'value' is located in the array, if the function returned false then it means that 'value' was not found in the array however the 'index' points to the place where it should be added in the array while preserving sorted data, 'index' will always be in range (0..elms) inclusive
T2(DATA, VALUE)  Bool   BinarySearch(C DATA *data, Int elms, C VALUE &value, Int &index, Int compare(C DATA &a, C VALUE &b)=Compare);
T2(DATA, VALUE)  Bool   BinaryHas   (C DATA *data, Int elms, C VALUE &value,             Int compare(C DATA &a, C VALUE &b)=Compare) {Int i; return BinarySearch(data, elms, value, i, compare);                  } // check if 'value' is present in array
T2(DATA, VALUE)  DATA*  BinaryFind  (  DATA *data, Int elms, C VALUE &value,             Int compare(C DATA &a, C VALUE &b)=Compare) {Int i; return BinarySearch(data, elms, value, i, compare) ? &data[i] : null;} // check if 'value' is present in array and return it, null on fail
/******************************************************************************/
