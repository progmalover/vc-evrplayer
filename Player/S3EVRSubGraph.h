#ifndef EVRSUBGRAPH_HEADER
#define EVRSUBGRAPH_HEADER

#pragma once
#include "S3SignageSetting.h"
#include "S3DShowSubgraph.h"
#include "EVR.h"


class S3EVRSubGraph: public S3DShowSubGraph
{
public:
    S3EVRSubGraph();
    virtual ~S3EVRSubGraph();

    // public methods
    virtual HRESULT AddRenderFilter(IFilterGraph *pGraph, S3DShowWizard* pWizard, IBaseFilter **ppRenderFilter);
};

#endif
