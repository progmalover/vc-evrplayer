

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Mon Nov 22 18:05:44 2010
 */
/* Compiler settings for .\S3RenderableObject.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IS3RODesigner,0x78EFA844,0xED01,0x4c4a,0xA7,0xCC,0xB1,0x5D,0xBA,0xA3,0x49,0xBB);


MIDL_DEFINE_GUID(IID, IID_IS3RODesigner2,0x30A377A9,0x8A1E,0x4908,0x9B,0xF7,0x5E,0x5A,0xC0,0x3B,0x92,0xA4);


MIDL_DEFINE_GUID(IID, IID_IS3ROPlayer,0x77364153,0x40DA,0x491d,0xBF,0x62,0x8D,0x76,0x61,0x29,0x79,0x20);


MIDL_DEFINE_GUID(IID, IID_IS3ROAdvancedPlayer,0xB7BDBC5C,0xBF7A,0x413b,0x99,0xC5,0x27,0x93,0xCA,0x78,0x86,0x5D);


MIDL_DEFINE_GUID(IID, IID_IS3RenderableObject,0x3C243A09,0xA5BB,0x4c92,0x89,0x05,0x72,0x9B,0xB8,0xAF,0x53,0xEC);


MIDL_DEFINE_GUID(IID, LIBID_S3RenderableObjectLib,0x9E34392D,0x71B8,0x439f,0x9D,0xBB,0x53,0xE4,0x70,0xF5,0x07,0x9D);


MIDL_DEFINE_GUID(CLSID, CLSID_S3RenderableObject,0x4E4AFE9E,0x6A54,0x49bd,0x84,0x55,0x52,0x44,0x2A,0x99,0x74,0xDD);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



