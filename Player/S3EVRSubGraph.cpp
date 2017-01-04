#include "stdafx.h"
#include "S3DShowWizard.h"
#include "S3RenderEngine.h"
#include "S3RenderMixer.h"
#include "S3EVRSubGraph.h"
#include "S3Signage.h"
#include "EVRPresenterInclude.h"

#define DIABLE_AUDIO_FOR_RTSP 0 
//By Howe, just for test ,can not be enabled as it will cause source filter can not be released.

/******************************Public*Routine******************************\
* S3EVRSubGraph
*
* constructor
\**************************************************************************/

S3EVRSubGraph::S3EVRSubGraph()
    :S3DShowSubGraph()

{
}

/******************************Public*Routine******************************\
* S3EVRSubGraph
*
* destructor
\**************************************************************************/
S3EVRSubGraph::~S3EVRSubGraph()
{
}


HRESULT S3EVRSubGraph::AddRenderFilter(IFilterGraph *pGraph, S3DShowWizard* pWizard, IBaseFilter **ppRenderFilter)
{
	IBaseFilter *pRenderFilter = NULL;
    HRESULT hr;

	// create and add EVR
	hr = CoCreateInstance( CLSID_EnhancedVideoRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&(pRenderFilter) );
	if( FAILED(hr))
	{
		DbgMsg(_T("Failed to create instance of EVR"));
		return hr;
    }



	hr = m_pGraph->AddFilter( pRenderFilter, L"Render");
	if( FAILED(hr))
	{
		DbgMsg(_T("Failed to add EVR to the graph"));
		return hr;
	}

	// if wizard is provided, set VMR to the renderless code and attach to the wizard
	if( pWizard )
	{

		hr = pWizard->Attach( pRenderFilter, &m_dwID );
		if( FAILED(hr))
		{
			DbgMsg(_T("Failed to attach graph to the wizard"));
			return hr;
		}
	}

    *ppRenderFilter = pRenderFilter;
    return hr;
}