#include "stdafx.h"
#include "S3RenderEngine.h"
#include "CaptureRenderer.h"
#include "MAMMD3D9.h"

_inline void YUV2RGB(unsigned char* pRGB, int Y, int U, int V)
{
    int result;
    Y -= 16;
    Y *= 298;
    Y += 128;
    U -= 128;
    V -= 128;
    result = ((Y + 516*U) >> 8);
    *pRGB++ = (result<0)?0:((result>255)?255:(unsigned char)result);
    result = ((Y - 100*U - 208*V) >> 8);
    *pRGB++ = (result<0)?0:((result>255)?255:(unsigned char)result);
    result = ((Y + 409*V) >> 8);
    *pRGB++ = (result<0)?0:((result>255)?255:(unsigned char)result);
 
}

static int GetFormatBpp(D3DFORMAT Format)
{
    switch(Format)
    {
    case D3DFMT_R8G8B8: return 24;
        break;
    case D3DFMT_A8R8G8B8: return 32;
        break;
    case D3DFMT_X8R8G8B8: return 32;
        break;

    case D3DFMT_R5G6B5: return 16;
        break;
    case D3DFMT_X1R5G5B5: return 16;
        break;
    case D3DFMT_A1R5G5B5: return 16;
        break;
    case D3DFMT_A4R4G4B4: return 16;
        break;
    case D3DFMT_R3G3B2: return 8;
        break; 
    case D3DFMT_A8:     return 8;
        break;
    case D3DFMT_A8R3G3B2: return 16;
        break;
    case D3DFMT_X4R4G4B4: return 16;
        break;
    case D3DFMT_A2B10G10R10: return 32;
        break;
    case D3DFMT_A8B8G8R8: return 32;
        break;
    case D3DFMT_X8B8G8R8: return 32;
        break;
    case D3DFMT_G16R16: return 32;
        break;
    case D3DFMT_A2R10G10B10: return 32;
        break;
    case D3DFMT_A16B16G16R16: return 64;
        break;
    case D3DFMT_A8P8: return 16;
        break;
    case D3DFMT_P8: return 8;
        break;
    case D3DFMT_L8: return 8;
        break;
    case D3DFMT_A8L8: return 16;
        break;
    case D3DFMT_A4L4: return 8;
        break;
    case D3DFMT_V8U8: return 16;
        break;
    case D3DFMT_L6V5U5: return 16;
        break;
    case D3DFMT_X8L8V8U8: return 32;
        break;
    case D3DFMT_Q8W8V8U8: return 32;
        break;
    case D3DFMT_V16U16: return 32;
        break;
    case D3DFMT_A2W10V10U10: return 32;
        break;
    case D3DFMT_UYVY: return 16;
        break;
    case D3DFMT_R8G8_B8G8: return 16;
        break;
    case D3DFMT_YUY2: return 16;
        break;
    case D3DFMT_G8R8_G8B8: return 16;
        break;
    case D3DFMT_DXT1: return 8;
        break;
    case D3DFMT_DXT2: return 8;
        break;
    case D3DFMT_DXT3: return 16;
        break;
    case D3DFMT_DXT4: return 16;
        break;
    case D3DFMT_DXT5: return 16;
        break;
    case D3DFMT_D16_LOCKABLE: return 16;
        break;
    case D3DFMT_D32: return 32;
        break;
    case D3DFMT_D15S1: return 16;
        break;
    case D3DFMT_D24S8: return 32;
        break;
    case D3DFMT_D24X8: return 32;
        break;
    case D3DFMT_D24X4S4: return 32;
        break;
    case D3DFMT_D16: return 16;
        break;
    case D3DFMT_D32F_LOCKABLE: return 32;
        break;
    case D3DFMT_D24FS8: return 32;
        break;
    case D3DFMT_D32_LOCKABLE: return 32;
        break;
    case D3DFMT_S8_LOCKABLE: return 8;
        break;
    case D3DFMT_L16: return 16;
        break;
    case D3DFMT_VERTEXDATA: return 8;
        break;
    case D3DFMT_INDEX16: return 16;
        break;
    case D3DFMT_INDEX32: return 32;
        break;
    case D3DFMT_Q16W16V16U16: return 64;
        break;
    case D3DFMT_MULTI2_ARGB8: return 32;
        break;
    case D3DFMT_R16F: return 16;
        break;
    case D3DFMT_G16R16F: return 32;
        break;
    case D3DFMT_A16B16G16R16F: return 64;
        break;
    case D3DFMT_R32F: return 32;
        break;
    case D3DFMT_G32R32F: return 64;
        break;
    case D3DFMT_A32B32G32R32F: return 128;
        break;
    case D3DFMT_CxV8U8: return 16;
        break;
    case D3DFMT_A1: return 1;
        break;
    //case D3DFMT_A2B10G10R10_XR_BIAS: return 32;
        //break;
    case D3DFMT_BINARYBUFFER: return 8;
        break;
    case D3DFMT_NV12: return 16;
        break;
    }
    return 32;
}


// {09577CB9-6C5B-4451-A541-BF86513FA533}
static const IID IID_CaptureMediaSample = 
{ 0x9577cb9, 0x6c5b, 0x4451, { 0xa5, 0x41, 0xbf, 0x86, 0x51, 0x3f, 0xa5, 0x33 } };


CaptureMediaSample::CaptureMediaSample(TCHAR *pName,
               CaptureAllocator *pAllocator,
               HRESULT *phr,
               LPBYTE pBuffer,
               LONG length,
               CaptureRenderer *pRender) :
CMediaSample(pName, pAllocator, phr, pBuffer, length),
m_pCaptureRender(pRender),
m_pVideoSurface(NULL)
{

    /* We must have an owner and it must also be derived from class
       CBaseAllocator BUT we do not hold a reference count on it */

    ASSERT(pAllocator);

    if(m_pCaptureRender->m_CaptureToVideo)
    {
        HRESULT hr;
        hr = m_pCaptureRender->m_pDevice->CreateRenderTarget(m_pCaptureRender->m_lVidWidth, m_pCaptureRender->m_InternalHeight, 
            m_pCaptureRender->m_SurfaceFormat,D3DMULTISAMPLE_NONE,0, TRUE,&m_pVideoSurface, NULL); 

        D3DLOCKED_RECT myLockedRect;

        m_pVideoSurface->LockRect(&myLockedRect, NULL, 0);

        m_pBuffer = (LPBYTE)myLockedRect.pBits;
    }

    else if(m_pCaptureRender->m_pRenderEngine->IsMAMMEnabled())
    {
        if(m_pCaptureRender->m_InternalWidth == m_pCaptureRender->m_lVidWidth)
        {
            if(m_pCaptureRender->m_bHDCPEnable)
            {
                HRESULT hr;
                MAMM3DDevice9 *pMAMMDevice = (MAMM3DDevice9*)m_pCaptureRender->m_pDevice;

                hr = pMAMMDevice->CreateProtectedOffscreenPlainSurface(m_pCaptureRender->m_lVidWidth,
                    m_pCaptureRender->m_InternalHeight,
                    m_pCaptureRender->m_SurfaceFormat,
                    D3DPOOL_SYSTEMMEM,
                    (MAMM3DSurface9**)&m_pVideoSurface, 
                    (HANDLE *)&pBuffer);
            }else
            {
                HRESULT hr;
                hr = m_pCaptureRender->m_pDevice->CreateOffscreenPlainSurface(m_pCaptureRender->m_lVidWidth,
                    m_pCaptureRender->m_InternalHeight,
                    m_pCaptureRender->m_SurfaceFormat,
                    D3DPOOL_SYSTEMMEM,
                    &m_pVideoSurface, 
                    (HANDLE *)&pBuffer);
            }
        }

    }
}

CaptureMediaSample::~CaptureMediaSample()
{
    if(m_pCaptureRender->m_CaptureToVideo)
    {
        m_pVideoSurface->UnlockRect();
    }
    if(m_pVideoSurface){m_pVideoSurface->Release(); m_pVideoSurface = NULL; };


    if(!m_pCaptureRender->m_CaptureToVideo)
    {
        EXECUTE_ASSERT(VirtualFree(m_pBuffer, 0, MEM_RELEASE));
    }
    m_pBuffer = NULL;
}

/* Override this to publicise our interfaces */

STDMETHODIMP
CaptureMediaSample::QueryInterface(REFIID riid, void **ppv)
{
    if (riid == IID_IMediaSample ||
        riid == IID_IMediaSample2 ||
        riid == IID_CaptureMediaSample ||
        riid == IID_IUnknown) {
        return GetInterface((CaptureMediaSample *) this, ppv);
    } else {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP
CaptureMediaSample::GetVideoSurface(LPDIRECT3DSURFACE9 *ppSurface)
{
    *ppSurface = m_pVideoSurface;
    return S_OK;
}

STDMETHODIMP CaptureMediaSample::SyncVideoSurface()
{
    if(m_pVideoSurface && !m_pCaptureRender->m_CaptureToVideo)
    {
        D3DLOCKED_RECT  myLockedRect;
        if(FAILED(m_pVideoSurface->LockRect(&myLockedRect, NULL, 0)))
        {
            ::DbgMsg(_T("CaptureRenderer::SyncVideoSurface: failed in lock()"));

            return E_FAIL;
        }
        m_pVideoSurface->UnlockRect();
    }
    if(m_pVideoSurface && m_pCaptureRender->m_CaptureToVideo)
    {
        D3DLOCKED_RECT myLockedRect;

        m_pVideoSurface->LockRect(&myLockedRect, NULL, 0);

        m_pBuffer = (LPBYTE)myLockedRect.pBits;
    }


    return S_OK;
}

CaptureAllocator::CaptureAllocator(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr,
    CaptureRenderer *pRenderer)
    : CBaseAllocator(pName, pUnk, phr, TRUE, TRUE), m_pCaptureRender(pRenderer)
{
}




/* This sets the size and count of the required samples. The memory isn't
   actually allocated until Commit() is called, if memory has already been
   allocated then assuming no samples are outstanding the user may call us
   to change the buffering, the memory will be released in Commit() */
STDMETHODIMP
CaptureAllocator::SetProperties(
                ALLOCATOR_PROPERTIES* pRequest,
                ALLOCATOR_PROPERTIES* pActual)
{
    CheckPointer(pActual,E_POINTER);
    ValidateReadWritePtr(pActual,sizeof(ALLOCATOR_PROPERTIES));
    CAutoLock cObjectLock(this);

    ZeroMemory(pActual, sizeof(ALLOCATOR_PROPERTIES));

    ASSERT(pRequest->cbBuffer > 0);

    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);


    /*  Check the alignment request is a power of 2 */
    if ((-pRequest->cbAlign & pRequest->cbAlign) != pRequest->cbAlign) {
        DbgLog((LOG_ERROR, 1, TEXT("Alignment requested 0x%x not a power of 2!"),
               pRequest->cbAlign));
    }
    /*  Check the alignment requested */
    if (pRequest->cbAlign == 0 ||
    (SysInfo.dwAllocationGranularity & (pRequest->cbAlign - 1)) != 0) {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid alignment 0x%x requested - granularity = 0x%x"),
               pRequest->cbAlign, SysInfo.dwAllocationGranularity));
        return VFW_E_BADALIGN;
    }

    /* Can't do this if already committed, there is an argument that says we
       should not reject the SetProperties call if there are buffers still
       active. However this is called by the source filter, which is the same
       person who is holding the samples. Therefore it is not unreasonable
       for them to free all their samples before changing the requirements */

    if (m_bCommitted == TRUE) {
        return VFW_E_ALREADY_COMMITTED;
    }

    /* Must be no outstanding buffers */

    if (m_lFree.GetCount() < m_lAllocated) {
        return VFW_E_BUFFERS_OUTSTANDING;
    }

    /* There isn't any real need to check the parameters as they
       will just be rejected when the user finally calls Commit */

    // round length up to alignment - remember that prefix is included in
    // the alignment
    LONG lSize = pRequest->cbBuffer + pRequest->cbPrefix;
    LONG lRemainder = lSize % pRequest->cbAlign;
    if (lRemainder != 0) {
        lSize = lSize - lRemainder + pRequest->cbAlign;
    }
    pActual->cbBuffer = m_lSize = (lSize - pRequest->cbPrefix);

    pActual->cBuffers = m_lCount = min(pRequest->cBuffers, 4);
    pActual->cbAlign = m_lAlignment = pRequest->cbAlign;
    pActual->cbPrefix = m_lPrefix = pRequest->cbPrefix;

    m_bChanged = TRUE;
    return NOERROR;
}


HRESULT CaptureAllocator::GetBuffer(IMediaSample **ppBuffer,
                                  REFERENCE_TIME *pStartTime,
                                  REFERENCE_TIME *pEndTime,
                                  DWORD dwFlags
                                  )
{
    UNREFERENCED_PARAMETER(pStartTime);
    UNREFERENCED_PARAMETER(pEndTime);
    UNREFERENCED_PARAMETER(dwFlags);
    CaptureMediaSample *pSample;

    *ppBuffer = NULL;
    for (;;)
    {
        {  // scope for lock
            CAutoLock cObjectLock(this);

            /* Check we are committed */
            if (!m_bCommitted) {
                return VFW_E_NOT_COMMITTED;
            }
            pSample = (CaptureMediaSample *) m_lFree.RemoveHead();
            if (pSample == NULL) {
                SetWaiting();
            }
        }

        /* If we didn't get a sample then wait for the list to signal */

        if (pSample) {
            break;
        }
        if (dwFlags & AM_GBF_NOWAIT) {
            return VFW_E_TIMEOUT;
        }
        ASSERT(m_hSem != NULL);
        WaitForSingleObject(m_hSem, INFINITE);
    }

    /* Addref the buffer up to one. On release
       back to zero instead of being deleted, it will requeue itself by
       calling the ReleaseBuffer member function. NOTE the owner of a
       media sample must always be derived from CBaseAllocator */


    ASSERT(pSample->m_cRef == 0);
    pSample->m_cRef = 1;
    *ppBuffer = pSample;


    return NOERROR;
}


// override this to allocate our resources when Commit is called.
//
// note that our resources may be already allocated when this is called,
// since we don't free them on Decommit. We will only be called when in
// decommit state with all buffers free.
//
// object locked by caller
HRESULT
CaptureAllocator::Alloc(void)
{
    CAutoLock lck(this);

    /* Check he has called SetProperties */
    HRESULT hr = CBaseAllocator::Alloc();
    if (FAILED(hr)) {
        return hr;
    }

    /* If the requirements haven't changed then don't reallocate */
    if (hr == S_FALSE) {
        return NOERROR;
    }
    ASSERT(hr == S_OK); // we use this fact in the loop below


    ReallyFree();


    /* Compute the aligned size */
    DWORD Width = m_pCaptureRender->m_lVidWidth;
    DWORD Height = m_pCaptureRender->m_InternalHeight;
    DWORD BitCount = GetFormatBpp(m_pCaptureRender->m_SurfaceFormat);

    DWORD RealSize = Width * Height * BitCount / 8;
    if(m_pCaptureRender->m_bHDCPEnable)
    {
        RealSize += 4096;
    }
    LONG lAlignedSize = RealSize + m_lPrefix;
    if (m_lAlignment > 1) {
        LONG lRemainder = lAlignedSize % m_lAlignment;
        if (lRemainder != 0) {
            lAlignedSize += (m_lAlignment - lRemainder);
        }
    }

    /* Create the contiguous memory block for the samples
       making sure it's properly aligned (64K should be enough!)
    */
    ASSERT(lAlignedSize % m_lAlignment == 0);


    CaptureMediaSample *pSample;

    ASSERT(m_lAllocated == 0);



    // Create the new samples - we have allocated m_lSize bytes for each sample
    // plus m_lPrefix bytes per sample as a prefix. We set the pointer to
    // the memory after the prefix - so that GetPointer() will return a pointer
    // to m_lSize bytes.
    for (; m_lAllocated < m_lCount; m_lAllocated++) {
        LPBYTE pBuffer = NULL;

        if(m_pCaptureRender->m_CaptureToVideo)
        {
        }else
        {

            if(m_pCaptureRender->m_PreferedNumaNode >= 0)
            {
                 pBuffer = (PBYTE)VirtualAllocExNuma(
                            GetCurrentProcess(),
                            NULL,
                            lAlignedSize,
                            MEM_RESERVE | MEM_COMMIT,
                            PAGE_READWRITE,
                            m_pCaptureRender->m_PreferedNumaNode
                        );
            }else
            {
                pBuffer = (PBYTE)VirtualAlloc(NULL,
                        lAlignedSize,
                        MEM_COMMIT,
                        PAGE_READWRITE);
            }

            if (pBuffer == NULL) {
                return E_OUTOFMEMORY;
            }
        }

        pSample = new CaptureMediaSample(
                            NAME("Capture media sample"),
                            this,
                            &hr,
                            pBuffer,      // GetPointer() value
                            m_lSize,
                            m_pCaptureRender);               // not including prefix

            ASSERT(SUCCEEDED(hr));
        if (pSample == NULL) {
            return E_OUTOFMEMORY;
        }

        // This CANNOT fail
        m_lFree.Add(pSample);
    }

    m_bChanged = FALSE;
    return NOERROR;
}


// override this to free up any resources we have allocated.
// called from the base class on Decommit when all buffers have been
// returned to the free list.
//
// caller has already locked the object.

// in our case, we keep the memory until we are deleted, so
// we do nothing here. The memory is deleted in the destructor by
// calling ReallyFree()
void
CaptureAllocator::Free(void)
{
    return;
}


// called from the destructor (and from Alloc if changing size/count) to
// actually free up the memory
void
CaptureAllocator::ReallyFree(void)
{
    /* Should never be deleting this unless all buffers are freed */

    ASSERT(m_lAllocated == m_lFree.GetCount());

    /* Free up all the CMediaSamples */

    CaptureMediaSample *pSample;
    for (;;) {
        pSample = (CaptureMediaSample *)m_lFree.RemoveHead();
        if (pSample != NULL) {
            delete pSample;
        } else {
            break;
        }
    }

    m_lAllocated = 0;


}


/* Destructor frees our memory resources */

CaptureAllocator::~CaptureAllocator()
{
    Decommit();
    ReallyFree();
}




CaptureInputPin::CaptureInputPin(CaptureRenderer *pRenderer,
                                     HRESULT *phr,
                                     LPCWSTR pPinName) :
    CRendererInputPin(pRenderer,phr, pPinName), m_pCaptureRender(pRenderer)
{

}

STDMETHODIMP
CaptureInputPin::GetAllocator(
    IMemAllocator **ppAllocator)
{
    if(!(m_pCaptureRender->m_CaptureToPCIe || m_pCaptureRender->m_CaptureToVideo))
        return VFW_E_NO_ALLOCATOR;

    CheckPointer(ppAllocator,E_POINTER);
    ValidateReadWritePtr(ppAllocator,sizeof(IMemAllocator *));
    CAutoLock cObjectLock(m_pLock);

    if (m_pAllocator == NULL) {
        HRESULT hr = S_OK;
            m_pAllocator = new CaptureAllocator(L"CaptureAllocator", NULL, &hr, m_pCaptureRender); 
            m_pAllocator->AddRef();
            //HRESULT hr = CreateMemoryAllocator(&m_pAllocator);
        if (FAILED(hr)) {
            return hr;
        }
    }
    ASSERT(m_pAllocator != NULL);
    *ppAllocator = m_pAllocator;
    m_pAllocator->AddRef();
    return NOERROR;
}

STDMETHODIMP
CaptureInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps)
{
    if(pProps == NULL)
        return E_POINTER;

    memset(pProps, 0,sizeof(ALLOCATOR_PROPERTIES));
    //pProps->cbAlign = 0x4000;

    DWORD Width = ((CaptureRenderer *)m_pRenderer)->m_lVidWidth;
    DWORD Height = ((CaptureRenderer *)m_pRenderer)->m_lVidHeight;
    DWORD BitCount = GetFormatBpp(((CaptureRenderer *)m_pRenderer)->m_SurfaceFormat);

    pProps->cbBuffer = Width * Height * BitCount / 8;

    if(((CaptureRenderer *)m_pRenderer)->m_bHDCPEnable)
    {
        pProps->cbBuffer += 4096;
    }
    return S_OK;
}


CaptureRenderer::CaptureRenderer(HRESULT *phr)
                        : CBaseVideoRenderer(__uuidof(CLSID_S3CaptureRenderer), 
                        NAME("S3 Capture Renderer"), NULL, phr)
{
    m_pVideoSurface = NULL;
    m_pSurfaceTemp = NULL;
    m_pTexture = NULL;
    m_pDevice = NULL;
    *phr = S_OK;
    m_bEnableSFRUpload = FALSE;
    m_bEnableSplitUpload = FALSE;
	memset(&m_SFRRect, 0, sizeof(m_SFRRect));
	m_bForceMediaType = FALSE;
    m_lastFrameTime = 0;
    m_PreferedNumaNode = -1;
    m_CaptureToPCIe = FALSE;
    m_CaptureToVideo = FALSE;
    m_bHDCPEnable = FALSE;

}


//-----------------------------------------------------------------------------
// CTextureRenderer destructor
//-----------------------------------------------------------------------------
CaptureRenderer::~CaptureRenderer()
{
    if(m_pVideoSurface) m_pVideoSurface->Release();
    if(m_pSurfaceTemp) m_pSurfaceTemp->Release();
    if(m_pTexture) m_pTexture->Release();
}


//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CaptureRenderer::CheckMediaType(const CMediaType *pmt)
{
    VIDEOINFO *pvi=0;
    if(!pmt)
    {
        return E_INVALIDARG;
    }

    if( *pmt->FormatType() != FORMAT_VideoInfo ) 
    {
        return E_INVALIDARG;
    }
        
    // Only accept RGB24
    pvi = (VIDEOINFO *)pmt->Format();

    if( IsEqualGUID( *pmt->Type(), MEDIATYPE_Video) )
    {
        if(!IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB32) && \
            !IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24) && \
			!IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_YUY2) && \
			!IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_UYVY))
        {
            return DDERR_INVALIDPIXELFORMAT;
        }
    }

	if(m_bForceMediaType && !IsEqualGUID( *pmt->Subtype(), m_ForceMediaType))
	{
		return DDERR_INVALIDPIXELFORMAT;
	}

    return S_OK;
}

HRESULT CaptureRenderer::ForceMediaType(GUID MediaType)
{
	m_bForceMediaType = TRUE;
	m_ForceMediaType = MediaType;
	return S_OK;
}


//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made. 
//-----------------------------------------------------------------------------
HRESULT CaptureRenderer::SetMediaType(const CMediaType *pmt)
{
    VIDEOINFO *pviBmp = NULL;   // Bitmap info header
    if(!pmt)
    {
        return E_INVALIDARG;
    }

    // Retreive the size of this media type
    pviBmp = (VIDEOINFO *)pmt->Format();

    m_lVidWidth  = pviBmp->bmiHeader.biWidth;
    m_lVidHeight = abs(pviBmp->bmiHeader.biHeight);

    m_videoSubtype = pmt->subtype;

    if(IsEqualGUID(pmt->subtype, MEDIASUBTYPE_RGB24))
    {
        m_lVidPitch  = (m_lVidWidth * 3 + 3) & ~(3); 
        m_SurfaceFormat = D3DFMT_X8R8G8B8;
    }
    if(IsEqualGUID(pmt->subtype, MEDIASUBTYPE_RGB32))
    {
        m_lVidPitch  = (m_lVidWidth * 4 + 4) & ~(4); 
        m_SurfaceFormat = D3DFMT_X8R8G8B8;
    }
    else if(IsEqualGUID(pmt->subtype, MEDIASUBTYPE_YUY2) || IsEqualGUID(pmt->subtype, MEDIASUBTYPE_UYVY))
    {
        m_lVidPitch  = m_lVidWidth * 2; 
#if USE_NV12
		m_SurfaceFormat = (D3DFORMAT)D3DFMT_NV12;
#else 
		m_SurfaceFormat = D3DFMT_YUY2;
#endif
    }
    else
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    D3DFORMAT TextureFormat;

    m_InternalWidth = (m_lVidWidth + 31)/32 *32;
    m_InternalHeight = (m_lVidHeight + 7)/8 *8;

    if(m_pRenderEngine->IsMAMMEnabled())
    {
        //hr = m_pDevice->CreateOffscreenPlainSurface(m_InternalWidth, m_InternalHeight, m_SurfaceFormat, D3DPOOL_SYSTEMMEM, &m_pVideoSurface, NULL);
        //if(FAILED(hr))::DbgMsg(_T("CaptureRenderer::SetMediaType: failed in CreateMedia(), hr = 0x%08x"),hr);

        IDirect3D9 *pDirect3D9 = NULL;
        m_pDevice->GetDirect3D(&pDirect3D9);

        hr = pDirect3D9->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
            D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, m_SurfaceFormat);
        pDirect3D9->Release();

        if(hr == D3D_OK && m_pRenderEngine->IsMAMMEnabled())
        {
            TextureFormat = m_SurfaceFormat;
        }else
        {
            hr = m_pDevice->CreateOffscreenPlainSurface(m_InternalWidth, m_InternalHeight, m_SurfaceFormat, D3DPOOL_DEFAULT, &m_pSurfaceTemp, NULL);
            if(FAILED(hr))::DbgMsg(_T("CaptureRenderer::SetMediaType: failed in CreateMedia(), hr = 0x%08x"),hr);
            TextureFormat = D3DFMT_X8R8G8B8;
        }

    }else
    {
        //hr = m_pDevice->CreateOffscreenPlainSurface(m_InternalWidth, m_InternalHeight, m_SurfaceFormat, D3DPOOL_DEFAULT, &m_pVideoSurface, NULL);
        //if(FAILED(hr))::DbgMsg(_T("CaptureRenderer::SetMediaType: failed in CreateMedia(), hr = 0x%08x"),hr);

        TextureFormat = D3DFMT_X8R8G8B8;
    }

    DWORD Usage = D3DUSAGE_RENDERTARGET;

    if(m_bHDCPEnable && m_pRenderEngine->IsMAMMEnabled())
    {
        HRESULT hr;
        MAMM3DDevice9 *pMAMMDevice = (MAMM3DDevice9*)m_pDevice;

        hr = pMAMMDevice->CreateProtectedTexture(m_InternalWidth, m_InternalHeight, 1, Usage, 
            TextureFormat, D3DPOOL_DEFAULT, (MAMM3DTexture9**)&m_pTexture, NULL);
    }else
    {
        hr = m_pDevice->CreateTexture(m_InternalWidth, m_InternalHeight, 1, Usage, 
            TextureFormat, D3DPOOL_DEFAULT, &m_pTexture, NULL);
    }

    if(FAILED(hr))::DbgMsg(_T("CaptureRenderer::SetMediaType: failed in CreateMedia(), hr = 0x%08x"),hr);

    return S_OK;
}


CBasePin *CaptureRenderer::GetPin(int n)
{
    CAutoLock cObjectCreationLock(&m_ObjectCreationLock);

    // Should only ever be called with zero
    ASSERT(n == 0);

    if (n != 0) {
        return NULL;
    }

    // Create the input pin if not already done so

    if (m_pInputPin == NULL) {

        // hr must be initialized to NOERROR because
        // CRendererInputPin's constructor only changes
        // hr's value if an error occurs.
        HRESULT hr = NOERROR;

        m_pInputPin = new CaptureInputPin(this,&hr,L"In");
        if (NULL == m_pInputPin) {
            return NULL;
        }

        if (FAILED(hr)) {
            delete m_pInputPin;
            m_pInputPin = NULL;
            return NULL;
        }
    }
    return m_pInputPin;
}

STDMETHODIMP CaptureRenderer::Pause()
{
    CAutoLock cRendererLock(&m_InterfaceLock);
    FILTER_STATE OldState = m_State;
    ASSERT(m_pInputPin->IsFlushing() == FALSE);

    // Make sure there really is a state change

    if (m_State == State_Paused) {
        return CompleteStateChange(State_Paused);
    }

    // Has our input pin been connected

    if (m_pInputPin->IsConnected() == FALSE) {
        NOTE("Input pin is not connected");
        m_State = State_Paused;
        return CompleteStateChange(State_Paused);
    }

    // Pause the base filter class

    HRESULT hr = CBaseFilter::Pause();
    if (FAILED(hr)) {
        NOTE("Pause failed");
        return hr;
    }

    // Enable EC_REPAINT events again

    SetRepaintStatus(TRUE);
    //StopStreaming();
    SourceThreadCanWait(TRUE);
    CancelNotification();
    ResetEndOfStreamTimer();

    // If we are going into a paused state then we must commit whatever
    // allocator we are using it so that any source filter can call the
    // GetBuffer and expect to get a buffer without returning an error

    if (m_pInputPin->Allocator()) {
        m_pInputPin->Allocator()->Commit();
    }

    // There should be no outstanding advise
    ASSERT(CancelNotification() == S_FALSE);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(m_EndOfStreamTimer == 0);
    ASSERT(m_pInputPin->IsFlushing() == FALSE);

    // When we come out of a stopped state we must clear any image we were
    // holding onto for frame refreshing. Since renderers see state changes
    // first we can reset ourselves ready to accept the source thread data
    // Paused or running after being stopped causes the current position to
    // be reset so we're not interested in passing end of stream signals

    if (OldState == State_Stopped) {
        m_bAbort = FALSE;
        ClearPendingSample();
    }
    return CompleteStateChange(OldState);
}

#define DUMPTIME 0

#if DUMPTIME
DWORD rendertime[1024][2];
DWORD g_curPos = 0;
#endif
//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CaptureRenderer::DoRenderSample( IMediaSample * pSample )
{

    HRESULT hr;
    if(!pSample)
    {
        return E_INVALIDARG;
    }

    m_lastFrameTime = timeGetTime();


    CaptureMediaSample *pCaptureSample = NULL;
    LPDIRECT3DSURFACE9 pVideoSource = NULL;

    hr = pSample->QueryInterface(IID_CaptureMediaSample, (VOID **)&pCaptureSample);

    // try to get video source from sample
    if(hr == S_OK)
    {
        pCaptureSample->GetVideoSurface(&pVideoSource);
        pCaptureSample->Release();
    }

    if(pVideoSource == NULL)
    {
        BYTE * pSampleBuffer = NULL;
        BYTE*  pDestBuffer;
        // Get the video bitmap buffer
        hr = pSample->GetPointer( &pSampleBuffer );
        if( FAILED(hr))
        {
            return hr;
        }

        if(m_pVideoSurface == NULL)
        {
            HRESULT hr = S_OK;
            if(m_pRenderEngine->IsMAMMEnabled())
            {
                hr = m_pDevice->CreateOffscreenPlainSurface(m_InternalWidth, m_InternalHeight, m_SurfaceFormat, D3DPOOL_SYSTEMMEM, &m_pVideoSurface, NULL);
                if(FAILED(hr))::DbgMsg(_T("CaptureRenderer::SetMediaType: failed in CreateMedia(), hr = 0x%08x"),hr);


            }else
            {
                hr = m_pDevice->CreateOffscreenPlainSurface(m_InternalWidth, m_InternalHeight, m_SurfaceFormat, D3DPOOL_DEFAULT, &m_pVideoSurface, NULL);
                if(FAILED(hr))::DbgMsg(_T("CaptureRenderer::SetMediaType: failed in CreateMedia(), hr = 0x%08x"),hr);

            }
        }

        D3DLOCKED_RECT  myLockedRect;
        if(FAILED(m_pVideoSurface->LockRect(&myLockedRect, NULL, 0)))
        {
            ::DbgMsg(_T("CaptureRenderer::SetMediaType: failed in CreateMedia()"));

            return E_FAIL;
        }

        pDestBuffer = (BYTE*)myLockedRect.pBits;
    #if DUMPTIME
	    if(g_curPos<1024)
	    {
		    LONGLONG sTime = 0, etime = 0;
		    HRESULT hr;
		    rendertime[g_curPos][0] = timeGetTime();
		    hr = pSample->GetTime(&sTime, &etime);
		    rendertime[g_curPos][1] = sTime;
		    g_curPos++;
	    }
	    if(g_curPos == 1024)
	    {
		    CFile myFile;
		    BOOL bOK;
		    bOK = myFile.Open(L"D:\\Temp\\timetable.bin", CFile::modeCreate|CFile::modeWrite);

		    myFile.Write(rendertime, sizeof(DWORD)*1024*2);
		    myFile.Close();
		    g_curPos++;
	    }
    #endif
        if(IsEqualGUID(m_videoSubtype, MEDIASUBTYPE_YUY2))
        {
            
    #if USE_NV12
		    LONG row,col;
		    BYTE* pDstUV = pDestBuffer + myLockedRect.Pitch*m_lVidHeight;
		    BYTE* pSrcBuffer = pSampleBuffer;
		    for(row = 0; row < m_lVidHeight; row+=2)
		    {
			    BYTE* pSrc0 = pSrcBuffer;
			    BYTE* pSrc1 = pSrcBuffer + m_lVidPitch;
			    BYTE* pDst = pDestBuffer;
			    BYTE* pDst2 = pDestBuffer + myLockedRect.Pitch;
			    BYTE* pDstA = pDstUV;
			    unsigned int temp1,temp2;
			    for(col = 0; col < m_lVidWidth; col+=2)
			    {
				    *pDst++ = *pSrc0++;
				    temp1 = *pSrc0++;
				    *pDst++ = *pSrc0++;
				    temp2 = *pSrc0++;

				    *pDst2++ = *pSrc1++;
				    temp1 += *pSrc1++;
				    *pDst2++ = *pSrc1++;
				    temp2 += *pSrc1++;

				    *pDstA++ = (BYTE)(temp1/2);
				    *pDstA++ = (BYTE)(temp2/2);
			    }
			    pSrcBuffer  += 2*m_lVidPitch;
			    pDestBuffer += 2*myLockedRect.Pitch;
			    pDstUV += myLockedRect.Pitch;
		    }
    #else
		    LONG row;
            for(row = 0; row < m_lVidHeight; row++ ) 
            {
                memcpy(pDestBuffer, pSampleBuffer, 2*m_lVidWidth);
                pSampleBuffer  += m_lVidPitch;
                pDestBuffer += myLockedRect.Pitch;
            }
    #endif
        }
	    else if(IsEqualGUID(m_videoSubtype, MEDIASUBTYPE_UYVY))
	    {
    #if USE_NV12
		    BYTE* pDstUV = pDestBuffer + m_lVidWidth*m_lVidHeight;
    #else
		    LONG row, col;

            for(row = 0; row < m_lVidHeight; row++ ) 
            {
                unsigned char *pSrc = pSampleBuffer;
                unsigned char *pDst = pDestBuffer;
                for(col = 0; col < m_lVidWidth; col += 2)
                {
                    pDst[1] = *pSrc++;
                    pDst[0]  = *pSrc++;
                    pDst[3] = *pSrc++;
                    pDst[2]  = *pSrc++;
                    pDst += 4;
                }
                pSampleBuffer  += m_lVidPitch;
                pDestBuffer += myLockedRect.Pitch;
            }
    #endif
	    }
        else if(IsEqualGUID(m_videoSubtype, MEDIASUBTYPE_RGB24))
        {
            LONG row, col;
            LONG copysize = 3*m_lVidWidth;

            for(row = 0; row < m_lVidHeight; row++ ) 
            {
		        BYTE* pTempSRC = pSampleBuffer;
		        BYTE* pTempDST = pDestBuffer;
		        for(col = 0; col < m_lVidWidth; col ++)
		        {
			        *pTempDST++ = *pTempSRC++;
			        *pTempDST++ = *pTempSRC++;
			        *pTempDST++ = *pTempSRC++;
			        *pTempDST++ = 0;
		        }
                pSampleBuffer  += m_lVidPitch;
                pDestBuffer += myLockedRect.Pitch;
            }
        }
        else if(IsEqualGUID(m_videoSubtype, MEDIASUBTYPE_RGB32))
        {
		    LONG row;
            for(row = 0; row < m_lVidHeight; row++ ) 
            {
                memcpy(pDestBuffer, pSampleBuffer, 4*m_lVidWidth);
                pSampleBuffer  += m_lVidPitch;
                pDestBuffer += myLockedRect.Pitch;
            }

        }

        hr = m_pVideoSurface->UnlockRect();
        pVideoSource = m_pVideoSurface;

    }
    


    LPDIRECT3DSURFACE9 pBackBuffer;
        
    CHECK_HR(
        hr = m_pTexture->GetSurfaceLevel( 0, &pBackBuffer),
        ::DbgMsg(_T("S3VMRWizard::PresentImage: failed to get the 0-level surface from the private texture , hr = 0x%08x, pPrivTextureSurf = 0x%08x"), hr, pBackBuffer));

    if(m_CaptureToVideo)
    {
        pVideoSource->UnlockRect();
    }

    if(m_pRenderEngine->IsMAMMEnabled())
    {
        if(m_pSurfaceTemp)
        {
            hr = m_pDevice->UpdateSurface(pVideoSource, NULL, m_pSurfaceTemp, NULL);
            if(FAILED(hr))::DbgMsg(_T("CaptureRenderer::DoRenderSample: failed in render(), hr = 0x%08x"),hr);

            hr = m_pDevice->StretchRect(m_pSurfaceTemp, NULL, pBackBuffer, NULL, D3DTEXF_NONE );
            if(FAILED(hr))::DbgMsg(_T("CaptureRenderer::DoRenderSample: failed in render(), hr = 0x%08x"),hr);
        }else
        {
            MAMM3DSurface9 *pMAMMSurface = (MAMM3DSurface9 *)pBackBuffer;



            RECT DisplayRect = m_SFRRect;


            long DisplayWidth = DisplayRect.right - DisplayRect.left;
            long DisplayHeight = DisplayRect.bottom - DisplayRect.top;



        //if (!ClipInfo.IsRectEmpty())
            {
                DisplayRect.left -= DisplayWidth * m_ClipInfo.left/(m_lVidWidth - m_ClipInfo.left);  
                DisplayRect.right += DisplayWidth * m_ClipInfo.right/(m_lVidWidth - m_ClipInfo.right); 
                DisplayRect.top -= DisplayHeight * m_ClipInfo.top / (m_lVidHeight - m_ClipInfo.top); 
                DisplayRect.bottom += DisplayHeight * m_ClipInfo.bottom / (m_lVidHeight - m_ClipInfo.bottom);
            }

            DisplayWidth = DisplayRect.right - DisplayRect.left;
            DisplayHeight = DisplayRect.bottom - DisplayRect.top;


            if(m_InternalWidth != m_lVidWidth || m_InternalHeight != m_lVidHeight)
            {
                // scale display rect
                DisplayWidth = DisplayWidth * m_InternalWidth/m_lVidWidth;
                DisplayHeight = DisplayHeight * m_InternalHeight/m_lVidHeight;
                                                            
                DisplayRect.right = DisplayRect.left + DisplayWidth;
                DisplayRect.bottom = DisplayRect.top + DisplayHeight;
            }




            pMAMMSurface->EnableSFRUploadFreeAngle(m_bEnableSFRUpload, m_bEnableSplitUpload, &DisplayRect, m_RotateDegree);


            hr = m_pDevice->UpdateSurface(pVideoSource, NULL, pBackBuffer, NULL);
            if(FAILED(hr))::DbgMsg(_T("CaptureRenderer::DoRenderSample: failed in render(), hr = 0x%08x"),hr);
        }
    }else
    {
        hr = m_pDevice->StretchRect(pVideoSource, NULL, pBackBuffer, NULL, D3DTEXF_NONE );
        if(FAILED(hr))::DbgMsg(_T("CaptureRenderer::DoRenderSample: failed in render(), hr = 0x%08x"),hr);

    }

    pBackBuffer->Release();

    if(FAILED(hr))::DbgMsg(_T("CaptureRenderer::DoRenderSample: failed in Present(), hr = 0x%08x"),hr);

    hr = pSample->QueryInterface(IID_CaptureMediaSample, (VOID **)&pCaptureSample);

    // try to get video source from sample
    if(hr == S_OK)
    {
        pCaptureSample->SyncVideoSurface();
        pCaptureSample->Release();
    }

    return S_OK;
}


HRESULT CaptureRenderer::GetTexture(LPDIRECT3DTEXTURE9* ppTexture)
{
    HRESULT hr = E_FAIL;

    if( !ppTexture )
    {
        ::DbgMsg(_T("CaptureRenderer::GetTexture: second argument is NULL"));
        return E_POINTER;
    }

    *ppTexture = m_pTexture;

    return S_OK;
}

HRESULT CaptureRenderer::GetVideoSize(LONG* plWidth, LONG* plHeight, LONG* preferedx, LONG* preferedy)
{
    HRESULT hr = E_FAIL;

    if( !plWidth || !plHeight || !preferedx || !preferedy )
    {
        ::DbgMsg(_T("CaptureRenderer::GetVideoSize: argument is NULL"));
        return E_POINTER;
    }

    *plWidth = m_lVidWidth;
    *plHeight = m_lVidHeight;

    *preferedx = m_lVidWidth;
    *preferedy = m_lVidHeight;

    return S_OK;
}

HRESULT CaptureRenderer::Init(LPDIRECT3DDEVICE9 pDevice, S3RenderEngine *pRenderEngine)
{
    m_pDevice = pDevice;
    m_pRenderEngine = pRenderEngine;
    return S_OK;
}

HRESULT CaptureRenderer::EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree, CRect ClipInfo)
{
    m_bEnableSFRUpload = bEnabled;
    m_bEnableSplitUpload = bSplit;
    m_SFRRect = *pDisplayRect;
    m_RotateDegree = RotateDegree;
    m_ClipInfo = ClipInfo;
    return S_OK;
}


HRESULT CaptureRenderer::ForceNumaNode(int Node)
{
    m_PreferedNumaNode = Node;
    return S_OK;
}

HRESULT CaptureRenderer::SetEncrpytEnable()
{
    m_bHDCPEnable = TRUE;
    m_CaptureToPCIe = TRUE;
    return S_OK;

}
