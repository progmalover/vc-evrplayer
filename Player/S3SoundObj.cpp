#include "stdafx.h"
#include "S3Signage.h"
#include "S3SoundObj.h"
#include "AttributeParser.h"

#define S3MO_TRACE_PERF 1

S3SoundObj::S3SoundObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute)
:S3RenderableObject(iWidth, iHeight, fScaleRate, Attribute)
{
    m_bInited = FALSE;
    m_pGraph = NULL;
    m_pMc = NULL;
    m_pMs = NULL;
    m_pBa = NULL;

    m_FilePath = GetStringAttrib(m_Attribute, _T("Filename"), _T(""));
    m_Volume = GetIntAttrib(m_Attribute, _T("Volume"), 100);


    if( INVALID_FILE_ATTRIBUTES == GetFileAttributes( m_FilePath))
    {
        return;
    }

    HRESULT hr;
    CComPtr<IGraphBuilder>  pGb;
        // create graph
    hr = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
        IID_IFilterGraph, (void**)&(m_pGraph.p) );
    if( FAILED(hr))
    {
        DbgMsg(TEXT("Failed to create the filter graph"));
        return;
    }

    // try to render media source
    hr = m_pGraph->QueryInterface( IID_IGraphBuilder, (void**)&(pGb.p) );
    if( FAILED(hr))
    {
        DbgMsg(TEXT("Cannot get IGraphBuilder from the filter graph"));
        return;
    }

    hr = pGb->RenderFile( m_FilePath, NULL);
    if( FAILED(hr))
    {
        DbgMsg(TEXT("Failed to render specified media file"));
        return;
    }

    // ok, all is rendered, now get MediaControl, MediaSeeking and continue
    hr = m_pGraph->QueryInterface( IID_IMediaControl, (void**)&(m_pMc.p) );
    if( FAILED(hr))
    {
        DbgMsg(TEXT("Cannot find IMediaControl interface"));
        return;
    }

    hr = m_pGraph->QueryInterface( IID_IMediaSeeking, (void**)&(m_pMs.p) );
    if( FAILED(hr))
    {
        DbgMsg(TEXT("Cannot find IMediaSeeking interface"));
        return;
    }

    hr = m_pGraph->QueryInterface( IID_IBasicAudio, (void**)&(m_pBa.p) );
    if( FAILED(hr))
    {
        DbgMsg(TEXT("Cannot find IBasicAudio interface"));
        return;
    }

    m_bInited = TRUE;
}

S3SoundObj::~S3SoundObj()
{
}

HRESULT S3SoundObj::PrepareRender()
{
    if(!m_bInited)
    {
        return S_FALSE;
    }
    return S_FALSE;
}

HRESULT S3SoundObj::EndRender()
{
    if(!m_bInited)
    {
        return S_FALSE;
    }
    return S_OK;
}

HRESULT S3SoundObj::ProcessMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return S_OK;
}

HRESULT S3SoundObj::Start()
{
    if(!m_bInited)
    {
        return S_FALSE;
    }
    HRESULT hr = S_OK;
    if( !m_pMc )
    {
        return E_UNEXPECTED;
    }
    if(m_pBa)
    {
        long setVolume;
        if(m_Volume>=100)
        {
            setVolume = 0;
        }
        else if(m_Volume==0)
        {
            setVolume = -10000;
        }
        else
        {
            setVolume = (long)(1000.0f*log(m_Volume/100.0f));
        }
        m_pBa->put_Volume(setVolume);
    }
    LONGLONG llCur = 0L;
    m_pMs->SetPositions(&llCur, AM_SEEKING_AbsolutePositioning, 
                        NULL, AM_SEEKING_NoPositioning);
    hr = m_pMc->Run();

    return hr;
}

HRESULT S3SoundObj::Stop()
{
    HRESULT hr = S_OK;

    if( !m_pMc )
    {
        return E_UNEXPECTED;
    }
    hr = m_pMc->Stop();

    return hr;
}

HRESULT S3SoundObj::LoopContent()
{
    HRESULT hr = S_OK;
	
    LONGLONG llCur;
    LONGLONG llDur;
    if( !m_pMs )
        return E_FAIL;

    hr = m_pMs->GetPositions( &llCur, &llDur );

    // 100ms
    if( llDur - llCur < 1000000L )//Nearly Finished
    {
        llCur = 0;
        hr = m_pMs->SetPositions(&llCur, AM_SEEKING_AbsolutePositioning, 
                    NULL, AM_SEEKING_NoPositioning);
    }
    return hr;
}


HRESULT S3SoundObj::Pause()
{
    if( !m_pMc )
    {
        return E_UNEXPECTED;
    }
    return m_pMc->Pause();
}

HRESULT S3SoundObj::Resume()
{
    if( !m_pMc )
    {
        return E_UNEXPECTED;
    }

    return m_pMc->Run();;
}