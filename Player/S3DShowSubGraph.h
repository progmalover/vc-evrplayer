#pragma once
#include "S3DShowWizard.h"
#include "S3SignageSetting.h"

enum DECODE_FILTER_SELECT
{
    DECODE_FILTER_DEFAULT,
    DECODE_FILTER_FFDSHOW,
    DECODE_FILTER_S3DXVA,
    DECODE_FILTER_PDVD8,
};

class S3DShowSubGraph
{
public:
    S3DShowSubGraph(void);
    virtual ~S3DShowSubGraph(void);

    virtual HRESULT BuildAndRender( WCHAR* wcPath, S3DShowWizard* pWizard, HWND hWnd);
    virtual HRESULT Seek(LONGLONG SeekPosition);
    virtual HRESULT GetPosition(LONGLONG *pPosition);
    virtual HRESULT Run(ULONG volume);
    virtual HRESULT Stop();
    virtual HRESULT Pause();
    virtual HRESULT Resume();
    virtual HRESULT DestroyGraph();
    virtual HRESULT DisconnectPins( CComPtr<IBaseFilter> pFilter);
    virtual HRESULT AutoLoop();
    virtual void    SetRepeatMode(S3S_PLAYSETTING mode);
    virtual DWORD_PTR GetID(){ return m_dwID;};
    virtual OAFilterState GetState();

    virtual HRESULT AddRenderFilter(IFilterGraph *pGraph, S3DShowWizard* pWizard, IBaseFilter **ppRenderFilter) = 0;

    virtual HRESULT AddDecoderFilter();
protected:
    // protected members
    S3S_PLAYSETTING         m_RepeatMode;

	BOOL					m_bStream;

    // private data
    CComPtr<IFilterGraph>   m_pGraph;   // filter graph
    CComPtr<IBaseFilter>    m_pRender;  // RenderFile
    CComPtr<IMediaControl>  m_pMc;      // media control
    CComPtr<IMediaSeeking>  m_pMs;      // media seeking
    CComPtr<IBasicAudio>    m_pBa;

    WCHAR m_wcPath[MAX_PATH];   // path to the media file, wide char
    TCHAR m_achPath[MAX_PATH];  // path to the media file, TCHAR

    DWORD_PTR m_dwID;   // actual cookie identifying the subgraph; assigned in
                        // IS3VRWizard::Attach
    HWND      m_hWnd;

    HRESULT CheckConnection();
};
