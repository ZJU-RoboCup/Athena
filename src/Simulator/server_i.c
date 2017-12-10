

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Tue Apr 22 16:09:58 2014
 */
/* Compiler settings for \Users\Zjunlict\Desktop\simulator\windows\server.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if !defined(_M_IA64) && !defined(_M_AMD64)


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

MIDL_DEFINE_GUID(IID, IID_IPlayer,0x9026A6CF,0xBF02,0x486e,0xA7,0x05,0x35,0x4C,0x79,0x49,0x0E,0xA3);


MIDL_DEFINE_GUID(IID, IID_ITeam,0x729D6232,0xC5A7,0x4d5e,0x95,0xB0,0x09,0xC4,0x86,0xB8,0x76,0xFC);


MIDL_DEFINE_GUID(IID, IID_IViewer,0x630015B6,0xEBAF,0x43c0,0x9D,0x4E,0x09,0x30,0x57,0xF0,0x97,0x6A);


MIDL_DEFINE_GUID(IID, IID_IController,0xCA4D680D,0x35F8,0x4ae5,0x8A,0x2A,0x38,0x4C,0xF9,0x18,0xEF,0x1B);


MIDL_DEFINE_GUID(IID, IID_IPlayerModule,0x9977F039,0x3816,0x443f,0x85,0xE0,0x4E,0x6E,0x6B,0x1F,0x90,0x70);


MIDL_DEFINE_GUID(IID, LIBID_SmallSim,0x09E8E31D,0xB95D,0x4027,0x86,0x80,0x8F,0x47,0xF4,0xCB,0x6A,0x98);


MIDL_DEFINE_GUID(CLSID, CLSID_SimServer,0xB24B1E3D,0xF465,0x47f0,0x80,0x72,0x89,0x64,0x80,0x10,0x67,0x7F);


MIDL_DEFINE_GUID(CLSID, CLSID_SimPlayerModule,0x40211A04,0x89C3,0x44b0,0x80,0x4B,0x33,0x6B,0x20,0xE9,0x4B,0x3F);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

