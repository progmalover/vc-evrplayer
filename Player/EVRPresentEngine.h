//////////////////////////////////////////////////////////////////////////
//
// PresentEngine.h: Defines the D3DPresentEngine object.
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

//-----------------------------------------------------------------------------
// D3DPresentEngine class
//
// This class creates the Direct3D device, allocates Direct3D surfaces for
// rendering, and presents the surfaces. This class also owns the Direct3D
// device manager and provides the IDirect3DDeviceManager9 interface via
// GetService.
//
// The goal of this class is to isolate the EVRCustomPresenter class from
// the details of Direct3D as much as possible.
//-----------------------------------------------------------------------------

DECLARE_INTERFACE_(IEVRSamplePresenter, IUnknown)
{
public:
    virtual HRESULT PresentSample(IMFSample* pSample, DWORD_PTR UserID);
    virtual HRESULT CreateVideoSamples(IMFMediaType *pFormat, VideoSampleList& videoSampleQueue, DWORD_PTR UserID);
    virtual void    ReleaseResources(DWORD_PTR UserID);
  
};


class D3DPresentEngine : public SchedulerCallback
{
public:

    // State of the Direct3D device.
    enum DeviceState
    {
        DeviceOK,
        DeviceReset,    // The device was reset OR re-created.
        DeviceRemoved,  // The device was removed.
    };

    D3DPresentEngine(HRESULT& hre, IEVRSamplePresenter *pSamplePresenter, DWORD_PTR ID);
    virtual ~D3DPresentEngine();

    HRESULT CreateVideoSamples(IMFMediaType *pFormat, VideoSampleList& videoSampleQueue);
    void    ReleaseResources();

    HRESULT PresentSample(IMFSample* pSample, LONGLONG llTarget);
protected:

    CRITICAL_SECTION            m_ObjectLock;           // Thread lock for the D3D device.
    // COM interfaces
    IEVRSamplePresenter         *m_pSamplePresenter;
    DWORD_PTR                    m_dwUserID;
};