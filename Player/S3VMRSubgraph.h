#ifndef VMR9SUBGRAPH_HEADER
#define VMR9SUBGRAPH_HEADER

#pragma once
#include "S3DShowSubgraph.h"

class S3VMRWizard;
// S3VMRSubgraph

// This class represents a subgraph with VMR9 to be attached to the wizard
// This sample supports only media files, but any DirectShow filter graph
// with VMR9 can be used.

class S3VMRSubgraph: public S3DShowSubGraph
{
public:
    S3VMRSubgraph();
    virtual ~S3VMRSubgraph();

    virtual HRESULT AddRenderFilter(IFilterGraph *pGraph, S3DShowWizard* pWizard, IBaseFilter **ppRenderFilter);

};

#endif

