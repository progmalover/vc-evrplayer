#include "stdafx.h"
#include "S3Signage.h"
#include "S3DShowSubGraph.h"
#include "S3DShowWizard.h"
#include "S3RenderEngine.h"
#include "S3RenderMixer.h"
#include "S3Signage.h"

const IID CLSID_FFDShowDecoder = {0x04fe9017, 0xf873, 0x410e, {0x87, 0x1e, 0xab, 0x91, 0x66, 0x1a, 0x4e, 0xf7}};
const IID CLSID_DXVADShowDecoder = {0x4bd951fd, 0x84aa, 0x4dcf, {0xb8, 0x08, 0x5f, 0xe7, 0xf7, 0x30, 0x83, 0xcc}};
const IID CLSID_PDVDDecoder = {0xc16541ff, 0x49ed, 0x4dea, {0x91, 0x26, 0x86, 0x2f, 0x57, 0x72, 0x2e, 0x31}};
const IID CLSID_VTDecoder = {0x971a86ba, 0xf54d, 0x47d4, {0xa3, 0x77, 0x50, 0x9c, 0x72, 0xbe, 0xe0, 0xf7}};
const IID CLSID_PDVD9MP4Spliter = {0x97D48B32, 0xAFD8, 0x4923, {0xba, 0x97, 0xF4, 0xF3, 0xB9, 0x9B, 0xF2, 0x93}};

static HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath) 
{
    const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
    HRESULT hr;

    IStorage *pStorage = NULL;
    hr = StgCreateDocfile(
        wszPath,
        STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
        0, &pStorage);
    if(FAILED(hr)) 
    {
        return hr;
    }

    IStream *pStream;
    hr = pStorage->CreateStream(
        wszStreamName,
        STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
        0, 0, &pStream);
    if (FAILED(hr)) 
    {
        pStorage->Release();    
        return hr;
    }

    IPersistStream *pPersist = NULL;
    pGraph->QueryInterface(IID_IPersistStream, (void**)&pPersist);
    hr = pPersist->Save(pStream, TRUE);
    pStream->Release();
    pPersist->Release();
    if (SUCCEEDED(hr)) 
    {
        hr = pStorage->Commit(STGC_DEFAULT);
    }
    pStorage->Release();
    return hr;
}

S3DShowSubGraph::S3DShowSubGraph(void)
    : m_dwID( NULL)
    , m_RepeatMode( S3S_PS_NONE)
	, m_bStream ( FALSE )
{
    m_wcPath[0] = L'\0';
    m_achPath[0] = TEXT('\0');
}

S3DShowSubGraph::~S3DShowSubGraph(void)
{
}

/******************************Public*Routine******************************\
* BuildAndRender
*
* Return values: errors from the filter graph and wizard
\**************************************************************************/
HRESULT S3DShowSubGraph::BuildAndRender( WCHAR *wcPath, S3DShowWizard * pWizard, HWND hWnd)
{
    HRESULT hr = S_OK;
    CComPtr<IGraphBuilder>  pGb;

    m_hWnd = hWnd;
    if( !wcPath )
    {
        return E_POINTER;
    }

    USES_CONVERSION;
    (void)StringCchCopyW( m_wcPath, MAX_PATH, wcPath);
    (void)StringCchCopy( m_achPath, MAX_PATH, W2T(wcPath));

	CString UrlPrefix = wcPath;
    UrlPrefix = UrlPrefix.Left(UrlPrefix.Find(_T("://")));
    CString UrlExtension = wcPath;
    UrlExtension = UrlExtension.Right(UrlExtension.GetLength() - UrlExtension.ReverseFind(_T('.')) - 1);


    if(UrlPrefix.CompareNoCase(_T("rtsp")) == 0)
	{
		m_bStream = TRUE;
		// create graph
		hr = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
			IID_IFilterGraph, (void**)&(m_pGraph.p) );
		if( FAILED(hr))
		{
			DbgMsg(_T("Failed to create the filter graph"));
			return hr;
		}

		CComPtr<IFileSourceFilter> pSrcFilter;
		CComPtr<IBaseFilter>    pRtspStream;


        AddDecoderFilter();

		const IID CLSID_RtspStream = {0x4d668173, 0x35f, 0x47cc, {0x9b, 0xf9, 0x63, 0x34, 0xcc, 0x22, 0x0, 0x11}};
		hr = CoCreateInstance(CLSID_RtspStream, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void**)&(pRtspStream.p) );
		if( FAILED(hr))
		{
			DbgMsg(L"Failed to create instance of rtspstream");
			return hr;
		}
		else
		{
			hr = pRtspStream->QueryInterface(IID_IFileSourceFilter, (void**)&(pSrcFilter.p));
			if( FAILED(hr))
			{
				DbgMsg(L"Failed to query file source filter");
				return hr;
			}
			hr = pSrcFilter->Load(wcPath, NULL);
			if( FAILED(hr))
			{
				pSrcFilter = NULL;
				DbgMsg(L"Failed to load stream");
				return hr;
			}

			pSrcFilter = NULL;

			hr = m_pGraph->AddFilter( pRtspStream, L"RTSP STREAM Filter");
			if( FAILED(hr))
			{
				DbgMsg(L"Failed to add rtspstream to the graph");
				return hr;
			}
		}


		// try to render media source
		try{

            hr = AddRenderFilter(m_pGraph, pWizard, (IBaseFilter**)&(m_pRender.p));
		    if( FAILED(hr))
		    {
			    DbgMsg(_T("Failed to add Render filter"));
			    throw hr;
		    }            
            
            hr = m_pGraph->QueryInterface( IID_IGraphBuilder, (void**)&(pGb.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot get IGraphBuilder from the filter graph"));
				throw hr;
			}

			CComPtr<IEnumPins> pEnum;
			CComPtr<IPin>	  pPin;
			hr = pRtspStream->EnumPins(&pEnum);
			if( FAILED(hr))
			{
				DbgMsg(_T("Failed to enum pins for source filter"));
				throw hr;
			}
            pEnum->Reset();
			while(pEnum->Next(1, &pPin, 0) == S_OK)
			{
				PIN_INFO myPinInfo;
				pPin->QueryPinInfo(&myPinInfo);

				if(myPinInfo.dir == PINDIR_OUTPUT)
				{
					hr = pGb->Render(pPin);
				}
                // Make sure to release IBaseFilter interface
                QueryPinInfoReleaseFilter(myPinInfo);

                pPin = NULL;
			}
            pEnum = NULL;

#ifdef OPTIONAL_AUDIO
			if( FAILED(hr))
			{
				DbgMsg(_T("Failed to render specified media file"));
				throw hr;
			}

			hr = CheckConnection();
			if( FAILED(hr))
			{
				DbgMsg(_T("Application does not support this media type.\r\nTry some other media source"));
				throw hr;
			}
#endif // OPTIONAL_AUDIO

            // Fix video and audio lag
            CComQIPtr<IMediaFilter> mediaFilter(m_pGraph);
            mediaFilter->SetSyncSource(NULL);
#ifdef _DEBUG
            SaveGraphFile(pGb, _T("c:\\test.grf"));
#endif
			// ok, all is rendered, now get MediaControl, MediaSeeking and continue
			hr = m_pGraph->QueryInterface( IID_IMediaControl, (void**)&(m_pMc.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot find IMediaControl interface"));
				throw hr;
			}

			hr = m_pGraph->QueryInterface( IID_IMediaSeeking, (void**)&(m_pMs.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot find IMediaSeeking interface"));
				throw hr;
			}

#ifdef OPTIONAL_AUDIO
            hr = m_pGraph->QueryInterface( IID_IBasicAudio, (void**)&(m_pBa.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot find IBasicAudio interface"));
 				throw hr;
			}
#endif
		}
		catch(HRESULT hr1)
		{
			pWizard->Detach( m_dwID );
			m_dwID = 0;
			return hr1;
		}
	}
    else  if(UrlPrefix.CompareNoCase(_T("mms")) == 0 ||
		UrlPrefix.CompareNoCase(_T("http")) == 0)
	{
		m_bStream = TRUE;
		// create graph
		hr = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
			IID_IFilterGraph, (void**)&(m_pGraph.p) );
		if( FAILED(hr))
		{
			DbgMsg(_T("Failed to create the filter graph"));
			return hr;
		}

		CComPtr<IFileSourceFilter> pSrcFilter;
		CComPtr<IBaseFilter>    pMMSStream;


        AddDecoderFilter();



		const IID CLSID_MMSStream = {0x187463A0, 0x5BB7, 0x11D3, {0xAC, 0xBE, 0x00, 0x80, 0xC7, 0x5E,0x24, 0x6E}};
		hr = CoCreateInstance(CLSID_MMSStream, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void**)&(pMMSStream.p) );
		if( FAILED(hr))
		{
			DbgMsg(L"Failed to create instance of rtspstream");
			return hr;
		}
		else
		{
			hr = pMMSStream->QueryInterface(IID_IFileSourceFilter, (void**)&(pSrcFilter.p));
			if( FAILED(hr))
			{
				DbgMsg(L"Failed to query file source filter");
				return hr;
			}
			hr = pSrcFilter->Load(wcPath, NULL);
			if( FAILED(hr))
			{
				pSrcFilter = NULL;
				DbgMsg(L"Failed to load stream");
				return hr;
			}

			pSrcFilter = NULL;

			hr = m_pGraph->AddFilter( pMMSStream, L"MMS STREAM Filter");
			if( FAILED(hr))
			{
				DbgMsg(L"Failed to add rtspstream to the graph");
				return hr;
			}
		}


        hr = AddRenderFilter(m_pGraph, pWizard, (IBaseFilter**)&(m_pRender.p));
		if( FAILED(hr))
		{
			DbgMsg(_T("Failed to add Render filter"));
			throw hr;
		}

		// try to render media source
		try{
			hr = m_pGraph->QueryInterface( IID_IGraphBuilder, (void**)&(pGb.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot get IGraphBuilder from the filter graph"));
				throw hr;
			}

			CComPtr<IEnumPins> pEnum;
			CComPtr<IPin>	  pPin;
			hr = pMMSStream->EnumPins(&pEnum);
			if( FAILED(hr))
			{
				DbgMsg(_T("Failed to enum pins for source filter"));
				throw hr;
			}
            pEnum->Reset();
			while(pEnum->Next(1, &pPin, 0) == S_OK)
			{
				PIN_INFO myPinInfo;
				pPin->QueryPinInfo(&myPinInfo);

				if(myPinInfo.dir == PINDIR_OUTPUT)
				{
				    hr = pGb->Render(pPin);
				}
                // Make sure to release IBaseFilter interface
                QueryPinInfoReleaseFilter(myPinInfo);

                pPin = NULL;
			}
            pEnum = NULL;

			if( FAILED(hr))
			{
				DbgMsg(_T("Failed to render specified media file"));
				throw hr;
			}

			hr = CheckConnection();
			if( FAILED(hr))
			{
				DbgMsg(_T("Application does not support this media type.\r\nTry some other media source"));
				throw hr;
			}

			// ok, all is rendered, now get MediaControl, MediaSeeking and continue
			hr = m_pGraph->QueryInterface( IID_IMediaControl, (void**)&(m_pMc.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot find IMediaControl interface"));
				throw hr;
			}

			hr = m_pGraph->QueryInterface( IID_IMediaSeeking, (void**)&(m_pMs.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot find IMediaSeeking interface"));
				throw hr;
			}

			hr = m_pGraph->QueryInterface( IID_IBasicAudio, (void**)&(m_pBa.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot find IBasicAudio interface"));
				throw hr;
			}
		}
		catch(HRESULT hr1)
		{
			pWizard->Detach( m_dwID );
			m_dwID = 0;
			return hr1;
		}
    }
	else
	{
		m_bStream = FALSE;
		// first, check that file exists
		if( INVALID_FILE_ATTRIBUTES == GetFileAttributes( m_achPath))
		{
			DbgMsg(_T("Requested media file was not found"));
			return VFW_E_NOT_FOUND;
		}

		// create graph
		hr = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
			IID_IFilterGraph, (void**)&(m_pGraph.p) );
		if( FAILED(hr))
		{
			DbgMsg(_T("Failed to create the filter graph"));
			return hr;
		}

        AddDecoderFilter();

        if(UrlExtension.CompareNoCase(_T("mp4")) == 0)
        {
            CComPtr<IBaseFilter>    pSourceFilter;
		    hr = CoCreateInstance(CLSID_PDVD9MP4Spliter, NULL, CLSCTX_INPROC_SERVER,
			    IID_IBaseFilter, (void**)&(pSourceFilter.p) );

            if(SUCCEEDED(hr))
            {
			    hr = m_pGraph->AddFilter( pSourceFilter, L"MP4 Spliter");
			    if( FAILED(hr))
			    {
				    DbgMsg(_T("Failed to add Decoder Filter to the graph"));
                }
    				
            }
        }

        hr = AddRenderFilter(m_pGraph, pWizard, (IBaseFilter**)&(m_pRender.p));
		if( FAILED(hr))
		{
			DbgMsg(_T("Failed to add Render filter"));
			throw hr;
		}

		// try to render media source
		try{
			hr = m_pGraph->QueryInterface( IID_IGraphBuilder, (void**)&(pGb.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot get IGraphBuilder from the filter graph"));
				throw hr;
			}


			hr = pGb->RenderFile( m_wcPath, NULL);
			if( FAILED(hr))
			{
				DbgMsg(_T("Failed to render specified media file"));
				throw hr;
			}

			hr = CheckConnection();
			if( FAILED(hr))
			{
				DbgMsg(_T("Application does not support this media type.\r\nTry some other media source"));
				throw hr;
			}

			// ok, all is rendered, now get MediaControl, MediaSeeking and continue
			hr = m_pGraph->QueryInterface( IID_IMediaControl, (void**)&(m_pMc.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot find IMediaControl interface"));
				throw hr;
			}

			hr = m_pGraph->QueryInterface( IID_IMediaSeeking, (void**)&(m_pMs.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot find IMediaSeeking interface"));
				throw hr;
			}

			hr = m_pGraph->QueryInterface( IID_IBasicAudio, (void**)&(m_pBa.p) );
			if( FAILED(hr))
			{
				DbgMsg(_T("Cannot find IBasicAudio interface"));
				throw hr;
			}
		}
		catch(HRESULT hr1)
		{
			pWizard->Detach( m_dwID );
			m_dwID = 0;
			return hr1;
		}
	}



    return hr;
}

HRESULT S3DShowSubGraph::AddDecoderFilter()
{
    HRESULT hr = S_OK;


    if(theApp.GetRenderEngine()->IsEVREnabled())
    {
/*
        {
            CComPtr<IBaseFilter>    pDecoderFilter;
		    hr = CoCreateInstance(CLSID_VTDecoder, NULL, CLSCTX_INPROC_SERVER,
			    IID_IBaseFilter, (void**)&(pDecoderFilter.p) );

            if(SUCCEEDED(hr))
            {
			    hr = m_pGraph->AddFilter( pDecoderFilter, L"Video Decoder MPEG4");
			    if( FAILED(hr))
			    {
				    DbgMsg(_T("Failed to add Decoder Filter to the graph"));
                }
    				
            }
        }
*/

        CComPtr<IBaseFilter>    pDecoderFilter;

		hr = CoCreateInstance(CLSID_DXVADShowDecoder, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void**)&(pDecoderFilter.p) );

        if(FAILED(hr))
        {
		    hr = CoCreateInstance(CLSID_FFDShowDecoder, NULL, CLSCTX_INPROC_SERVER,
			    IID_IBaseFilter, (void**)&(pDecoderFilter.p) );
        }

        if(SUCCEEDED(hr))
        {
			hr = m_pGraph->AddFilter( pDecoderFilter, L"Video Decoder");
			if( FAILED(hr))
			{
				DbgMsg(_T("Failed to add Decoder Filter to the graph"));
            }
				
        }
    }
    else
    {
        CComPtr<IBaseFilter>    pDecoderFilter;
		hr = CoCreateInstance(CLSID_FFDShowDecoder, NULL, CLSCTX_INPROC_SERVER,
			IID_IBaseFilter, (void**)&(pDecoderFilter.p) );

        if(SUCCEEDED(hr))
        {
			hr = m_pGraph->AddFilter( pDecoderFilter, L"Video Decoder");
			if( FAILED(hr))
			{
				DbgMsg(_T("Failed to add Decoder Filter to the graph"));
            }
				
        }
    }




    return S_OK;
}


/******************************Public*Routine******************************\
* CheckVMRConnection
\**************************************************************************/
HRESULT S3DShowSubGraph::CheckConnection()
{
    HRESULT hr = S_OK;
    bool bConnected = false;

    CComPtr<IEnumPins> pEnum;
    CComPtr<IPin> pPin;

    if( !m_pRender )
        return E_UNEXPECTED;

    hr = m_pRender->EnumPins( &pEnum );
    if( FAILED(hr))
        return hr;

    hr = pEnum->Next( 1, &pPin, NULL);
    while( SUCCEEDED(hr) && pPin)
    {
        CComPtr<IPin> pConnectedPin;
        hr = pPin->ConnectedTo( &pConnectedPin );

        if( SUCCEEDED(hr) && pConnectedPin )
        {
            bConnected = true;
            break;
        }

        pPin = NULL;
        hr = pEnum->Next( 1, &pPin, NULL);
    }// while

    hr = (true == bConnected) ? S_OK : E_FAIL;
    return hr;
}

HRESULT S3DShowSubGraph::Seek(LONGLONG SeekPosition)
{
    HRESULT hr = S_OK;

	if(m_bStream)
	{
		return S_FALSE;
	}

    if(!m_pMs)
    {
        return E_UNEXPECTED;
    }
    hr = m_pMs->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);

    hr = m_pMs->SetPositions(&SeekPosition, AM_SEEKING_AbsolutePositioning, 
                        NULL, AM_SEEKING_NoPositioning);
    return hr;
}


HRESULT S3DShowSubGraph::GetPosition(LONGLONG *pPosition)
{
    HRESULT hr = S_OK;

	if(m_bStream)
	{
		*pPosition = 0;
		return S_FALSE;
	}

    if(!m_pMs)
    {
        return E_UNEXPECTED;
    }
    hr = m_pMs->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);

    LONGLONG StopPosition;
    hr = m_pMs->GetPositions(pPosition, &StopPosition);

    return hr;

}


/******************************Public*Routine******************************\
* Run
\**************************************************************************/
HRESULT S3DShowSubGraph::Run(ULONG volume)
{
    HRESULT hr = S_OK;
    if( !m_pMc || !m_pMs)
    {
        return E_UNEXPECTED;
    }
    if(m_pBa)
    {
        long setVolume;
        if(volume>=100)
        {
            setVolume = 0;
        }
        else if(volume==0)
        {
            setVolume = -10000;
        }
        else
        {
            setVolume = (long)(1000.0f*log(volume/100.0f));
        }
        hr = m_pBa->put_Volume(setVolume);
    }

    hr = m_pMc->Run();

    return hr;
}

HRESULT S3DShowSubGraph::Resume()
{
    HRESULT hr = S_OK;
    if( !m_pMc || !m_pMs)
    {
        return E_UNEXPECTED;
    }

    hr = m_pMc->Run();

    return hr;
}

HRESULT S3DShowSubGraph::Pause()
{
    HRESULT hr = S_OK;
    if( !m_pMc || !m_pMs)
    {
        return E_UNEXPECTED;
    }

    hr = m_pMc->Pause();

    return hr;
}

/******************************Public*Routine******************************\
* Stop
\**************************************************************************/
HRESULT S3DShowSubGraph::Stop()
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

void S3DShowSubGraph::SetRepeatMode(S3S_PLAYSETTING mode)
{
    m_RepeatMode = mode;
}

/******************************Public*Routine******************************\
* GetState
*
* Returns OAFilterState from IMediaControl of the graph
*
* Return values: errors from the filter graph and wizard
\**************************************************************************/
OAFilterState S3DShowSubGraph::GetState()
{
    OAFilterState state = State_Stopped;
    if( m_pMc )
    {
        HRESULT hr = m_pMc->GetState( 20, &state );
    }
    return state;
}

/******************************Public*Routine******************************\
* AutoLoop
*
\**************************************************************************/

HRESULT S3DShowSubGraph::AutoLoop()
{
    HRESULT hr = S_OK;
	
	if(m_bStream)
	{
		return S_OK;
	}

    LONGLONG llCur;
    LONGLONG llDur;
    if( !m_pMs )
        return E_FAIL;

    hr = m_pMs->GetPositions( &llCur, &llDur );

    // 133ms
    if( llDur - llCur < 1330000L )//Nearly Finished
    {
        llCur = 0;
        if(m_RepeatMode == S3S_PS_REPEAT)
        {
            m_pMs->SetPositions(&llCur, AM_SEEKING_AbsolutePositioning, 
                        NULL, AM_SEEKING_NoPositioning);
        }
    }
    return hr;
}


/******************************Public*Routine******************************\
* DestroyGraph
*
* Stops the graph, destroys and removes all the filters (VMR9 is removed last)
*
\**************************************************************************/
HRESULT S3DShowSubGraph::DestroyGraph()
{
    HRESULT hr = S_OK;
    OAFilterState state;

    if( !m_pGraph )
    {
        return E_POINTER;
    }

    FILTER_INFO fi;
    CComPtr<IMediaControl> pMc;
    CComPtr<IEnumFilters> pEnum;
    CComPtr<IBaseFilter> pFilter;
    CComPtr<IBaseFilter> pRender = NULL;

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
    pEnum->Reset();
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

        if( 0 == wcscmp( fi.achName, L"Render"))
        {
            pRender = pFilter;
        }
        hr = m_pGraph->RemoveFilter( pFilter);
        pFilter = NULL;

        hr = pEnum->Reset();
        hr = pEnum->Next(1, &pFilter, NULL);
    }

    pFilter = NULL;
    pEnum = NULL;
    pRender = NULL;

    m_pMc = NULL;
    m_pMs = NULL;
    m_pBa = NULL;

    return S_OK;
}

/******************************Public*Routine******************************\
* DisconnectPins
*
* Disconnects pins of a filter
*
\**************************************************************************/
HRESULT S3DShowSubGraph::DisconnectPins( CComPtr<IBaseFilter> pFilter)
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

    pEnum->Reset();
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

