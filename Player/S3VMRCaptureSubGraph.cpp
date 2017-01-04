#include "stdafx.h"
#include "S3Signage.h"
#include "S3VMRWizard.h"
#include "S3RenderEngine.h"
#include "S3VMRCaptureSubgraph.h"
#include "CaptureRenderer.h"
#include "Utilities\wd3iface_i.h "
#include "Utilities\StringUtility.h"

#import "Utilities\DGC133ST.tlb" no_namespace named_guids

#define S3C_ENABLEYUY2 1



#define MAX_SUPPORTED_CAPTURE_CARDS     20
#define MAX_SUB_CHILD_COUNT             4

#define MAX_CHILD_DEVICES               (MAX_SUPPORTED_CAPTURE_CARDS*MAX_SUB_CHILD_COUNT)
// Considerring the extreme condition: 3 bridge chips exist on the branch of each capture card and all capture cards do
// not share the same capture device.
#define MAX_BRIDGE_COUNT_FOR_CAPTURE    (MAX_CHILD_DEVICES+MAX_SUPPORTED_CAPTURE_CARDS*2*3)

typedef struct _S3_PCIE_BRIDGE_CONFIG_SPACE
{
    WORD    VenderID;
    WORD    DeviceID;
    WORD    Command; 
    WORD    Status;
    UCHAR   RevisionID;
    UCHAR   ProgIF;
    UCHAR   SubClass;       // Offset 0x0a: SubClass       04 PCI-PCI Bridge
    UCHAR   BaseClass;      // Offset 0x0b: BaseClass      06 Bridge Device
    UCHAR   CacheLineSize;
    UCHAR   LatencyTimer;
    UCHAR   HeaderType;     // Offset 0x0e
    UCHAR   BIST;
    ULONG   BAR0;           // Offset 0x10
    ULONG   BAR1;           // Offset 0x14
    UCHAR   PriBusNum;      // Offset 0x18
    UCHAR   SecBusNum;      // Offset 0x19      //if 19 = 1a means last  device/bridge
    UCHAR   SubBusNum;      // Offset 0x1a
    UCHAR   SecLatencyTmr;
} S3_PCIE_BRIDGE_CONFIG_SPACE,*PS3_PCIE_BRIDGE_CONFIG_SPACE;


// Capture card's pci config and its bus/device/function, indicate it is 4 capture card or not.
// Its parent' upper stream bridge index in array and down stream bridge index in array
typedef struct _S3_STREAM_CHILD_DEVICE
{
    ULONG                           bus;
    ULONG                           device;
    ULONG                           function;
    S3_PCIE_BRIDGE_CONFIG_SPACE     PciSpace;

    BOOL                            b4CaptureCard;      // Indicate it is S3 4 capture input card or not.
                                                        // Since single card also can link to PLX bridge in motherboard, we can not simply differ it is 4 capture 
                                                        // card from it's parent PLX bridge or not. We need match all bridge's child just only 4,5,6,8,9 S3 capture card.
    ULONG                           UpperStreamIndex;
    BOOL                            DownStreamIndex;
    ULONG                           ChipsetBridgeIndex;
} S3_STREAM_CHILD_DEVICE,*PS3_STREAM_CHILD_DEVICE;

// Down stream bridge's pci config and its bus/device/function, also its capture child device index.
typedef struct _S3_BRIDGE
{
    ULONG                           bus;
    ULONG                           device;
    ULONG                           function;
    S3_PCIE_BRIDGE_CONFIG_SPACE     PciSpace;

    ULONG                           UpperBridgeIndex;
    ULONG                           NumberOfDownBridge; // If NumberOfDownBridge > 0, DownBridgeIndex is valid, else it is downstream device and ChildDeviceIndex is valid.
    ULONG                           DownBridgeIndex[MAX_SUPPORTED_CAPTURE_CARDS];   // Down bridge devices which in the branch of capture device.

    ULONG                           ChildDeviceIndex;   // Valid only when NumberOfDownBridge == 0.
} S3_BRIDGE,*PS3_BRIDGE;

typedef struct _S3_CAPTURE_CARD_ARCHITECTURE
{
    ULONG                       NumberOfCaptureCard;
    S3_STREAM_CHILD_DEVICE      child_device[MAX_CHILD_DEVICES];

    ULONG                       NumberOfBridges;
    // Bridges assosiated with s3 capture adapter. index 0 is used as no bridge case.
    S3_BRIDGE                   bridges[MAX_BRIDGE_COUNT_FOR_CAPTURE+1];

} S3_CAPTURE_CARD_ARCHITECTURE,*PS3_CAPTURE_CARD_ARCHITECTURE;





bool WstrCmp(const WCHAR* str1, const WCHAR* str2)
{
    if(!str1 || !str2)
    {
        return false;
    }
    UINT i=0;
    while(1)
    {
        if(str1[i] != str2[i])
        {
            return false;
        }
        if(str1[i] == 0)
        {
            break;
        }
        i++;
    }
    return true;
}


void UtilDeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    if (pmt->cbFormat != 0) 
    {
        CoTaskMemFree((PVOID)pmt->pbFormat);

        // Strictly unnecessary but tidier
        pmt->cbFormat = 0;
        pmt->pbFormat = NULL;
    }
    if (pmt->pUnk != NULL) 
    {
        pmt->pUnk->Release();
        pmt->pUnk = NULL;
    }
    CoTaskMemFree((PVOID)pmt);
}


/******************************Public*Routine******************************\
* S3VMRCaptureSubgraph
*
* constructor
\**************************************************************************/

S3VMRCaptureSubGraph::S3VMRCaptureSubGraph()
{
    m_pCapture = NULL;
    m_pGraph = NULL;
    m_pMc = NULL;
#ifdef USE_VMR_RENDER
    m_pVMR = NULL;
#else
    m_pCR = NULL;
#endif
    m_pConfig = NULL;
    m_pOutPin = NULL;

    m_LastDetectTime = 0;
    m_bDatapath = FALSE;
    m_bTC2000 = FALSE;

}

/******************************Public*Routine******************************\
* S3VMRCaptureSubgraph
*
* destructor
\**************************************************************************/
S3VMRCaptureSubGraph::~S3VMRCaptureSubGraph()
{
    SAFE_RELEASE(m_pCR);
}


HRESULT SetFrameRate(
   ULONG          frameRate,
   AM_MEDIA_TYPE  *pAMT)
{
   if (pAMT->formattype == FORMAT_VideoInfo)
   {
      VIDEOINFOHEADER *pVIH;
      // Set the capture interval.
      pVIH = (VIDEOINFOHEADER*)pAMT->pbFormat;
      pVIH->AvgTimePerFrame = (int)(10000000 / (frameRate+(float)0.0005));
   }

   else
   {
      return E_UNEXPECTED;
   }

   return S_OK;
}


/******************************Public*Routine******************************\
* BuildAndRender
*
* Builds the filter graph to render the media file located at wcPath with use of VMR9; 
* if wizard is provided, VMR9 is set to the renderless mode and custom allocator-
* presenter (which is pWizard in this sample) is advised.
*
* Return values: errors from the filter graph and wizard
\**************************************************************************/
HRESULT S3VMRCaptureSubGraph::BuildAndRender( CString DeviceName, int ChannelNo, BOOL bForceXRGB, CString DeviceClsID)
{
    HRESULT hr;

    CComPtr<IVMRFilterConfig9> pVMRConfig;

    DeviceName.Trim();
    m_DeviceName = DeviceName;
    m_DeviceClsID = DeviceClsID;

    m_ChannelNo = ChannelNo;

    m_bForceXRGB = bForceXRGB;

    m_pCapture = NULL;
    m_pGraph = NULL;
    m_pMc = NULL;
#ifdef USE_VMR_RENDER
    m_pVMR = NULL;
#else
    m_pCR = NULL;
#endif
    m_pConfig = NULL;
    m_pOutPin = NULL;

    //add your code here
    m_bCaptureInited        = false;


    hr = CoInitialize(NULL);


    hr=CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, 
                    IID_ICaptureGraphBuilder2, (void **) &m_pCapture);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_pGraph.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC);
    m_pGraph.QueryInterface(&m_pMc);
//////////////////////////////////////////////////
    hr = m_pCapture->SetFiltergraph(m_pGraph);
    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr <ICreateDevEnum> pCde;
    CComPtr <IEnumMoniker> pEm;

    hr=CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **) &pCde);

    if (FAILED(hr))
    {
        return hr;
    }

    hr = pCde->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);

    if (FAILED(hr))
    {
        return hr;
    }
    if(pEm==NULL)
    {
        return E_FAIL;
    }

    CComPtr <IMoniker> pM[32];

    ULONG cFetched;

    CComPtr<IBaseFilter> pBf=NULL;

    hr = pEm->Next(32,&pM[0],&cFetched);

    if (FAILED(hr))
    {
        return hr;
    }

    UINT i;
    UINT usedDev = 0;

    if(cFetched)
    {
        IPropertyBag* piPropertyBag = NULL;
        VARIANT varName;
        VariantInit(&varName);
        for(i=0;i<cFetched;i++)
        {
            BOOL bDatapathHD = FALSE;

            hr = pM[i]->BindToStorage(0, 0, IID_IPropertyBag, reinterpret_cast<void**>(&piPropertyBag) );
            if (SUCCEEDED(hr))
            {
                hr = piPropertyBag->Read(L"DevicePath", &varName, 0);
                if (SUCCEEDED(hr))
                {
                    if(WstrCmp(m_DeviceName, varName.bstrVal))
                    {
                        m_bCaptureInited = true;
                        usedDev = i;
                    }

                    if(m_DeviceName.CompareNoCase(_T("auto")) == 0)
                    {
                        m_bCaptureInited = true;
                        usedDev = i;
                    }

                    CString Num;
                    Num.Format(_T("#%d"), i);
                    if (m_DeviceName.CompareNoCase(Num) == 0)
                    {
                        m_bCaptureInited = true;
                        usedDev = i;
                    }
                    VariantClear(&varName);
                }
                else if (DeviceName.IsEmpty())
                {
                    hr = pM[i]->BindToObject(
                        NULL,
                        NULL,
                        IID_IBaseFilter,
                        reinterpret_cast< void** >( &pBf ) );
                    if( SUCCEEDED( hr ) )
                    {

                        CLSID ClsID;
                        hr = pBf->GetClassID(&ClsID);
                        if( !SUCCEEDED( hr ) )
                        {
                            ASSERT(0);
                        }

                        if (0 == DeviceClsID.CompareNoCase(StringUtility::GUIDToStr(ClsID).c_str()))
                        {
                            m_bCaptureInited = true;
                            usedDev = i;

                            CComPtr<IVision>   pDatapathConfig;
                            hr = pBf->QueryInterface(
                                __uuidof( pDatapathConfig ),
                                reinterpret_cast< void** >( &pDatapathConfig ) );
                            if( SUCCEEDED( hr ) && pDatapathConfig != NULL)
                            {
                                bDatapathHD = TRUE;
                            }
                        }
                        else
                        {
                            pBf = NULL;
                        }
                    }
                }

                hr = piPropertyBag->Read(L"FriendlyName", &varName, 0);
                if (SUCCEEDED(hr))
                {
                    CString FriendlyName = varName.bstrVal;

                    if(m_bCaptureInited)
                    {
                        if(FriendlyName.MakeLower().Find(_T("datapath")) >= 0 && bDatapathHD)
                        {
                            m_bDatapath = TRUE;
                        }
                        if(FriendlyName.MakeLower().Find(_T("tc2000")) >= 0)
                        {
                            m_bTC2000 = TRUE;
                        }
                    }

                    VariantClear(&varName);
                }

                piPropertyBag->Release();
            }

			if(m_bCaptureInited)
			{
				break;
			}
        }
    }
    if(!m_bCaptureInited)
    {
        return E_FAIL;
    }

#ifdef USE_VMR_RENDER
    S3VMRWizard* pWizard = (S3VMRWizard *)theApp.GetWizard();

	// create and add VMR9
	hr = CoCreateInstance( CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&(m_pVMR.p) );
	if( FAILED(hr))
	{
		DbgMsg(_T("Failed to create instance of VMR9"));
		return hr;
	}

	hr = m_pGraph->AddFilter( m_pVMR, L"VMR9");
	if( FAILED(hr))
	{
		DbgMsg(_T("Failed to add VMR9 to the graph"));
		return hr;
	}
	// configure VMR9
	hr = m_pVMR->QueryInterface( IID_IVMRFilterConfig9, (void**)&(pVMRConfig.p) );
	if( FAILED(hr))
	{
		DbgMsg(_T("Cannot get IVMRFilterConfig9 from VMR9"));
		return hr;
	}
    
	// if wizard is provided, set VMR to the renderless code and attach to the wizard
	if( pWizard )
	{
		// set VMR to the renderless mode
		hr = pVMRConfig->SetRenderingMode( VMR9Mode_Renderless );
		if( FAILED(hr))
		{
			DbgMsg(_T("Failed to set VMR9 to the renderless mode"));
			return hr;
		}

		hr = pWizard->Attach( m_pVMR, &m_dwID );
		if( FAILED(hr))
		{
			DbgMsg(_T("Failed to attach graph to the wizard"));
			return hr;
		}
	}


#else

    CComPtr<IBaseFilter> pRender;
    
    m_pCR = new CaptureRenderer(&hr);

    m_pCR->AddRef();

    S3RenderEngine *pRenderEngine;
    theApp.GetWizard()->GetRenderEngine(&pRenderEngine);

    IDirect3DDevice9 *pDevice;
    pRenderEngine->Get3DDevice(&pDevice);

    m_pCR->Init(pDevice, pRenderEngine);

    pDevice->Release();

    if (FAILED(hr) || !m_pCR)
    {
        m_bCaptureInited = false;
        return hr;
    }
    pRender = m_pCR;
    m_pGraph->AddFilter(pRender, L"S3 Capture Renderer");

#endif

    if (!pBf)
    {
        hr = pM[usedDev]->BindToObject(0,0,IID_IBaseFilter, (void**)&pBf);
    }

    hr = m_pGraph->AddFilter(pBf, L"Video Capture");
    if(FAILED(hr))
    {
        m_bCaptureInited = false;
        return hr;
    }

    hr = m_pCapture->FindInterface( 
        &PIN_CATEGORY_CAPTURE, //  
        &MEDIATYPE_Video,    // Any media type. 
        pBf, // Pointer to the capture filter. 
        IID_IAMStreamConfig, (void**)&m_pConfig); 

    if(FAILED(hr))
    {
        return hr;
    }

    int iCount = 0, iSize = 0; 
    hr = m_pConfig->GetNumberOfCapabilities(&iCount, &iSize); 

    BOOL bSupportYUY2 = FALSE;
    BOOL bSupportUYVY = FALSE;
    BOOL bSupportRGB24 = FALSE;
    BOOL bSupportRGB32 = FALSE;


    if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
    { 
        VIDEO_STREAM_CONFIG_CAPS scc; 
        AM_MEDIA_TYPE *pmtConfig; 

        // Use the video capabilities structure. 
        for (int iFormat = 0; iFormat < iCount; iFormat++) 
        { 
            hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc); 
            if (SUCCEEDED(hr)) 
            { 
                /**//* Examine the format, and possibly use it. */ 
                if ((pmtConfig->majortype == MEDIATYPE_Video) && 
                    (pmtConfig->formattype == FORMAT_VideoInfo) && 
                    (pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) && 
                    (pmtConfig->pbFormat != NULL)) 
                { 
                    if(IsEqualGUID(pmtConfig->subtype, MEDIASUBTYPE_YUY2))
                    {
                        bSupportYUY2 = TRUE;
                    }

                    if(IsEqualGUID(pmtConfig->subtype, MEDIASUBTYPE_RGB24))
                    {
                        bSupportRGB24 = TRUE;
                    }
                    
                    if(IsEqualGUID(pmtConfig->subtype, MEDIASUBTYPE_UYVY))
                    {
                        bSupportUYVY = TRUE;
                    }

                    if(IsEqualGUID(pmtConfig->subtype, MEDIASUBTYPE_RGB32))
                    {
                        bSupportRGB32 = TRUE;
                    }

                    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat; 
                    // pVih contains the detailed format information. 
                    LONG lWidth = pVih->bmiHeader.biWidth; 
                    LONG lHeight = pVih->bmiHeader.biHeight;


                    //DbgMsg(_T("Found Capture format %d*%d %dbit\n"), lWidth, lHeight,  pVih->bmiHeader.biBitCount);

                } 
                // Delete the media type when you are done. 
                UtilDeleteMediaType(pmtConfig);
            }
        }
    }

    GUID PreferFormat;

    if(bForceXRGB)
    {
        if(bSupportRGB24)
        {
            PreferFormat = MEDIASUBTYPE_RGB24;
        }else if(bSupportRGB32)
        {
            PreferFormat = MEDIASUBTYPE_RGB32;
        }
        else if(bSupportYUY2)
        {
            PreferFormat = MEDIASUBTYPE_YUY2;
        }
        else if(bSupportUYVY)
        {
            PreferFormat = MEDIASUBTYPE_UYVY;
        }
        else
        {
            return E_FAIL;
        }

    }else
    {
 
        if(bSupportYUY2)
        {
            PreferFormat = MEDIASUBTYPE_YUY2;
        }
        else if(bSupportUYVY)
        {
            PreferFormat = MEDIASUBTYPE_UYVY;
        }
        else if(bSupportRGB24)
        {
            PreferFormat = MEDIASUBTYPE_RGB24;
        }else if(bSupportRGB32)
        {
            PreferFormat = MEDIASUBTYPE_RGB32;
        }
        else
        {
            return E_FAIL;
        }
    }


    m_PreferFormat = PreferFormat;

    hr = m_pCapture->FindPin( 
        pBf,
        PINDIR_OUTPUT,
        &PIN_CATEGORY_CAPTURE, // Preview pin. 
        &MEDIATYPE_Video,    // Any media type. 
        TRUE,
        m_ChannelNo,
        &m_pOutPin); 

    if(FAILED(hr))
    {
        m_bCaptureInited = false;
        return hr;
    }


    if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
    { 
        VIDEO_STREAM_CONFIG_CAPS scc; 
        AM_MEDIA_TYPE *pmtConfig; 

        // Use the video capabilities structure. 
        for (int iFormat = 0; iFormat < iCount; iFormat++) 
        { 
            hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc); 
            if (SUCCEEDED(hr)) 
            { 
                /**//* Examine the format, and possibly use it. */ 
                if ((pmtConfig->majortype == MEDIATYPE_Video) && 
                    (pmtConfig->formattype == FORMAT_VideoInfo) && 
                    (pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) && 
                    (pmtConfig->pbFormat != NULL)) 
                { 
                    if(IsEqualGUID(pmtConfig->subtype, m_PreferFormat))
                    {
                        SetFrameRate(30, pmtConfig);
            
                        hr = m_pConfig->SetFormat(pmtConfig);
                        UtilDeleteMediaType(pmtConfig);
                        break;
                    }

                } 
                // Delete the media type when you are done. 
                UtilDeleteMediaType(pmtConfig);
            }
        }
    }


#ifdef USE_VMR_RENDER

    CComPtr<IEnumPins> pEnum;
    CComPtr<IPin> pPin;


    hr = m_pVMR->EnumPins( &pEnum );

    hr = pEnum->Next( 1, &pPin, NULL);

    hr = m_pOutPin->Connect(pPin.p, NULL);
#else
    //m_pVMR
    m_pCR->ForceMediaType(PreferFormat);

	CComPtr<IWd3KsproxySampleConfig>   pS3Config;
	hr = pBf->QueryInterface(
		__uuidof( pS3Config ),
		reinterpret_cast< void** >( &pS3Config ) );
	if(pS3Config != NULL && SUCCEEDED(hr))
	{
        LONG busNumber = 0;

        hr = pS3Config->GetBusNumber(&busNumber);

        LONG RootBusNumber = busNumber;
        LONG RootBridgeIndex = 0;



        S3_CAPTURE_CARD_ARCHITECTURE BusInfo;


        hr = pS3Config->GetCaptureCardArchitecture(&BusInfo);


        for(int i=0; i< BusInfo.NumberOfCaptureCard; i++)
        {
            if(BusInfo.child_device[i].bus == busNumber)
            {
                RootBridgeIndex = BusInfo.child_device[i].ChipsetBridgeIndex;
                break;
            }
        }
        
        do{

            RootBusNumber = BusInfo.bridges[RootBridgeIndex].bus;

            if(BusInfo.bridges[RootBridgeIndex].UpperBridgeIndex == 0)
                break;

            RootBridgeIndex = BusInfo.bridges[RootBridgeIndex].UpperBridgeIndex;

        }
        while(TRUE);



        DWORD NumOfRootBridge = 0;
        DWORD RootBridgeBus[MAX_BRIDGE_COUNT_FOR_CAPTURE];

        memset(RootBridgeBus, 0, sizeof(RootBridgeBus));

        for(int i=1; i< BusInfo.NumberOfBridges ; i++)
        {
            if( BusInfo.bridges[i].UpperBridgeIndex == 0)
            {
                 BOOL bFound = FALSE;
                 for(int j=0; j< NumOfRootBridge; j++)
                 {
                     if(RootBridgeBus[j] == BusInfo.bridges[i].bus)
                     {
                         bFound = TRUE;
                         break;
                     }
                 }

                 if(!bFound)
                 {
                    RootBridgeBus[NumOfRootBridge] = BusInfo.bridges[i].bus;
                    NumOfRootBridge++;
                 }
            }
        }

        if(NumOfRootBridge > 1)
        {
            RootBridgeIndex = 0;
            for(int i=0; i< NumOfRootBridge; i++)
            {
                if(RootBridgeBus[i] == RootBusNumber)
                {
                    RootBridgeIndex = i;
                    break;
                }
            }

            m_pCR->ForceNumaNode(RootBridgeIndex);
        }


        BOOL bHDCPEnabled = FALSE;
        pS3Config->GetSignalHDCP((long *)&bHDCPEnabled);
        if(bHDCPEnabled)
        {
            pS3Config->SetEnableEncrypeKey(TRUE);
            m_pCR->SetEncrpytEnable();
        }
    }

    CComPtr<IEnumPins> pEnum;
    CComPtr<IPin> pPin;


    hr = m_pCR->EnumPins( &pEnum );

    hr = pEnum->Next( 1, &pPin, NULL);

    hr = m_pOutPin->Connect(pPin.p, NULL);
#endif

    if(FAILED(hr))
    {
        m_bCaptureInited = false;
        return hr;
    }
    m_LastDetectTime = timeGetTime();


    return S_OK;
}



HRESULT S3VMRCaptureSubGraph::HandleEvent()
{
    HRESULT hr = S_OK;

    if(timeGetTime() - m_LastDetectTime < 1000)
    {
        return S_OK;
    }

    m_LastDetectTime = timeGetTime();

    if( !m_pGraph )
    {
        return E_POINTER;
    }

    if(m_bDatapath)
    {
        OAFilterState state = State_Stopped;
        if( m_pMc )
        {
            HRESULT hr = m_pMc->GetState( 20, &state );
        }
        if(state != State_Running)
        {
            Run();
        }
    }


    BOOL bNeedRebuiltGraph = FALSE;
    
    if (m_ChannelNo == 0)
    {
        int iCount = 0, iSize = 0; 
        hr = m_pConfig->GetNumberOfCapabilities(&iCount, &iSize); 

        // Check the size to make sure we pass in the correct structure. 
        //m_drawSize.cx = m_drawSize.cy = 0;
        int choosenFormat = -1;
        //int rectarea = m_rectSize.cx*m_rectSize.cy;
        if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
        { 
            VIDEO_STREAM_CONFIG_CAPS scc; 
            AM_MEDIA_TYPE *pmtConfig; 

            // Use the video capabilities structure. 
            for (int iFormat = 0; iFormat < iCount; iFormat++) 
            { 
                hr = m_pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc); 
                if (SUCCEEDED(hr)) 
                { 
                    /**//* Examine the format, and possibly use it. */ 
                    if ((pmtConfig->majortype == MEDIATYPE_Video) && 
                        (IsEqualGUID(pmtConfig->subtype, m_PreferFormat)) && 
                        (pmtConfig->formattype == FORMAT_VideoInfo) && 
                        (pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) && 
                        (pmtConfig->pbFormat != NULL)) 
                    { 
                        VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat; 
                        // pVih contains the detailed format information. 
                        LONG lWidth = scc.InputSize.cx; 
                        LONG lHeight = scc.InputSize.cy;

                        //DbgMsg(_T("%d*%d %dbit\n"), lWidth, lHeight,  pVih->bmiHeader.biBitCount);

#ifdef USE_VMR_RENDER
                        LONG videoWidth, videoHeight;
                        LONG preferedx, preferedy;
                        S3VMRWizard* pWizard = theApp.GetWizard();

                        if(FAILED(pWizard->GetVideoSize(m_dwID, &videoWidth, &videoHeight, &preferedx, &preferedy)))
                        {
                            return S_FALSE;
                        }

                        if (lWidth != preferedx || lHeight != preferedy)
                        {
                            bNeedRebuiltGraph = TRUE;
                        }
#else
                        if (lWidth != m_pCR->m_lVidWidth || lHeight != m_pCR->m_lVidHeight)
                        {
                            bNeedRebuiltGraph = TRUE;
                        }

                        if(m_bDatapath)
                        {
                            if(timeGetTime() - m_pCR->m_lastFrameTime > 2000)
                            {
                                bNeedRebuiltGraph = TRUE;

                            }
                        }
#endif

                        choosenFormat = iFormat;

                        UtilDeleteMediaType(pmtConfig);

                        break;

                    } 
                    // Delete the media type when you are done. 
                    UtilDeleteMediaType(pmtConfig);
                }
            }
        }
    }
    else
    {
        CComPtr<IEnumMediaTypes> pEnum;

        hr = m_pOutPin->EnumMediaTypes(&pEnum);
        if (SUCCEEDED(hr))
        {   
            ULONG nfetched = 0;
            AM_MEDIA_TYPE *pmtConfig; 
            while (SUCCEEDED(pEnum->Next(1, &pmtConfig,&nfetched)) && nfetched > 0)
            {        
                /**//* Examine the format, and possibly use it. */ 
                if ((pmtConfig->majortype == MEDIATYPE_Video) && 
                    (IsEqualGUID(pmtConfig->subtype, m_PreferFormat)) && 
                    (pmtConfig->formattype == FORMAT_VideoInfo) && 
                    (pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) && 
                    (pmtConfig->pbFormat != NULL)) 
                {                      
                    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat; 

                    // pVih contains the detailed format information. 
                    LONG lWidth = pVih->bmiHeader.biWidth; 
                    LONG lHeight = pVih->bmiHeader.biHeight;

#ifdef USE_VMR_RENDER
                    LONG videoWidth, videoHeight;
                    LONG preferedx, preferedy;
                    S3VMRWizard* pWizard = theApp.GetWizard();

                    if(FAILED(pWizard->GetVideoSize(m_dwID, &videoWidth, &videoHeight, &preferedx, &preferedy)))
                    {
                        return S_FALSE;
                    }

                    if (lWidth != preferedx || lHeight != preferedy)
                    {
                        bNeedRebuiltGraph = TRUE;
                    }
#else
                    if (lWidth != m_pCR->m_lVidWidth || lHeight != m_pCR->m_lVidHeight)
                    {
                        bNeedRebuiltGraph = TRUE;
                    }

                    if(m_bDatapath)
                    {
                        if(timeGetTime() - m_pCR->m_lastFrameTime > 2000)
                        {
                            bNeedRebuiltGraph = TRUE;

                        }
                    }
#endif

                    UtilDeleteMediaType(pmtConfig);

                    break;

                } 
                // Delete the media type when you are done. 
                UtilDeleteMediaType(pmtConfig);
            }
        }
    }


    if (bNeedRebuiltGraph)
    {
        CAutoLock AutoLock(&SurfaceMutex);
        Stop();
        DestroyGraph();

        BuildAndRender(m_DeviceName, m_ChannelNo, m_bForceXRGB, m_DeviceClsID);
        Run();
    }

    return S_OK;
}


/******************************Public*Routine******************************\
* Run
\**************************************************************************/
HRESULT S3VMRCaptureSubGraph::Run()
{
    HRESULT hr = S_OK;

    hr = m_pMc->Run();

    return hr;
}

/******************************Public*Routine******************************\
* Stop
\**************************************************************************/
HRESULT S3VMRCaptureSubGraph::Stop()
{
    HRESULT hr = S_OK;
    OAFilterState state;

    if( !m_pMc )
    {
        return E_UNEXPECTED;
    }

    hr = m_pMc->Stop();
    state = State_Running;

    while( State_Stopped != state && SUCCEEDED(hr))
    {
        hr = m_pMc->GetState(100, &state);
    }


    return hr;
}

/******************************Public*Routine******************************\
* GetState
*
* Returns OAFilterState from IMediaControl of the graph
*
* Return values: errors from the filter graph and wizard
\**************************************************************************/
OAFilterState S3VMRCaptureSubGraph::GetState()
{
    OAFilterState state = State_Stopped;
    if( m_pMc )
    {
        HRESULT hr = m_pMc->GetState( 20, &state );
    }
    return state;
}




/******************************Public*Routine******************************\
* DestroyGraph
*
* Stops the graph, destroys and removes all the filters (VMR9 is removed last)
*
\**************************************************************************/
HRESULT S3VMRCaptureSubGraph::DestroyGraph()
{
    HRESULT hr = S_OK;
    OAFilterState state;


#ifdef USE_VMR_RENDER
    S3VMRWizard* pWizard = (S3VMRWizard *)theApp.GetWizard();
    pWizard->Detach(m_dwID);
#endif


    if( !m_pGraph )
    {
        return E_POINTER;
    }


    FILTER_INFO fi;
    CComPtr<IMediaControl> pMc;
    CComPtr<IEnumFilters> pEnum;
    CComPtr<IBaseFilter> pFilter;
    CComPtr<IBaseFilter> pVMR = NULL;

    // 1. stop the graph
    hr = m_pGraph->QueryInterface( IID_IMediaControl, (void**)&(pMc.p) );
    if( FAILED(hr))
    {
        return hr;
    }

    do
    {
        hr = pMc->GetState(100, &state);
    } while( S_OK == hr && State_Stopped != state );

    hr = m_pGraph->EnumFilters( &(pEnum.p) );
    if( FAILED(hr))
    {
        return hr;
    }

    // tear off
    hr = pEnum->Next(1, &(pFilter.p), NULL);
    while( S_OK == hr && pFilter )
    {
        hr = DisconnectPins( pFilter );
        pFilter = NULL;
        hr = pEnum->Next(1, &(pFilter.p), NULL);
    }
    pFilter = NULL;

    // remove filters
    hr = pEnum->Reset();
    hr = pEnum->Next(1, &(pFilter.p), NULL);
    while( S_OK == hr && pFilter )
    {
        hr = pFilter->QueryFilterInfo( &fi);
        if( fi.pGraph)
            fi.pGraph->Release();

        if( 0 == wcscmp( fi.achName, L"VMR9"))
        {
            pVMR = pFilter;
        }
        hr = m_pGraph->RemoveFilter( pFilter);
        pFilter = NULL;

        hr = pEnum->Reset();
        hr = pEnum->Next(1, &pFilter, NULL);
    }

    pFilter = NULL;
    pEnum = NULL;
    pVMR = NULL;


    m_pCapture = NULL;
    m_pGraph = NULL;
    m_pMc = NULL;
    m_pConfig = NULL;
    m_pOutPin = NULL;



#ifdef USE_VMR_RENDER
    m_pVMR = NULL;     
#else
    SAFE_RELEASE(m_pCR);
#endif


    return S_OK;
}

/******************************Public*Routine******************************\
* DisconnectPins
*
* Disconnects pins of a filter
*
\**************************************************************************/
HRESULT S3VMRCaptureSubGraph::DisconnectPins( CComPtr<IBaseFilter> pFilter)
{
    HRESULT hr = S_OK;

    CComPtr<IEnumPins> pEnum;
    CComPtr<IPin> pPin;

    if( !pFilter )
    {
        return E_POINTER;
    }

    hr = pFilter->EnumPins( &pEnum );
    if( FAILED(hr))
    {
        return hr;
    }
    hr = pEnum->Next( 1, &pPin, NULL);

    while( S_OK == hr && pPin )
    {
        hr = pPin->Disconnect();
        pPin = NULL;
        hr = pEnum->Next( 1, &pPin, NULL);
    }

    pPin = NULL;
    pEnum = NULL;

    return S_OK;
}


HRESULT S3VMRCaptureSubGraph::GetTexture(LPDIRECT3DTEXTURE9* ppTexture)
{
    SurfaceMutex.Lock();

#ifdef USE_VMR_RENDER
    S3VMRWizard* m_pWizard = (S3VMRWizard *)theApp.GetWizard();

    if(m_pWizard)
    {
        return  m_pWizard->GetTexture(m_dwID, ppTexture);
    }
#else
    if (m_pCR)
    {
        return m_pCR->GetTexture(ppTexture);
    }
#endif    
    return E_FAIL;
}

HRESULT S3VMRCaptureSubGraph::GetVideoSize(LONG* plWidth, LONG* plHeight, LONG* preferedx, LONG* preferedy)
{
#ifdef USE_VMR_RENDER
    S3VMRWizard* m_pWizard = (S3VMRWizard *)theApp.GetWizard();

    if(m_pWizard)
    {
        return m_pWizard->GetVideoSize(m_dwID, plWidth, plHeight, preferedx, preferedy); 
    }
 
#else
    if (m_pCR)
    {
        return m_pCR->GetVideoSize(plWidth, plHeight, preferedx, preferedy);
    }
#endif  
    return E_FAIL;
}

HRESULT S3VMRCaptureSubGraph::EnableSFRUpload(BOOL bEnabled, BOOL bSplit, RECT* pDisplayRect, FLOAT RotateDegree,  RECT Clip)
{
#ifdef USE_VMR_RENDER
    S3VMRWizard* m_pWizard = (S3VMRWizard *)theApp.GetWizard();

    if(m_pWizard)
    {
        return m_pWizard->EnableSFRUpload(m_dwID, bEnabled, bSplit, pDisplayRect, RotateDegree);
    }
#else
    if (m_pCR)
    {
        return m_pCR->EnableSFRUpload(bEnabled, bSplit, pDisplayRect, RotateDegree, Clip);
    }
#endif  
    return E_FAIL;
}

HRESULT S3VMRCaptureSubGraph::ReleaseTexture()
{
#ifdef USE_VMR_RENDER
    S3VMRWizard* m_pWizard = (S3VMRWizard *)theApp.GetWizard();

    if(m_pWizard)
    {
        m_pWizard->ReleaseTexture(m_dwID);
    }
#else
#endif
    SurfaceMutex.Unlock();
    return S_FALSE;
}
