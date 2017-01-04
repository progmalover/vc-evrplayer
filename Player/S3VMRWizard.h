#pragma once

class S3RenderEngine;
class S3RenderMixer;

#pragma warning(push, 2)

// C4995'function': name was marked as #pragma deprecated
//
// The version of list which shipped with Visual Studio .NET 2003 
// indirectly uses some deprecated functions.  Warning C4995 is disabled 
// because the file cannot be changed and we do not want to 
// display warnings which the user cannot fix.
#pragma warning(disable : 4995)
#include "S3DShowWizard.h"

#include <list>
using namespace std;
#pragma warning(pop)

#define TRACE_VIDEO_PRESENT 0
#if TRACE_VIDEO_PRESENT
struct PresentTimeInfo{
    DWORD BTime;
    DWORD ETime;
    DWORD ID;
};
#endif

class S3VMRWizard:
    public CUnknown,
    public S3DShowWizard,
    public IVMRSurfaceAllocator9,
    public IVMRImagePresenter9
{
public:
    S3VMRWizard(void);
    virtual ~S3VMRWizard(void);

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);

    // S3VMRWizard implementation
    virtual HRESULT Attach(IBaseFilter* pVMRFilter, DWORD_PTR* pdwID);

    virtual HRESULT Detach(DWORD_PTR dwID);

	virtual HRESULT EndDeviceLoss(IDirect3DDevice9* pDevice);

    virtual HRESULT EnableSFRUpload(DWORD_PTR dwID, BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree);

    // IVMRSurfaceAllocator9 implementation
    STDMETHOD(AdviseNotify)(IVMRSurfaceAllocatorNotify9*  lpIVMRSurfAllocNotify);

    STDMETHOD(GetSurface)(
        DWORD_PTR  dwUserID,
        DWORD  SurfaceIndex,
        DWORD  SurfaceFlags,
        IDirect3DSurface9**  lplpSurface);

    STDMETHOD(InitializeDevice)(
        DWORD_PTR  dwUserID,
        VMR9AllocationInfo*  lpAllocInfo,
        DWORD*  lpNumBuffers);

    STDMETHOD(TerminateDevice)(DWORD_PTR  dwID);

    // IVMRImagePresenter9 implementation
    STDMETHOD(StartPresenting)(DWORD_PTR  dwUserID);

    STDMETHOD(StopPresenting)(DWORD_PTR  dwUserID);

    STDMETHOD(PresentImage)(DWORD_PTR  dwUserID, VMR9PresentationInfo*  lpPresInfo);


};
