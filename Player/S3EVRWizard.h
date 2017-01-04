#pragma once
#include "s3dshowwizard.h"
#include "EVRPresenterInclude.h"

class S3EVRWizard :
    public CUnknown,
    public S3DShowWizard,
    public IEVRSamplePresenter
{
public:
    S3EVRWizard(void);
    virtual ~S3EVRWizard(void);

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

    // S3VMRWizard implementation
    virtual HRESULT Attach(IBaseFilter* pVMRFilter, DWORD_PTR* pdwID);

    virtual HRESULT Detach(DWORD_PTR dwID);

	virtual HRESULT EndDeviceLoss(IDirect3DDevice9* pDevice);

    virtual HRESULT PresentSample(IMFSample* pSample, DWORD_PTR UserID);

    virtual HRESULT CreateVideoSamples(IMFMediaType *pFormat, VideoSampleList& videoSampleQueue, DWORD_PTR UserID);
   
    virtual void    ReleaseResources(DWORD_PTR UserID);


};
