#include "stdafx.h"
#include "S3DShowWizard.h"
#include "S3RenderEngine.h"
#include "S3RenderMixer.h"
#include "S3VMRSubgraph.h"
#include "S3Signage.h"


/******************************Public*Routine******************************\
* S3VMRSubgraph
*
* constructor
\**************************************************************************/

S3VMRSubgraph::S3VMRSubgraph()
    :S3DShowSubGraph()
{

}

/******************************Public*Routine******************************\
* S3VMRSubgraph
*
* destructor
\**************************************************************************/
S3VMRSubgraph::~S3VMRSubgraph()
{
}


HRESULT S3VMRSubgraph::AddRenderFilter(IFilterGraph *pGraph, S3DShowWizard* pWizard, IBaseFilter **ppRenderFilter)
{
    HRESULT hr;
	IBaseFilter *pRenderFilter = NULL;
    CComPtr<IVMRFilterConfig9> pConfig;

	hr = CoCreateInstance( CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&(pRenderFilter) );
	if( FAILED(hr))
	{
		DbgMsg(_T("Failed to create instance of VMR9"));
		return hr;
	}

	hr = m_pGraph->AddFilter( pRenderFilter, L"Render");
	if( FAILED(hr))
	{
		DbgMsg(_T("Failed to add VMR9 to the graph"));
		return hr;
	}
	// configure VMR9
	hr = pRenderFilter->QueryInterface( IID_IVMRFilterConfig9, (void**)&(pConfig.p) );
	if( FAILED(hr))
	{
		DbgMsg(_T("Cannot get IVMRFilterConfig9 from VMR9"));
		return hr;
	}
    
	// if wizard is provided, set VMR to the renderless code and attach to the wizard
	if( pWizard )
	{
		// set VMR to the renderless mode
		hr = pConfig->SetRenderingMode( VMR9Mode_Renderless );
		if( FAILED(hr))
		{
			DbgMsg(_T("Failed to set VMR9 to the renderless mode"));
			return hr;
		}

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