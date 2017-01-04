

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Sun Nov 21 20:15:37 2010
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


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __S3RenderableObject_h_h__
#define __S3RenderableObject_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IS3RODesigner_FWD_DEFINED__
#define __IS3RODesigner_FWD_DEFINED__
typedef interface IS3RODesigner IS3RODesigner;
#endif 	/* __IS3RODesigner_FWD_DEFINED__ */


#ifndef __IS3RODesigner2_FWD_DEFINED__
#define __IS3RODesigner2_FWD_DEFINED__
typedef interface IS3RODesigner2 IS3RODesigner2;
#endif 	/* __IS3RODesigner2_FWD_DEFINED__ */


#ifndef __IS3ROPlayer_FWD_DEFINED__
#define __IS3ROPlayer_FWD_DEFINED__
typedef interface IS3ROPlayer IS3ROPlayer;
#endif 	/* __IS3ROPlayer_FWD_DEFINED__ */


#ifndef __IS3ROAdvancedPlayer_FWD_DEFINED__
#define __IS3ROAdvancedPlayer_FWD_DEFINED__
typedef interface IS3ROAdvancedPlayer IS3ROAdvancedPlayer;
#endif 	/* __IS3ROAdvancedPlayer_FWD_DEFINED__ */


#ifndef __IS3RenderableObject_FWD_DEFINED__
#define __IS3RenderableObject_FWD_DEFINED__
typedef interface IS3RenderableObject IS3RenderableObject;
#endif 	/* __IS3RenderableObject_FWD_DEFINED__ */


#ifndef __S3RenderableObject_FWD_DEFINED__
#define __S3RenderableObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class S3RenderableObject S3RenderableObject;
#else
typedef struct S3RenderableObject S3RenderableObject;
#endif /* __cplusplus */

#endif 	/* __S3RenderableObject_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_S3RenderableObject_0000_0000 */
/* [local] */ 






typedef struct __RO_THUMBNAIL_INFO
    {
    int width;
    int height;
    int pitch;
    void *pAddr;
    } 	RO_THUMBNAIL_INFO;

typedef struct __RO_THUMBNAIL_INFO *PRO_THUMBNAIL_INFO;

typedef struct __RO_PROPERTY
    {
    TCHAR propertyName[ 256 ];
    TCHAR propertyValue[ 256 ];
    } 	RO_PROPERTY;

typedef struct __RO_PROPERTY *PRO_PROPERTY;

typedef struct __RO_PROPERTY_SETTING
    {
    DWORD propertyCount;
    PRO_PROPERTY pProperties;
    } 	RO_PROPERTY_SETTING;

typedef struct __RO_PROPERTY_SETTING *PRO_PROPERTY_SETTING;

typedef struct __RO_CONTENTINFO
    {
    DWORD duration;
    DWORD width;
    DWORD height;
    } 	RO_CONTENTINFO;

typedef struct __RO_CONTENTINFO *PRO_CONTENTINFO;



extern RPC_IF_HANDLE __MIDL_itf_S3RenderableObject_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_S3RenderableObject_0000_0000_v0_0_s_ifspec;

#ifndef __IS3RODesigner_INTERFACE_DEFINED__
#define __IS3RODesigner_INTERFACE_DEFINED__

/* interface IS3RODesigner */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IS3RODesigner;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("78EFA844-ED01-4c4a-A7CC-B15DBAA349BB")
    IS3RODesigner : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateDesigner( 
            /* [in] */ const LPCWSTR pContent,
            /* [in] */ const PRO_PROPERTY_SETTING pPropertySetting) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateContent( 
            /* [in] */ const LPCWSTR pContent,
            /* [in] */ const PRO_PROPERTY_SETTING pPropertySetting,
            /* [out] */ BOOL *pbNewThumbnail) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConfigProperty( 
            /* [in] */ HANDLE hWnd,
            /* [in] */ LANGID langID,
            /* [out] */ BOOL *pbNewThumbnail) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropertySettings( 
            /* [out] */ DWORD *pCount,
            /* [out] */ PRO_PROPERTY pProperty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetThumbnail( 
            /* [out] */ PRO_THUMBNAIL_INFO pThunbnailImage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContentInfo( 
            /* [out] */ PRO_CONTENTINFO pContentInfo) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IS3RODesignerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IS3RODesigner * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IS3RODesigner * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IS3RODesigner * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateDesigner )( 
            IS3RODesigner * This,
            /* [in] */ const LPCWSTR pContent,
            /* [in] */ const PRO_PROPERTY_SETTING pPropertySetting);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateContent )( 
            IS3RODesigner * This,
            /* [in] */ const LPCWSTR pContent,
            /* [in] */ const PRO_PROPERTY_SETTING pPropertySetting,
            /* [out] */ BOOL *pbNewThumbnail);
        
        HRESULT ( STDMETHODCALLTYPE *ConfigProperty )( 
            IS3RODesigner * This,
            /* [in] */ HANDLE hWnd,
            /* [in] */ LANGID langID,
            /* [out] */ BOOL *pbNewThumbnail);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertySettings )( 
            IS3RODesigner * This,
            /* [out] */ DWORD *pCount,
            /* [out] */ PRO_PROPERTY pProperty);
        
        HRESULT ( STDMETHODCALLTYPE *GetThumbnail )( 
            IS3RODesigner * This,
            /* [out] */ PRO_THUMBNAIL_INFO pThunbnailImage);
        
        HRESULT ( STDMETHODCALLTYPE *GetContentInfo )( 
            IS3RODesigner * This,
            /* [out] */ PRO_CONTENTINFO pContentInfo);
        
        END_INTERFACE
    } IS3RODesignerVtbl;

    interface IS3RODesigner
    {
        CONST_VTBL struct IS3RODesignerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IS3RODesigner_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IS3RODesigner_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IS3RODesigner_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IS3RODesigner_CreateDesigner(This,pContent,pPropertySetting)	\
    ( (This)->lpVtbl -> CreateDesigner(This,pContent,pPropertySetting) ) 

#define IS3RODesigner_UpdateContent(This,pContent,pPropertySetting,pbNewThumbnail)	\
    ( (This)->lpVtbl -> UpdateContent(This,pContent,pPropertySetting,pbNewThumbnail) ) 

#define IS3RODesigner_ConfigProperty(This,hWnd,langID,pbNewThumbnail)	\
    ( (This)->lpVtbl -> ConfigProperty(This,hWnd,langID,pbNewThumbnail) ) 

#define IS3RODesigner_GetPropertySettings(This,pCount,pProperty)	\
    ( (This)->lpVtbl -> GetPropertySettings(This,pCount,pProperty) ) 

#define IS3RODesigner_GetThumbnail(This,pThunbnailImage)	\
    ( (This)->lpVtbl -> GetThumbnail(This,pThunbnailImage) ) 

#define IS3RODesigner_GetContentInfo(This,pContentInfo)	\
    ( (This)->lpVtbl -> GetContentInfo(This,pContentInfo) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IS3RODesigner_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_S3RenderableObject_0000_0001 */
/* [local] */ 

typedef 
enum __RO_PROPERTY_TYPE
    {	S3RO_PT_INT	= 0,
	S3RO_PT_FLOAT	= 1,
	S3RO_PT_STR	= 2,
	S3RO_PT_INTB	= 3,
	S3RO_PT_INTLIST	= 4
    } 	RO_PROPERTY_TYPE;

typedef struct __RO_PROPERTY_INFO
    {
    DWORD PID;
    RO_PROPERTY_TYPE PType;
    TCHAR InfoString[ 1024 ];
    union 
        {
        struct 
            {
            int defaultI;
            int minI;
            int maxI;
            } 	;
        struct 
            {
            float defaultF;
            float minF;
            float maxF;
            } 	;
        TCHAR defaultStr[ 256 ];
        } 	;
    } 	RO_PROPERTY_INFO;

typedef struct __RO_PROPERTY_INFO *PRO_PROPERTY_INFO;



extern RPC_IF_HANDLE __MIDL_itf_S3RenderableObject_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_S3RenderableObject_0000_0001_v0_0_s_ifspec;

#ifndef __IS3RODesigner2_INTERFACE_DEFINED__
#define __IS3RODesigner2_INTERFACE_DEFINED__

/* interface IS3RODesigner2 */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IS3RODesigner2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("30A377A9-8A1E-4908-9BF7-5E5AC03B92A4")
    IS3RODesigner2 : public IS3RODesigner
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetProperties( 
            /* [in] */ LANGID langID,
            /* [out] */ DWORD *pPropertyCount,
            /* [out] */ PRO_PROPERTY_INFO pProperties) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropertyI( 
            /* [in] */ DWORD PID,
            /* [in] */ int value,
            /* [out] */ BOOL *pbNewThumbnail) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropertyF( 
            /* [in] */ DWORD PID,
            /* [in] */ float value,
            /* [out] */ BOOL *pbNewThumbnail) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetPropertyS( 
            /* [in] */ DWORD PID,
            /* [in] */ const LPCWSTR pStr,
            /* [out] */ BOOL *pbNewThumbnail) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IS3RODesigner2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IS3RODesigner2 * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IS3RODesigner2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IS3RODesigner2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateDesigner )( 
            IS3RODesigner2 * This,
            /* [in] */ const LPCWSTR pContent,
            /* [in] */ const PRO_PROPERTY_SETTING pPropertySetting);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateContent )( 
            IS3RODesigner2 * This,
            /* [in] */ const LPCWSTR pContent,
            /* [in] */ const PRO_PROPERTY_SETTING pPropertySetting,
            /* [out] */ BOOL *pbNewThumbnail);
        
        HRESULT ( STDMETHODCALLTYPE *ConfigProperty )( 
            IS3RODesigner2 * This,
            /* [in] */ HANDLE hWnd,
            /* [in] */ LANGID langID,
            /* [out] */ BOOL *pbNewThumbnail);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropertySettings )( 
            IS3RODesigner2 * This,
            /* [out] */ DWORD *pCount,
            /* [out] */ PRO_PROPERTY pProperty);
        
        HRESULT ( STDMETHODCALLTYPE *GetThumbnail )( 
            IS3RODesigner2 * This,
            /* [out] */ PRO_THUMBNAIL_INFO pThunbnailImage);
        
        HRESULT ( STDMETHODCALLTYPE *GetContentInfo )( 
            IS3RODesigner2 * This,
            /* [out] */ PRO_CONTENTINFO pContentInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperties )( 
            IS3RODesigner2 * This,
            /* [in] */ LANGID langID,
            /* [out] */ DWORD *pPropertyCount,
            /* [out] */ PRO_PROPERTY_INFO pProperties);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropertyI )( 
            IS3RODesigner2 * This,
            /* [in] */ DWORD PID,
            /* [in] */ int value,
            /* [out] */ BOOL *pbNewThumbnail);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropertyF )( 
            IS3RODesigner2 * This,
            /* [in] */ DWORD PID,
            /* [in] */ float value,
            /* [out] */ BOOL *pbNewThumbnail);
        
        HRESULT ( STDMETHODCALLTYPE *SetPropertyS )( 
            IS3RODesigner2 * This,
            /* [in] */ DWORD PID,
            /* [in] */ const LPCWSTR pStr,
            /* [out] */ BOOL *pbNewThumbnail);
        
        END_INTERFACE
    } IS3RODesigner2Vtbl;

    interface IS3RODesigner2
    {
        CONST_VTBL struct IS3RODesigner2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IS3RODesigner2_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IS3RODesigner2_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IS3RODesigner2_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IS3RODesigner2_CreateDesigner(This,pContent,pPropertySetting)	\
    ( (This)->lpVtbl -> CreateDesigner(This,pContent,pPropertySetting) ) 

#define IS3RODesigner2_UpdateContent(This,pContent,pPropertySetting,pbNewThumbnail)	\
    ( (This)->lpVtbl -> UpdateContent(This,pContent,pPropertySetting,pbNewThumbnail) ) 

#define IS3RODesigner2_ConfigProperty(This,hWnd,langID,pbNewThumbnail)	\
    ( (This)->lpVtbl -> ConfigProperty(This,hWnd,langID,pbNewThumbnail) ) 

#define IS3RODesigner2_GetPropertySettings(This,pCount,pProperty)	\
    ( (This)->lpVtbl -> GetPropertySettings(This,pCount,pProperty) ) 

#define IS3RODesigner2_GetThumbnail(This,pThunbnailImage)	\
    ( (This)->lpVtbl -> GetThumbnail(This,pThunbnailImage) ) 

#define IS3RODesigner2_GetContentInfo(This,pContentInfo)	\
    ( (This)->lpVtbl -> GetContentInfo(This,pContentInfo) ) 


#define IS3RODesigner2_GetProperties(This,langID,pPropertyCount,pProperties)	\
    ( (This)->lpVtbl -> GetProperties(This,langID,pPropertyCount,pProperties) ) 

#define IS3RODesigner2_SetPropertyI(This,PID,value,pbNewThumbnail)	\
    ( (This)->lpVtbl -> SetPropertyI(This,PID,value,pbNewThumbnail) ) 

#define IS3RODesigner2_SetPropertyF(This,PID,value,pbNewThumbnail)	\
    ( (This)->lpVtbl -> SetPropertyF(This,PID,value,pbNewThumbnail) ) 

#define IS3RODesigner2_SetPropertyS(This,PID,pStr,pbNewThumbnail)	\
    ( (This)->lpVtbl -> SetPropertyS(This,PID,pStr,pbNewThumbnail) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IS3RODesigner2_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_S3RenderableObject_0000_0002 */
/* [local] */ 

typedef struct __RO_TEXCOORD
    {
    float tu0;
    float tv0;
    float tu1;
    float tv1;
    } 	RO_TEXCOORD;

typedef struct __RO_RENDERRECT
    {
    RECT theRect;
    RO_TEXCOORD TexCoord;
    } 	RO_RENDERRECT;

typedef struct __RO_RENDERRECT *PRO_RENDERRECT;

typedef struct __RO_RENDERPARAM
    {
    RECT contentDirtyRect;
    UINT drawObjectIndex;
    RO_RENDERRECT renderRect;
    } 	RO_RENDERPARAM;

typedef struct __RO_RENDERPARAM *PRO_RENDERPARAM;



extern RPC_IF_HANDLE __MIDL_itf_S3RenderableObject_0000_0002_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_S3RenderableObject_0000_0002_v0_0_s_ifspec;

#ifndef __IS3ROPlayer_INTERFACE_DEFINED__
#define __IS3ROPlayer_INTERFACE_DEFINED__

/* interface IS3ROPlayer */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IS3ROPlayer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("77364153-40DA-491d-BF62-8D7661297920")
    IS3ROPlayer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreatePlayer( 
            /* [in] */ SIZE rectSize,
            /* [in] */ const LPCWSTR pContent,
            /* [in] */ const PRO_PROPERTY_SETTING pPropertySetting,
            /* [out] */ SIZE *pDrawSize,
            /* [out] */ BOOL *bTransparent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ HANDLE hDrawObject) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Run( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Seek( 
            UINT absolutePosition) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PrepareRender( 
            /* [out] */ PRO_RENDERPARAM pParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndRender( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ProcessMessage( 
            /* [in] */ UINT uMsg,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IS3ROPlayerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IS3ROPlayer * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IS3ROPlayer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IS3ROPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePlayer )( 
            IS3ROPlayer * This,
            /* [in] */ SIZE rectSize,
            /* [in] */ const LPCWSTR pContent,
            /* [in] */ const PRO_PROPERTY_SETTING pPropertySetting,
            /* [out] */ SIZE *pDrawSize,
            /* [out] */ BOOL *bTransparent);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IS3ROPlayer * This,
            /* [in] */ HANDLE hDrawObject);
        
        HRESULT ( STDMETHODCALLTYPE *Run )( 
            IS3ROPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IS3ROPlayer * This);

        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IS3ROPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Seek )( 
            IS3ROPlayer * This,
            UINT absolutePosition);
        
        HRESULT ( STDMETHODCALLTYPE *PrepareRender )( 
            IS3ROPlayer * This,
            /* [out] */ PRO_RENDERPARAM pParam);
        
        HRESULT ( STDMETHODCALLTYPE *EndRender )( 
            IS3ROPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessMessage )( 
            IS3ROPlayer * This,
            /* [in] */ UINT uMsg,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam);
        
        END_INTERFACE
    } IS3ROPlayerVtbl;

    interface IS3ROPlayer
    {
        CONST_VTBL struct IS3ROPlayerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IS3ROPlayer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IS3ROPlayer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IS3ROPlayer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IS3ROPlayer_CreatePlayer(This,rectSize,pContent,pPropertySetting,pDrawSize,bTransparent)	\
    ( (This)->lpVtbl -> CreatePlayer(This,rectSize,pContent,pPropertySetting,pDrawSize,bTransparent) ) 

#define IS3ROPlayer_Initialize(This,hDrawObject)	\
    ( (This)->lpVtbl -> Initialize(This,hDrawObject) ) 

#define IS3ROPlayer_Run(This)	\
    ( (This)->lpVtbl -> Run(This) ) 

#define IS3ROPlayer_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IS3ROPlayer_Pause(This)	\
    ( (This)->lpVtbl -> Pause(This) ) 

#define IS3ROPlayer_Seek(This,absolutePosition)	\
    ( (This)->lpVtbl -> Seek(This,absolutePosition) ) 

#define IS3ROPlayer_PrepareRender(This,pParam)	\
    ( (This)->lpVtbl -> PrepareRender(This,pParam) ) 

#define IS3ROPlayer_EndRender(This)	\
    ( (This)->lpVtbl -> EndRender(This) ) 

#define IS3ROPlayer_ProcessMessage(This,uMsg,wParam,lParam)	\
    ( (This)->lpVtbl -> ProcessMessage(This,uMsg,wParam,lParam) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IS3ROPlayer_INTERFACE_DEFINED__ */


#ifndef __IS3ROAdvancedPlayer_INTERFACE_DEFINED__
#define __IS3ROAdvancedPlayer_INTERFACE_DEFINED__

/* interface IS3ROAdvancedPlayer */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IS3ROAdvancedPlayer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B7BDBC5C-BF7A-413b-99C5-2793CA78865D")
    IS3ROAdvancedPlayer : public IS3ROPlayer
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateAdvancedPlayer( 
            /* [in] */ SIZE rectSize,
            /* [in] */ const LPCWSTR pContent,
            /* [in] */ const PRO_PROPERTY_SETTING pPropertySetting,
            /* [out] */ SIZE *pDrawSize,
            /* [out] */ BOOL *bTransparent,
            /* [out] */ BOOL *bNeedDoubleBuffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ DWORD objCount,
            /* [in] */ HANDLE *pHDrawObject) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IS3ROAdvancedPlayerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IS3ROAdvancedPlayer * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IS3ROAdvancedPlayer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IS3ROAdvancedPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreatePlayer )( 
            IS3ROAdvancedPlayer * This,
            /* [in] */ SIZE rectSize,
            /* [in] */ const LPCWSTR pContent,
            /* [in] */ const PRO_PROPERTY_SETTING pPropertySetting,
            /* [out] */ SIZE *pDrawSize,
            /* [out] */ BOOL *bTransparent);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IS3ROAdvancedPlayer * This,
            /* [in] */ HANDLE hDrawObject);
        
        HRESULT ( STDMETHODCALLTYPE *Run )( 
            IS3ROAdvancedPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IS3ROAdvancedPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *PrepareRender )( 
            IS3ROAdvancedPlayer * This,
            /* [out] */ PRO_RENDERPARAM pParam);
        
        HRESULT ( STDMETHODCALLTYPE *EndRender )( 
            IS3ROAdvancedPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *ProcessMessage )( 
            IS3ROAdvancedPlayer * This,
            /* [in] */ UINT uMsg,
            /* [in] */ WPARAM wParam,
            /* [in] */ LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *CreateAdvancedPlayer )( 
            IS3ROAdvancedPlayer * This,
            /* [in] */ SIZE rectSize,
            /* [in] */ const LPCWSTR pContent,
            /* [in] */ const PRO_PROPERTY_SETTING pPropertySetting,
            /* [out] */ SIZE *pDrawSize,
            /* [out] */ BOOL *bTransparent,
            /* [out] */ BOOL *bNeedDoubleBuffer);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IS3ROAdvancedPlayer * This,
            /* [in] */ DWORD objCount,
            /* [in] */ HANDLE *pHDrawObject);
        
        END_INTERFACE
    } IS3ROAdvancedPlayerVtbl;

    interface IS3ROAdvancedPlayer
    {
        CONST_VTBL struct IS3ROAdvancedPlayerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IS3ROAdvancedPlayer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IS3ROAdvancedPlayer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IS3ROAdvancedPlayer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IS3ROAdvancedPlayer_CreatePlayer(This,rectSize,pContent,pPropertySetting,pDrawSize,bTransparent)	\
    ( (This)->lpVtbl -> CreatePlayer(This,rectSize,pContent,pPropertySetting,pDrawSize,bTransparent) ) 

#define IS3ROAdvancedPlayer_Initialize(This,hDrawObject)	\
    ( (This)->lpVtbl -> Initialize(This,hDrawObject) ) 

#define IS3ROAdvancedPlayer_Run(This)	\
    ( (This)->lpVtbl -> Run(This) ) 

#define IS3ROAdvancedPlayer_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IS3ROAdvancedPlayer_PrepareRender(This,pParam)	\
    ( (This)->lpVtbl -> PrepareRender(This,pParam) ) 

#define IS3ROAdvancedPlayer_EndRender(This)	\
    ( (This)->lpVtbl -> EndRender(This) ) 

#define IS3ROAdvancedPlayer_ProcessMessage(This,uMsg,wParam,lParam)	\
    ( (This)->lpVtbl -> ProcessMessage(This,uMsg,wParam,lParam) ) 


#define IS3ROAdvancedPlayer_CreateAdvancedPlayer(This,rectSize,pContent,pPropertySetting,pDrawSize,bTransparent,bNeedDoubleBuffer)	\
    ( (This)->lpVtbl -> CreateAdvancedPlayer(This,rectSize,pContent,pPropertySetting,pDrawSize,bTransparent,bNeedDoubleBuffer) ) 

#define IS3ROAdvancedPlayer_Initialize(This,objCount,pHDrawObject)	\
    ( (This)->lpVtbl -> Initialize(This,objCount,pHDrawObject) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IS3ROAdvancedPlayer_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_S3RenderableObject_0000_0004 */
/* [local] */ 

typedef 
enum __S3RO_WORKMODE
    {	S3RO_WM_UNKOWN	= 0,
	S3RO_WM_DESIGNER	= 1,
	S3RO_WM_PLAYER	= 2,
	S3RO_DWALIGN	= 0xffffffff
    } 	S3RO_WORKMODE;

typedef struct __RO_SUPPORTEDFILE
    {
    TCHAR FileExt[ 64 ];
    BOOL bPrefix;
    DWORD priority;
    } 	RO_SUPPORTEDFILE;

typedef struct __RO_SUPPORTEDFILE *PRO_SUPPORTEDFILE;



extern RPC_IF_HANDLE __MIDL_itf_S3RenderableObject_0000_0004_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_S3RenderableObject_0000_0004_v0_0_s_ifspec;

#ifndef __IS3RenderableObject_INTERFACE_DEFINED__
#define __IS3RenderableObject_INTERFACE_DEFINED__

/* interface IS3RenderableObject */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IS3RenderableObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3C243A09-A5BB-4c92-8905-729BB8AF53EC")
    IS3RenderableObject : public IUnknown
    {
    public:
        
        virtual HRESULT STDMETHODCALLTYPE GetSupportedFiles( 
            /* [out] */ DWORD *pCount,
            /* [out] */ PRO_SUPPORTEDFILE pFileList) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIcon( 
            /* [out] */ HANDLE *phIcon) = 0;

        virtual BOOL STDMETHODCALLTYPE WSWGSupported( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IS3RenderableObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IS3RenderableObject * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IS3RenderableObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IS3RenderableObject * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetSupportedFiles )( 
            IS3RenderableObject * This,
            /* [out] */ DWORD *pCount,
            /* [out] */ PRO_SUPPORTEDFILE pFileList);
        
        HRESULT ( STDMETHODCALLTYPE *GetIcon )( 
            IS3RenderableObject * This,
            /* [out] */ HANDLE *phIcon);

        BOOL ( STDMETHODCALLTYPE *WSWGSupported )( 
            IS3RenderableObject * This);
        
        END_INTERFACE
    } IS3RenderableObjectVtbl;

    interface IS3RenderableObject
    {
        CONST_VTBL struct IS3RenderableObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IS3RenderableObject_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IS3RenderableObject_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IS3RenderableObject_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 

#define IS3RenderableObject_GetSupportedFiles(This,pCount,pFileList)	\
    ( (This)->lpVtbl -> GetSupportedFiles(This,pCount,pFileList) ) 

#define IS3RenderableObject_GetIcon(This,phIcon)	\
    ( (This)->lpVtbl -> GetIcon(This,phIcon) ) 

#define IS3RenderableObject_WSWGSupported(This)	\
    ( (This)->lpVtbl -> WSWGSupported(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IS3RenderableObject_INTERFACE_DEFINED__ */



#ifndef __S3RenderableObjectLib_LIBRARY_DEFINED__
#define __S3RenderableObjectLib_LIBRARY_DEFINED__

/* library S3RenderableObjectLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_S3RenderableObjectLib;

EXTERN_C const CLSID CLSID_S3RenderableObject;

#ifdef __cplusplus

class DECLSPEC_UUID("4E4AFE9E-6A54-49bd-8455-52442A9974DD")
S3RenderableObject;
#endif
#endif /* __S3RenderableObjectLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


