//////////////////////////////////////////////////////////////////////////
//
// PresentEngine.cpp: Defines the D3DPresentEngine object.
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
#include "stdafx.h"
#include "EVRPresenterInclude.h"



//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

D3DPresentEngine::D3DPresentEngine(HRESULT& hr,  IEVRSamplePresenter *pSamplePresenter, DWORD_PTR ID) :
    m_pSamplePresenter(pSamplePresenter),
    m_dwUserID(ID)
{
    InitializeCriticalSection(&m_ObjectLock);

    LeaveCriticalSection(&m_ObjectLock);
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------

D3DPresentEngine::~D3DPresentEngine()
{
    DeleteCriticalSection(&m_ObjectLock);
}




//-----------------------------------------------------------------------------
// CreateVideoSamples
//
// Creates video samples based on a specified media type.
//
// pFormat: Media type that describes the video format.
// videoSampleQueue: List that will contain the video samples.
//
// Note: For each video sample, the method creates a swap chain with a
// single back buffer. The video sample object holds a pointer to the swap
// chain's back buffer surface. The mixer renders to this surface, and the
// D3DPresentEngine renders the video frame by presenting the swap chain.
//-----------------------------------------------------------------------------

HRESULT D3DPresentEngine::CreateVideoSamples(
    IMFMediaType *pFormat,
    VideoSampleList& videoSampleQueue
    )
{
    if (pFormat == NULL)
    {
        return MF_E_UNEXPECTED;
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_ObjectLock);

    ReleaseResources();

    
    if(m_pSamplePresenter)
    {
        hr = m_pSamplePresenter->CreateVideoSamples(pFormat,videoSampleQueue, m_dwUserID);
    }

    if (FAILED(hr))
    {
        ReleaseResources();
    }

    LeaveCriticalSection(&m_ObjectLock);
    return hr;
}




//-----------------------------------------------------------------------------
// ReleaseResources
//
// Released Direct3D resources used by this object.
//-----------------------------------------------------------------------------

void D3DPresentEngine::ReleaseResources()
{
    if(m_pSamplePresenter)
    {
        m_pSamplePresenter->ReleaseResources(m_dwUserID);
    }
}


//-----------------------------------------------------------------------------
// PresentSample
//
// Presents a video frame.
//
// pSample:  Pointer to the sample that contains the surface to present. If
//           this parameter is NULL, the method paints a black rectangle.
// llTarget: Target presentation time.
//
// This method is called by the scheduler and/or the presenter.
//-----------------------------------------------------------------------------

HRESULT D3DPresentEngine::PresentSample(IMFSample* pSample, LONGLONG llTarget)
{
    if(m_pSamplePresenter)
    {
        m_pSamplePresenter->PresentSample(pSample, m_dwUserID);
    }

    return S_OK;
}





// Extracts the FOURCC code from the subtype.
// Not all subtypes follow this pattern.
HRESULT GetFourCC(IMFMediaType *pType, DWORD *pFourCC)
{
    if (pFourCC == NULL) { return E_POINTER; }

    HRESULT hr = S_OK;
    GUID guidSubType = GUID_NULL;

    if (SUCCEEDED(hr))
    {
        hr = pType->GetGUID(MF_MT_SUBTYPE, &guidSubType);
    }

    if (SUCCEEDED(hr))
    {
        *pFourCC = guidSubType.Data1;
    }
    return hr;
}