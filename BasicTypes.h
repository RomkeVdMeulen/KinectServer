#ifndef BASICTYPES_H_INCLUDED
#define BASICTYPES_H_INCLUDED

// Blatantly stolen from the osgrc codebase

typedef unsigned char		Byte;

typedef unsigned short int	Uint16;
typedef unsigned int	Uint32;

typedef signed short int	Int16;
typedef signed int		Int32;

typedef Uint16		prototypeID;
typedef Uint16		instanceID;

typedef char sizeof_Byte_MUST_be_1[2*((sizeof(Byte)==1)!=0)-1];//force compile error on size mismatch
typedef char sizeof_Int16_MUST_be_2[2*((sizeof(Int16)==2)!=0)-1];//force compile error on size mismatch
typedef char sizeof_Uint16_MUST_be_2[2*((sizeof(Uint16)==2)!=0)-1];//force compile error on size mismatch
typedef char sizeof_Int32_MUST_be_4[2*((sizeof(Int32)==4)!=0)-1];//force compile error on size mismatch
typedef char sizeof_Uint32_MUST_be_4[2*((sizeof(Uint32)==4)!=0)-1];//force compile error on size mismatch

typedef char sizeof_double_MUST_be_8[2*((sizeof(double)==8)!=0)-1];//force compile error on size mismatch

#ifdef OSGRC_LITTLE_ENDIAN
typedef struct Uint64 {
    Uint32     upper;
    Uint32     lower;
} Uint64 ;
#else
typedef struct Uint64 {
    Uint32     lower;
    Uint32     upper;
} Uint64 ;
#endif

#endif
