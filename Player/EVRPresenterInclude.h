//////////////////////////////////////////////////////////////////////////
//
// EVRPresenter.h : Internal header for building the DLL.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <evr9.h>
#include <evcode.h> // EVR event codes (IMediaEventSink)

#include "linklist.h"

#define LOWORD(_dw)     ((WORD)(((DWORD_PTR)(_dw)) & 0xffff))
#define HIWORD(_dw)     ((WORD)((((DWORD_PTR)(_dw)) >> 16) & 0xffff))
#define LODWORD(_qw)    ((DWORD)(_qw))
#define HIDWORD(_qw)    ((DWORD)(((_qw) >> 32) & 0xffffffff))


template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

typedef ComPtrList<IMFSample>           VideoSampleList;

// Custom Attributes

// MFSamplePresenter_SampleCounter
// Data type: UINT32
//
// Version number for the video samples. When the presenter increments the version
// number, all samples with the previous version number are stale and should be
// discarded.
static const GUID MFSamplePresenter_SampleCounter =
{ 0xb0bb83cc, 0xf10f, 0x4e2e, { 0xaa, 0x2b, 0x29, 0xea, 0x5e, 0x92, 0xef, 0x85 } };

// {861A1AA8-4DA9-482a-B262-B38EDD6C67E8}
static const GUID MFSampleUsageFlag = 
{ 0x861a1aa8, 0x4da9, 0x482a, { 0xb2, 0x62, 0xb3, 0x8e, 0xdd, 0x6c, 0x67, 0xe8 } };


// MFSamplePresenter_SampleSwapChain
// Data type: IUNKNOWN
//
// Pointer to a Direct3D swap chain.
static const GUID MFSamplePresenter_OffScreen =
{ 0xad885bd1, 0x7def, 0x414a, { 0xb5, 0xb0, 0xd3, 0xd2, 0x63, 0xd6, 0xe9, 0x6d } };


#define SAMPLE_USEDBY_MIXER     1
#define SAMPLE_USEDBY_PRESENTER 2
#define SAMPLE_USEDBY_ALL       (SAMPLE_USEDBY_MIXER | SAMPLE_USEDBY_PRESENTER)


void DllAddRef();
void DllRelease();

// Project headers.
#include "EVRHelpers.h"
#include "EVRScheduler.h"
#include "EVRPresentEngine.h"
#include "EVRPresenter.h"

// CopyComPointer
// Assigns a COM pointer to another COM pointer.
template <class T>
void CopyComPointer(T* &dest, T *src)
{
    if (dest)
    {
        dest->Release();
    }
    dest = src;
    if (dest)
    {
        dest->AddRef();
    }
}

// dxva.dll
typedef HRESULT (__stdcall *PTR_DXVA2CreateDirect3DDeviceManager9)(UINT* pResetToken, IDirect3DDeviceManager9** ppDeviceManager);

// mf.dll
typedef HRESULT (__stdcall *PTR_MFCreatePresentationClock)(IMFPresentationClock** ppPresentationClock);

// evr.dll
typedef HRESULT (__stdcall *PTR_MFCreateDXSurfaceBuffer)(REFIID riid, IUnknown* punkSurface, BOOL fBottomUpWhenLinear, IMFMediaBuffer** ppBuffer);
typedef HRESULT (__stdcall *PTR_MFCreateVideoSampleFromSurface)(IUnknown* pUnkSurface, IMFSample** ppSample);
typedef HRESULT (__stdcall *PTR_MFCreateVideoMediaType)(const MFVIDEOFORMAT* pVideoFormat, IMFVideoMediaType** ppIVideoMediaType);
typedef HRESULT (__stdcall *PTR_MFGetService)(IUnknown *punkObject,REFGUID guidService,REFIID riid, LPVOID *ppvObject);
typedef HRESULT (__stdcall *PTR_MFFrameRateToAverageTimePerFrame)(UINT32 unNumerator,UINT32 unDenominator,UINT64 *punAverageTimePerFrame);
typedef HRESULT (__stdcall *PTR_MFCreateMediaType)(IMFMediaType **ppMFType);


extern PTR_DXVA2CreateDirect3DDeviceManager9	g_pfDXVA2CreateDirect3DDeviceManager9;
extern PTR_MFCreateDXSurfaceBuffer				g_pfMFCreateDXSurfaceBuffer;
extern PTR_MFCreateVideoSampleFromSurface		g_pfMFCreateVideoSampleFromSurface;
extern PTR_MFCreateVideoMediaType				g_pfMFCreateVideoMediaType;
extern PTR_MFGetService				            g_pfMFGetService;
extern PTR_MFFrameRateToAverageTimePerFrame     g_pfMFFrameRateToAverageTimePerFrame;
extern PTR_MFCreateMediaType                    g_pfMFCreateMediaType;

