#include "stdafx.h"
#include "S3Signage.h"
#include "S3RenderMixer.h"
#include "S3RenderEngine.h"
#include "S3RenderScheduler.h"
#include <tinyxml/tinyxml.h>
#include "AttributeParser.h"


S3RenderScheduler::S3RenderScheduler(S3RenderMixer *pMixer)
    :m_pMixer(pMixer)
{
    m_Canvas.pBackgroundObj = NULL;
    m_bFitScreen = FALSE;
	//m_Canvas.Width = ::GetSystemMetrics(SM_CXSCREEN);
	//m_Canvas.Height = ::GetSystemMetrics(SM_CYSCREEN);
	//m_Canvas.Name = _T("C1");
	//m_Canvas.BGColor = 0;

    m_bPaused = FALSE;
    m_MediaLibraryPath = _T("");
    m_hMixer = NULL;
    amdInitialize();
    amdGetMasterVolumeControl();
}


S3RenderScheduler::~S3RenderScheduler(void)
{
    Stop();
    Terminate();
    amdUninitialize();
}


HRESULT S3RenderScheduler::BeginDeviceLoss()
{

    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;

    Pause();

    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
            S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if( pContent )
                {
                    if(pContent->pObject)
                    {
                        pContent->pObject->InvalidateDeviceObjects();
                    }
                }
            }
        }
    }

    if(m_Canvas.pBackgroundObj)
    {
        m_Canvas.pBackgroundObj->InvalidateDeviceObjects();
    }
    return S_OK;
}
        
HRESULT S3RenderScheduler::EndDeviceLoss(IDirect3DDevice9 *pDevice)
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;


    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
            S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if( pContent )
                {
                    if(pContent->pObject)
                    {
                        pContent->pObject->RestoreDeviceObjects(pDevice);
                    }
                }
            }
        }
    }

    if(m_Canvas.pBackgroundObj)
    {
        m_Canvas.pBackgroundObj->RestoreDeviceObjects(pDevice);
    }

    Resume();
    return S_OK;
}


HRESULT S3RenderScheduler::InitializeScheduler(int nFPS, CString MediaLibraryPath)
{
    m_Width = 0;
    m_Height = 0;


    m_Scale = 1.0f;
    m_XTrans = 0.0f;
    m_YTrans = 0.0f;

    m_nFPS = nFPS;

    m_MediaLibraryPath = MediaLibraryPath + _T("\\");
    return S_OK;
}

HRESULT S3RenderScheduler::SetVolume(int Vol)
{
    int VolValue = Vol * m_dwMaximum /100;
    amdSetMasterVolumeValue(VolValue);
    return S_OK;
}


HRESULT S3RenderScheduler::SetDisplayConfigure(int Width, int Height, FLOAT RotateDegree, BOOL bFitScreen, BOOL bFillScreen)
{
    m_Width = Width;
    m_Height = Height;
    m_RotationDegree = RotateDegree;
    m_bFitScreen = bFitScreen;      
    m_bFillScreen = bFillScreen;

    return S_OK;
}

VOID S3RenderScheduler::CalculateTransform()
{
    if(m_Width == 0 || m_Height == 0)
    {
        m_Scale = 1.0f;
        m_XTrans = 0.0f;
        m_YTrans = 0.0f;
        return;
    }

    int LogicWidth = m_Canvas.Width;
    int LogicHeight = m_Canvas.Height;


    if((FLOAT)LogicWidth / (FLOAT)LogicHeight > (FLOAT)m_Width / (FLOAT)m_Height)
    {
        m_Scale = (FLOAT)m_Width / (FLOAT)LogicWidth;
    }else
    {
        m_Scale = (FLOAT)m_Height / (FLOAT)LogicHeight;
    }

    if (m_bFitScreen)
    {   
        if (!m_bFillScreen)
        {                                   
            SIZEL FitSize;
            FitSize.cx = m_Width;
            FitSize.cy = m_Height;

            RECTF RotateRect;    
            RotateRect.left = (m_Width - m_Scale * LogicWidth) / 2 + 0.5f;
            RotateRect.right = RotateRect.left + LogicWidth * m_Scale;     
            RotateRect.top = (m_Height - m_Scale * LogicHeight) / 2 + 0.5f; 
            RotateRect.bottom = RotateRect.top + LogicHeight * m_Scale;     

            FLOAT fRotateScale = GetRotatedScaleOnFitScreen(-m_RotationDegree, &RotateRect, &FitSize);  
            m_Scale *= fRotateScale;
        }
        else
        {
            SIZEL FitSize;
            FitSize.cx = (LONG)(LogicWidth);
            FitSize.cy = (LONG)(LogicHeight);

            RECTF RotateRect;    
            RotateRect.left = ((LogicWidth - m_Width) / 2);
            RotateRect.right = RotateRect.left + m_Width;     
            RotateRect.top = ( (LogicHeight - m_Height) / 2); 
            RotateRect.bottom = RotateRect.top + m_Height;

            m_Scale = GetRotatedScaleOnReverseFitScreen(-m_RotationDegree, &RotateRect, &FitSize); 
        }

    }

    m_XTrans = (m_Width - m_Scale * LogicWidth) / 2;
    m_YTrans = (m_Height - m_Scale * LogicHeight) / 2;


}


RECT S3RenderScheduler::GetDisplayRect()
{
    RECT DisplayRect;
    DisplayRect.left = (int)(m_XTrans + 0.5f);
    DisplayRect.top = (int)(m_YTrans + 0.5f);
    DisplayRect.right = (int)(m_Canvas.Width * m_Scale + m_XTrans + 0.5f);
    DisplayRect.bottom = (int)(m_Canvas.Height * m_Scale + m_YTrans + 0.5f);
    return DisplayRect;
}

SIZE    S3RenderScheduler::GetCanvasSize()
{
    SIZE CanvasSize;
    CanvasSize.cx = m_Canvas.Width;
    CanvasSize.cy = m_Canvas.Height;
    return CanvasSize;
}
HRESULT S3RenderScheduler::Add(CString ParentID, CString Desc)
{
    S3SIGNAGE_LAYER *pLayer = NULL;
    S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;
	
	if(!FindID(ParentID, &pLayer, &pContent)) return E_UNEXPECTED;


	TiXmlDocument *pDoc = new TiXmlDocument();

	pDoc->Parse(CW2A(Desc, CP_UTF8), NULL, TIXML_ENCODING_UTF8);


	if(pLayer == NULL)
	{
		TiXmlElement* pLayerElement = pDoc->RootElement(); 

		if(pLayerElement == NULL) return E_UNEXPECTED;

		S3SIGNAGE_LAYER *pLayer = new S3SIGNAGE_LAYER;

		ParserLayer(pLayer, pLayerElement);

		pLayer->StartTime = timeGetTime();

		S3SIGNAGE_LAYER *OldpLayer = NULL;
		
		Delete(pLayer->Name);

        m_Canvas.Layers.push_front(pLayer);

    }

    if(pLayer != NULL)
    {
        TiXmlElement* pContentElement = pDoc->RootElement(); 
        if(pContentElement == NULL) return E_UNEXPECTED;

        S3SIGNAGE_CONTENT_CONTAINER *pContent = new S3SIGNAGE_CONTENT_CONTAINER;

        ParserContent(pContent, pContentElement);
        pLayer->Contents.push_back(pContent);
    }

    delete pDoc;
    GenerateLayerZOrder();
    ResetAllContentZOrder();
    UpdateRenderObjects();
    return S_OK;
}

HRESULT S3RenderScheduler::Delete(CString ID)
{
    S3SIGNAGE_LAYER *pLayer = NULL;
    S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

    if(!FindID(ID, &pLayer, &pContent)) return E_UNEXPECTED;

    if(pLayer == NULL && pContent == NULL) return E_UNEXPECTED;


    if( pContent )
    {
        if(pContent->pObject)
        {
            m_pMixer->RemoveRenderableObject(pContent->pObject);
            pContent->pObject->DeleteDeviceObjects();
            delete pContent->pObject; 
            pContent->pObject = NULL;
            if(pContent->bRendering)
            {
                pLayer->VisibleCount --;
            }
        }
        if(pContent->pObjectForNextLoop)
        {
            pContent->pObjectForNextLoop->DeleteDeviceObjects();
            delete pContent->pObjectForNextLoop; 
            pContent->pObjectForNextLoop = NULL;
        }


        delete pContent;  
        pLayer->Contents.remove(pContent);
        return S_OK;
    }

    if(pLayer && pContent == NULL)
    {
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if(pContent->pObject)
                {
                    m_pMixer->RemoveRenderableObject(pContent->pObject);
                    pContent->pObject->DeleteDeviceObjects();
                    delete pContent->pObject; 
                    pContent->pObject = NULL;
                }
                if(pContent->pObjectForNextLoop)
                {
                    pContent->pObjectForNextLoop->DeleteDeviceObjects();
                    delete pContent->pObjectForNextLoop; 
                    pContent->pObjectForNextLoop = NULL;
                }
                delete pContent;
            }
            pLayer->Contents.clear();
            pLayer->VisibleCount = 0;
        }
		delete pLayer;
		m_Canvas.Layers.remove(pLayer);
    }


    return S_OK;
}

HRESULT S3RenderScheduler::Hide(CString ID)
{
    S3SIGNAGE_LAYER *pLayer = NULL;
    S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

    if(!FindID(ID, &pLayer, &pContent)) return E_UNEXPECTED;

    if(pLayer == NULL && pContent == NULL) return E_UNEXPECTED;

    if(pContent && pContent->bHided != TRUE)
    {
        pContent->bHided = TRUE;
        if(pContent->pObject != NULL)
        {
            m_pMixer->RemoveRenderableObject(pContent->pObject);
        }
        pContent->bRendering = FALSE;
        return S_OK;
    }

    if(pLayer && pContent == NULL)
    {
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if(pContent && pContent->bHided != TRUE)
                {
                    pContent->bHided = TRUE;
                    if(pContent->pObject != NULL)
                    {
                        m_pMixer->RemoveRenderableObject(pContent->pObject);
                    }
                    pContent->bRendering = FALSE;
                }
            }
        }
    }

    return S_OK;
}

HRESULT S3RenderScheduler::Show(CString ID)
{
    S3SIGNAGE_LAYER *pLayer = NULL;
    S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

    if(!FindID(ID, &pLayer, &pContent)) return E_UNEXPECTED;

    if(pLayer == NULL && pContent == NULL) return E_UNEXPECTED;

    if(pContent && pContent->bHided == TRUE)
    {
        pContent->bHided = FALSE;
        return S_OK;
    }

    if(pLayer && pContent == NULL)
    {
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if(pContent && pContent->bHided == TRUE)
                {
                    pContent->bHided = FALSE;
                }
            }
        }
    }

    return S_OK;
}

BOOL S3RenderScheduler::FindID(CString ContentID, S3SIGNAGE_LAYER **ppLayer, S3SIGNAGE_CONTENT_CONTAINER **ppContent)
{
	list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
	S3SIGNAGE_LAYER *pLayer = NULL;

	if(ContentID == _T("")) return FALSE;

	if(m_Canvas.Name.CompareNoCase(ContentID) == 0 )
	{
		*ppLayer = NULL;
		*ppContent = NULL;
		return TRUE;
	}


	LayerStart = m_Canvas.Layers.begin();
	LayerEnd = m_Canvas.Layers.end();
	for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
	{
		pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
		if(pLayer && pLayer->Name.CompareNoCase(ContentID) == 0)
		{
			*ppLayer = pLayer;
			*ppContent = NULL;
			return TRUE;
		}

		if( pLayer  && (pLayer->Contents.size() > 0))
		{
			list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
			S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

			ContentStart = pLayer->Contents.begin();
			ContentEnd = pLayer->Contents.end();

			for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
			{
				pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
				if( pContent && pContent->Content.IDName.CompareNoCase(ContentID) == 0)
				{
					*ppLayer = pLayer;
					*ppContent = pContent;
					return TRUE;
				}
			}
		}
	}


	return FALSE;
}

HRESULT S3RenderScheduler::ParserTranstion(S3SIGNAGE_TRANSIION *pTransition, TiXmlElement *pTransitonElement)
{
    TiXmlAttribute* pAttributeOfTransition = pTransitonElement->FirstAttribute();  

    pTransition->Duration = 0;
    pTransition->Name = _T("");
    pTransition->Direction = _T("");
    // parser canvas attribute
    while(pAttributeOfTransition)
    {
        CString ValueStr = CA2W(pAttributeOfTransition->Value(), CP_UTF8);
        CString NameStr = CA2W(pAttributeOfTransition->Name(), CP_UTF8);

        if(NameStr.CompareNoCase( _T("Type")) == 0)   pTransition->Name = ValueStr;
        if(NameStr.CompareNoCase( _T("Duration")) == 0)   pTransition->Duration = ConvertStr2Duration(ValueStr);
        if(NameStr.CompareNoCase( _T("Direction")) == 0)   pTransition->Direction = ValueStr;

        pAttributeOfTransition = pAttributeOfTransition->Next();
    }

    return S_OK;
}

HRESULT S3RenderScheduler::ParserTransform(S3SIGNAGE_TRANSFORM *pTransform, TiXmlElement *pTransformElement)
{
    TiXmlElement* pTransformValueElement = pTransformElement->FirstChildElement();  
        
    while(pTransformValueElement)
    {
        if(CString(_T("Value")).CompareNoCase(CA2W(pTransformValueElement->Value(), CP_UTF8)) != 0) continue;

        TRANSFORM_VALUE TransformValue;

        TransformValue.Time = 0;
        TransformValue.XValue = 0;
        TransformValue.YValue = 0;

        TiXmlAttribute* pAttributeOfTransform = pTransformValueElement->FirstAttribute();  

        // parser layer attribute
        while(pAttributeOfTransform)
        {
            CString ValueStr = CA2W(pAttributeOfTransform->Value(), CP_UTF8);
            CString NameStr = CA2W(pAttributeOfTransform->Name(), CP_UTF8);

            if(NameStr.CompareNoCase( _T("Time")) == 0)  TransformValue.Time = ConvertStr2Duration(ValueStr);
            if(NameStr.CompareNoCase( _T("XValue")) == 0) TransformValue.XValue = (float)_tstof(ValueStr);
            if(NameStr.CompareNoCase( _T("YValue")) == 0) TransformValue.YValue = (float)_tstof(ValueStr);
            if(NameStr.CompareNoCase( _T("Deg")) == 0) TransformValue.XValue = (float)_tstof(ValueStr);

            pAttributeOfTransform = pAttributeOfTransform->Next();
        }


        pTransform->push_back(TransformValue);

        pTransformValueElement = pTransformValueElement->NextSiblingElement();
    }

    return S_OK;
}


HRESULT S3RenderScheduler::ParserLayer(S3SIGNAGE_LAYER *pLayer, TiXmlElement *pLayerElement)
{
    TiXmlAttribute* pAttributeOfLayer = pLayerElement->FirstAttribute();  

    pLayer->VisibleCount = 0;
    pLayer->StartTime = 0x20000000;
    pLayer->Name = _T("");
    pLayer->Duration = 0x200000;

    // parser layer attribute
    while(pAttributeOfLayer)
    {
        CString ValueStr = CA2W(pAttributeOfLayer->Value(), CP_UTF8);
        CString NameStr = CA2W(pAttributeOfLayer->Name(), CP_UTF8);

        if(NameStr.CompareNoCase( _T("ID")) == 0)  pLayer->Name = ValueStr;
        if(NameStr.CompareNoCase( _T("Duration")) == 0)
        {
            pLayer->Duration = ConvertStr2Duration(ValueStr);
            if(pLayer->Duration == 0) pLayer->Duration = 0x200000;
        }

        pAttributeOfLayer = pAttributeOfLayer->Next();
    }

    TiXmlElement* pContentElement = pLayerElement->FirstChildElement(); 

    int LastEndTime = 0;
    BOOL bError = FALSE;

    while(pContentElement)
    {
        S3SIGNAGE_CONTENT_CONTAINER *pContent = new S3SIGNAGE_CONTENT_CONTAINER;

        ParserContent(pContent, pContentElement);

        if(pContent->Content.StartTime + pContent->Content.Duration > pLayer->Duration)
        {
            DbgMsg(_T("Warning: Content %s duration exceed layer %s duration"), pContent->Content.TypeName, pLayer->Name);
            pContent->Content.Duration = pLayer->Duration - pContent->Content.StartTime;
        }
        if(pContent->Content.StartTime >= pLayer->Duration)
        {
            DbgMsg(_T("Error: Content %s start time exceed layer %s duration"), pContent->Content.TypeName, pLayer->Name);
            bError = TRUE;
        }
        if(pContent->Content.StartTime < LastEndTime)
        {
            DbgMsg(_T("Error: Content %s time range overlap in layer %s "), pContent->Content.TypeName, pLayer->Name);
            bError = TRUE;
        }

        if (bError)
        {
            delete pContent;
            pContent = NULL;
            bError = FALSE;
        }

        if(pContent)
        {
            LastEndTime = pContent->Content.StartTime + pContent->Content.Duration;
            pLayer->Contents.push_back(pContent); 
        }

        pContentElement = pContentElement->NextSiblingElement();
    }

    return S_OK;
}

HRESULT S3RenderScheduler::ParserTextFile(TextFileItem *pTextFile, TiXmlElement *pTextFileElement)
{
    TiXmlAttribute* pAttributeOfTextFile = pTextFileElement->FirstAttribute();  

    pTextFile->bResourceFile = FALSE;
    pTextFile->ResourceID = 0;
    pTextFile->Filename = _T("");
    pTextFile->TextFileScale = 1.0f;
    pTextFile->Content = _T("");

    // parser layer attribute
    while(pAttributeOfTextFile)
    {
        CString ValueStr = CA2W(pAttributeOfTextFile->Value(), CP_UTF8);
        CString NameStr = CA2W(pAttributeOfTextFile->Name(), CP_UTF8);

        if(NameStr.CompareNoCase( _T("FileName")) == 0)
        {
            if(ValueStr.Find(_T(':')) == -1)
            {
                CFileStatus status;
                if( !CFile::GetStatus( m_PlaylistPath + ValueStr, status ) &&  m_MediaLibraryPath != _T(""))
                {
                    ValueStr = m_MediaLibraryPath + ValueStr;
                }else
                {
                    ValueStr = m_PlaylistPath + ValueStr;
                }
            }

            pTextFile->Filename = ValueStr;
        }
        if(NameStr.CompareNoCase( _T("ScaleRate")) == 0)
        {
            pTextFile->TextFileScale = (float)_tstof(ValueStr);
        }

        if(NameStr.CompareNoCase( _T("Content")) == 0)
        {
            pTextFile->Content = ValueStr;
        }

        pAttributeOfTextFile = pAttributeOfTextFile->Next();
    }

    return S_OK;
}

HRESULT S3RenderScheduler::ParserContent(S3SIGNAGE_CONTENT_CONTAINER *pContent, TiXmlElement *pContentElement)
{
    TiXmlAttribute* pAttributeOfContent = pContentElement->FirstAttribute();  

    pContent->bHided = FALSE;
    pContent->bRendering = FALSE;
    pContent->pObject = NULL;
    pContent->pObjectForNextLoop = NULL;

    pContent->Content.TypeName = CA2W(pContentElement->Value(), CP_UTF8);

    pContent->Content.IDName = _T("");
    pContent->Content.XPos = 0;
    pContent->Content.YPos = 0;
    pContent->Content.Width = 0;
    pContent->Content.Height = 0;
    pContent->Content.BGColor = 0;
    pContent->Content.StartTime = 0;
    pContent->Content.Duration = 0x200000;

    pContent->Content.TransitionIn.Duration = 0;
    pContent->Content.TransitionIn.Name = _T("");
    pContent->Content.TransitionIn.Direction = _T("");

    pContent->Content.TransitionOut.Duration = 0;
    pContent->Content.TransitionOut.Name = _T("");
    pContent->Content.TransitionOut.Direction = _T("");

    // parser content attribute
    while(pAttributeOfContent)
    {
        CString ValueStr = CA2W(pAttributeOfContent->Value(), CP_UTF8);
        CString NameStr = CA2W(pAttributeOfContent->Name(), CP_UTF8);

        if(NameStr.CompareNoCase( _T("ID")) == 0)
        {
            pContent->Content.IDName = ValueStr;
        }
        else if(NameStr.CompareNoCase(_T("StartTime")) == 0)
        {
            pContent->Content.StartTime = ConvertStr2Duration(ValueStr);
        }
        else if(NameStr.CompareNoCase(_T("Duration")) == 0)
        {
            pContent->Content.Duration = ConvertStr2Duration(ValueStr);
            if(pContent->Content.Duration == 0) pContent->Content.Duration = 0x200000;

        }
        else if(NameStr.CompareNoCase( _T("XPos")) == 0)
        {
			int iIndex = ValueStr.Find(_T('%'));
			if (iIndex == -1)
			{
				pContent->Content.XPos = _tstoi(ValueStr);
			}
			else
			{
				pContent->Content.XPos = _tstoi(ValueStr) * m_Canvas.Width / 100;
			}

            pContent->Content.XPos = (int)(pContent->Content.XPos * m_Scale + m_XTrans + 0.5f);
        }
        else if(NameStr.CompareNoCase(_T("YPos")) == 0)
        {
			int iIndex = ValueStr.Find(_T('%'));
			if (iIndex == -1)
			{
				pContent->Content.YPos = _tstoi(ValueStr);
			}
			else
			{
				pContent->Content.YPos = _tstoi(ValueStr) * m_Canvas.Height / 100;
			}

            pContent->Content.YPos = (int)(pContent->Content.YPos * m_Scale + m_YTrans + 0.5f);       
        }
        else if(NameStr.CompareNoCase( _T("Width")) == 0)
        {
			int iIndex = ValueStr.Find(_T('%'));
			if (iIndex == -1)
			{
				pContent->Content.Width = _tstoi(ValueStr);
			}
			else
			{
				pContent->Content.Width = _tstoi(ValueStr) * m_Canvas.Width / 100;
			}

            pContent->Content.Width = (int)(pContent->Content.Width * m_Scale + 0.5f);
        }
        else if(NameStr.CompareNoCase( _T("Height")) == 0)
        {
			int iIndex = ValueStr.Find(_T('%'));
			if (iIndex == -1)
			{
				pContent->Content.Height = _tstoi(ValueStr);
			}
			else
			{
				pContent->Content.Height = _tstoi(ValueStr) * m_Canvas.Height / 100;
			}

            pContent->Content.Height = (int)(pContent->Content.Height * m_Scale + 0.5f);
        }
        else if(NameStr.CompareNoCase( _T("BgColor")) == 0)
        {
            pContent->Content.BGColor = _tcstoul(ValueStr, NULL, 16);
        }else
        {
            if(NameStr.CompareNoCase( _T("FileName")) == 0) 
            {
                if(ValueStr.Find(_T(':')) == -1)
                {
                    CFileStatus status;
                    if( !CFile::GetStatus( m_PlaylistPath + ValueStr, status ) &&  m_MediaLibraryPath != _T(""))
                    {
                        ValueStr = m_MediaLibraryPath + ValueStr;
                    }else
                    {
                        ValueStr = m_PlaylistPath + ValueStr;
                    }
                }
            }

            if(NameStr.CompareNoCase( _T("URL")) == 0)
            {
                NameStr = _T("FileName");
            }

            pContent->Content.Attribute[NameStr] = ValueStr;
        }

        pAttributeOfContent = pAttributeOfContent->Next();
    }

    TiXmlElement* pSubContentElement = pContentElement->FirstChildElement();  

    while(pSubContentElement)
    {
        CString SubContentName = CA2W(pSubContentElement->Value(), CP_UTF8);
        if(SubContentName.CompareNoCase( _T("TransitionIn")) == 0) 
        {
            ParserTranstion(&pContent->Content.TransitionIn, pSubContentElement);
        }

        if(SubContentName.CompareNoCase(  _T("TransitionOut")) == 0) 
        {
            ParserTranstion(&pContent->Content.TransitionOut, pSubContentElement);
        }

        if(SubContentName.CompareNoCase( _T("Translate")) == 0) 
        {
            ParserTransform(&pContent->Content.Translate, pSubContentElement);

            for(DWORD i=0; i<pContent->Content.Translate.size(); i++ )
            {
                pContent->Content.Translate[i].XValue *= m_Scale;
                pContent->Content.Translate[i].YValue *= m_Scale;
            }
        }

        if(SubContentName.CompareNoCase( _T("Scale")) == 0) 
        {
            ParserTransform(&pContent->Content.Scale, pSubContentElement);
        }

        if(SubContentName.CompareNoCase( _T("Rotation")) == 0) 
        {
            ParserTransform(&pContent->Content.Rotate, pSubContentElement);
        }

        if(SubContentName.CompareNoCase( _T("TextFile")) == 0) 
        {
            TextFileItem TextFile;
            ParserTextFile(&TextFile, pSubContentElement);
            pContent->Content.TextFile.push_back(TextFile);
        }

        pSubContentElement = pSubContentElement->NextSiblingElement();
    }

    if(pContent->Content.TransitionIn.Duration > (UINT)pContent->Content.Duration)
    {
        DbgMsg(_T("Warning: TransitionIn duration exceed %s duration"), pContent->Content.TypeName);
        pContent->Content.TransitionIn.Duration = pContent->Content.Duration;
    }

    if(pContent->Content.TransitionOut.Duration > (UINT)pContent->Content.Duration)
    {
        DbgMsg(_T("Warning: TransitionOut duration exceed %s duration"), pContent->Content.TypeName);
        pContent->Content.TransitionOut.Duration = pContent->Content.Duration;
    }

    if(pContent->Content.TransitionOut.Duration + pContent->Content.TransitionIn.Duration > (UINT)pContent->Content.Duration)
    {
        DbgMsg(_T("Warning: %s TransitionIn and our duration overlap"), pContent->Content.TypeName);
        pContent->Content.TransitionOut.Duration = pContent->Content.Duration - pContent->Content.TransitionIn.Duration;
    }

    return S_OK;
}


HRESULT S3RenderScheduler::LoadPlayList(CString Filename, CString MediaLibraryPath)
{
    HRESULT hr;
    hr = Stop();
    if(FAILED(hr)) return hr;

    hr = Terminate();
    if(FAILED(hr)) return hr;

    if(MediaLibraryPath != _T(""))
    {
        m_MediaLibraryPath = MediaLibraryPath;
    }

    TCHAR FullPathName[MAX_PATH];
    TCHAR *pFileName;

    GetFullPathName(Filename, MAX_PATH, FullPathName, &pFileName);

    Filename = FullPathName;

    int LastSlash = Filename.ReverseFind(_T('\\'));

    m_PlaylistPath = Filename.Left(LastSlash + 1);

    TiXmlDocument *pDoc = new TiXmlDocument();  
    if (NULL==pDoc)  
    {  
        DbgMsg(_T("Error: Out of memmory"));
        return E_UNEXPECTED;  
    }  

    FILE *pXMLFile = _tfopen(Filename, _T("rb"));
    if(pXMLFile == NULL)
    {
        DbgMsg(_T("Error: Can not open playlist file %s"), Filename);
        delete pDoc;
        return E_UNEXPECTED;
    }


    if(! pDoc->LoadFile(pXMLFile, TIXML_ENCODING_UTF8))
    {
        DbgMsg(_T("Error: Can not open playlist file %s"), Filename);
        delete pDoc;
        return E_UNEXPECTED;
    }
    
    TiXmlElement* pCanvasElement = pDoc->RootElement(); 

    ASSERT(CString(_T("Canvas")).CompareNoCase(CA2W(pCanvasElement->Value(), CP_UTF8)) == 0);

    m_Canvas.Name = _T("");
    m_Canvas.Width = 0;
    m_Canvas.Height = 0;
    m_Canvas.BGColor = 0xFF000000;

    TiXmlAttribute* pAttributeOfCanvas = pCanvasElement->FirstAttribute();  

    // parser canvas attribute
    while(pAttributeOfCanvas)
    {
        CString ValueStr = CA2W(pAttributeOfCanvas->Value(), CP_UTF8);
        CString NameStr = CA2W(pAttributeOfCanvas->Name(), CP_UTF8);

        if(NameStr.CompareNoCase( _T("ID")) == 0)   m_Canvas.Name = ValueStr;
        if(NameStr.CompareNoCase( _T("Width")) == 0)   m_Canvas.Width = _tstoi(ValueStr);
        if(NameStr.CompareNoCase( _T("Height")) == 0)   m_Canvas.Height = _tstoi(ValueStr);
        if(NameStr.CompareNoCase( _T("BgColor")) == 0)   m_Canvas.BGColor = _tcstoul(ValueStr, NULL, 16);

        pAttributeOfCanvas = pAttributeOfCanvas->Next();
    }

    CalculateTransform();

    TiXmlElement* pLayerElement = pCanvasElement->FirstChildElement();  
    while(pLayerElement)
    {
        if(CString(_T("Layer")).CompareNoCase(CA2W(pLayerElement->Value(), CP_UTF8)) != 0) continue;

        S3SIGNAGE_LAYER *pLayer = new S3SIGNAGE_LAYER;

        ParserLayer(pLayer, pLayerElement);

        m_Canvas.Layers.push_back(pLayer);

        TiXmlElement* pContentElement = pLayerElement->FirstChildElement();  
        
        pLayerElement = pLayerElement->NextSiblingElement();
    }

    delete pDoc;
    fclose(pXMLFile);

	if(g_bDEMO)
	{
		S3SIGNAGE_LAYER *pLayer = new S3SIGNAGE_LAYER;
		pLayer->VisibleCount = 0;
		pLayer->StartTime = 0x20000000;
		pLayer->Name = _T("");
		pLayer->Duration = 0x200000;

		S3SIGNAGE_CONTENT_CONTAINER *pContent = new S3SIGNAGE_CONTENT_CONTAINER;

		pContent->bHided = FALSE;
		pContent->bRendering = FALSE;
		pContent->pObject = NULL;
		pContent->pObjectForNextLoop = NULL;

		pContent->Content.TypeName = _T("ScrollTextFile");

		pContent->Content.IDName = _T("");
        pContent->Content.XPos = (int)((m_Canvas.Width - 256) * m_Scale + m_XTrans + 0.5f);
        pContent->Content.YPos = (int)((m_Canvas.Height - 152) * m_Scale + m_YTrans + 0.5f);
        pContent->Content.Width = (int)(256 * m_Scale + 0.5f);
        pContent->Content.Height = (int)(152 * m_Scale + 0.5f);
		pContent->Content.BGColor = 0x40FFFFFF;
		pContent->Content.StartTime = 0;
		pContent->Content.Duration = 0x200000;

        TextFileItem TextFile;
        TextFile.bResourceFile = TRUE;
        TextFile.ResourceID = IDB_S3_LOGO;
        TextFile.TextFileScale = 1;
        
        pContent->Content.TextFile.push_back(TextFile);

		pContent->Content.TransitionIn.Duration = 0;
		pContent->Content.TransitionIn.Name = _T("");
		pContent->Content.TransitionIn.Direction = _T("");

		pContent->Content.TransitionOut.Duration = 0;
		pContent->Content.TransitionOut.Name = _T("");
		pContent->Content.TransitionOut.Direction = _T("");

        pLayer->Contents.push_back(pContent); 
		m_Canvas.Layers.push_front(pLayer);
	}

    GenerateLayerZOrder();

    //create background object
    S3SIGNAGE_CONTENT BackgroundObject;
    BackgroundObject.TypeName = _T("Rectangle");

    BackgroundObject.IDName = _T("");
    BackgroundObject.XPos = (int)(m_XTrans + 0.5f);
    BackgroundObject.YPos = (int)(m_YTrans + 0.5f);
    BackgroundObject.Width = (int)(m_Canvas.Width * m_Scale + 0.5f);
    BackgroundObject.Height = (int)(m_Canvas.Height * m_Scale + 0.5f);
    BackgroundObject.BGColor = 0;
    BackgroundObject.StartTime = 0;
    BackgroundObject.Duration = 0x200000;
    CString ColorString;
    ColorString.Format(_T("0x%p"), m_Canvas.BGColor | 0xFF000000);
    BackgroundObject.Attribute[_T("Color")]= ColorString;


    m_Canvas.pBackgroundObj = new S3ObjectContainer(BackgroundObject, m_Scale, m_nFPS, m_RotationDegree);
    m_Canvas.pBackgroundObj->SetZOrder(1);

    S3RenderEngine *pRenderEning = NULL;
    LPDIRECT3DDEVICE9 pd3dDevice = NULL;
                    
    m_pMixer->GetRenderEngineOwner(&pRenderEning);
    pRenderEning->Get3DDevice(&pd3dDevice);
#ifndef PLAYER_DUMMY
    hr = m_Canvas.pBackgroundObj->InitDeviceObjects(pd3dDevice);
    if(FAILED(hr))
    {
        DbgMsg(_T("Error: Failed to initalize background"));
        return hr;
    }

    pd3dDevice->Release();
    m_Canvas.pBackgroundObj->WaitCreateThreadFinish();
    m_Canvas.pBackgroundObj->Start();
#endif
    m_pMixer->AddRenderableObject(m_Canvas.pBackgroundObj);
    m_pMixer->m_ScreenClip.left = (float)BackgroundObject.XPos;
    m_pMixer->m_ScreenClip.top = (float)BackgroundObject.YPos;
    m_pMixer->m_ScreenClip.right = (float)BackgroundObject.Width + BackgroundObject.XPos;
    m_pMixer->m_ScreenClip.bottom = (float)BackgroundObject.Height + BackgroundObject.YPos;

    DbgMsg(_T("Load playlist file %s"), Filename);

    return S_OK;
}

HRESULT S3RenderScheduler::Play()
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;
    HRESULT hr;

    InitAllLayerStartTime(timeGetTime());

    // create all object need to initalize
    hr = PreloadObjects();

    if(FAILED(hr)) return hr;

    // wait content loading

    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
            S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();
            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if( pContent && pContent->pObject != NULL)
                {
                    pContent->pObject->WaitCreateThreadFinish();
                }
            }
        }
    }

    //skip loading time
    InitAllLayerStartTime(timeGetTime());

    // start to rendering
    hr = SchedulerObjects();
    if(FAILED(hr)) return hr;

    return S_OK;
}

HRESULT S3RenderScheduler::Stop()
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;

    m_bPaused = FALSE;

    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
            S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if( pContent )
                {
                    if(pContent->pObject)
                    {
                        pContent->pObject->Stop();
                    }
                }
            }
        }
    }
    DbgMsg(_T("Player Stopped"));
    return S_OK;
}


HRESULT S3RenderScheduler::Quit()
{
    Stop();
    Terminate();
    DbgMsg(_T("Player Quit"));
    return S_OK;
}

HRESULT S3RenderScheduler::SaveSnapShot(CString Filename)
{
    return S_OK;
}

HRESULT S3RenderScheduler::UpdateRenderObjects()
{
    if(m_bPaused) return S_OK;
    PreloadObjects();
    SchedulerObjects();
	CheckObjectsLoop();
    return S_OK;
}

HRESULT S3RenderScheduler::CheckObjectsLoop()
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;


    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
            S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if( pContent )
                {
                    if(pContent->pObject && pContent->bRendering)
                    {
                        pContent->pObject->LoopContent();
                    }
                }
            }

        }
    }
    return S_OK;
}

HRESULT S3RenderScheduler::Terminate()
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;

    m_bPaused = FALSE;

    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
            S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if( pContent )
                {
                    if(pContent->pObject)
                    {
                        m_pMixer->RemoveRenderableObject(pContent->pObject);
                        pContent->pObject->DeleteDeviceObjects();
                        delete pContent->pObject; 
                        pContent->pObject = NULL;
                    }
                    if(pContent->pObjectForNextLoop)
                    {
                        pContent->pObjectForNextLoop->DeleteDeviceObjects();
                        delete pContent->pObjectForNextLoop; 
                        pContent->pObjectForNextLoop = NULL;
                    }
                    delete pContent;  
                }
            }
            pLayer->Contents.clear();
            delete pLayer;
        }
    }

    m_Canvas.Layers.clear();

    if(m_Canvas.pBackgroundObj)
    {
        m_pMixer->RemoveRenderableObject(m_Canvas.pBackgroundObj);
        m_Canvas.pBackgroundObj->DeleteDeviceObjects();
        delete m_Canvas.pBackgroundObj; 
        m_Canvas.pBackgroundObj = NULL;
    }

    return S_OK;
}

HRESULT S3RenderScheduler:: initCanvas()
{
	m_Canvas.pBackgroundObj = NULL;
	m_Canvas.Width = ::GetSystemMetrics(SM_CXSCREEN);
	m_Canvas.Height = ::GetSystemMetrics(SM_CYSCREEN);
	m_Canvas.Name = _T("C1");
	m_Canvas.BGColor = 0;

	CalculateTransform();
	//create background object
	S3SIGNAGE_CONTENT BackgroundObject;
	BackgroundObject.TypeName = _T("Rectangle");

	BackgroundObject.IDName = _T("");
	BackgroundObject.XPos = (int)(m_XTrans + 0.5f);
	BackgroundObject.YPos = (int)(m_YTrans + 0.5f);
	BackgroundObject.Width = (int)(m_Canvas.Width * m_Scale + 0.5f);
	BackgroundObject.Height = (int)(m_Canvas.Height * m_Scale + 0.5f);
	BackgroundObject.BGColor = 0;
	BackgroundObject.StartTime = 0;
	BackgroundObject.Duration = 0x200000;
	CString ColorString;
	ColorString.Format(_T("0x%p"), m_Canvas.BGColor | 0xFF000000);
	BackgroundObject.Attribute[_T("Color")]= ColorString;


	m_Canvas.pBackgroundObj = new S3ObjectContainer(BackgroundObject, m_Scale, m_nFPS, m_RotationDegree);
	m_Canvas.pBackgroundObj->SetZOrder(1);

	S3RenderEngine *pRenderEning = NULL;
	LPDIRECT3DDEVICE9 pd3dDevice = NULL;

	m_pMixer->GetRenderEngineOwner(&pRenderEning);
	pRenderEning->Get3DDevice(&pd3dDevice);
#ifndef PLAYER_DUMMY
	HRESULT hr = m_Canvas.pBackgroundObj->InitDeviceObjects(pd3dDevice);
	if(FAILED(hr))
	{
		DbgMsg(_T("Error: Failed to initalize background"));
		return hr;
	}

	pd3dDevice->Release();
	m_Canvas.pBackgroundObj->WaitCreateThreadFinish();
	m_Canvas.pBackgroundObj->Start();
#endif
	m_pMixer->AddRenderableObject(m_Canvas.pBackgroundObj);
	m_pMixer->m_ScreenClip.left = (float)BackgroundObject.XPos;
	m_pMixer->m_ScreenClip.top = (float)BackgroundObject.YPos;
	m_pMixer->m_ScreenClip.right = (float)BackgroundObject.Width + BackgroundObject.XPos;
	m_pMixer->m_ScreenClip.bottom = (float)BackgroundObject.Height + BackgroundObject.YPos;

	return TRUE;

}

VOID S3RenderScheduler::GenerateLayerZOrder()
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;
    DWORD ZOrderIndex = 10 + m_Canvas.Layers.size() * 10;

    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer )
        {
            pLayer->ZOrder = ZOrderIndex;
            ZOrderIndex -= 10;
        }
    }
}

HRESULT S3RenderScheduler::PreloadObjects()
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;
    HRESULT hr;

    DWORD CurrentTime = timeGetTime();

    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
            S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            int CurrentLayerTime = (int)(CurrentTime - pLayer->StartTime) / 1000;
            int NextLoopTime = (int)(CurrentTime - pLayer->StartTime) / 1000 - pLayer->Duration;

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if( pContent && pContent->pObject == NULL &&
                    ((pContent->Content.StartTime - S3S_PRELOAD_TIME < CurrentLayerTime) &&
                    (pContent->Content.StartTime +  pContent->Content.Duration > CurrentLayerTime)))
                {
                    pContent->pObject = new S3ObjectContainer(pContent->Content, m_Scale, m_nFPS, m_RotationDegree);
                    pContent->pObject->SetZOrder(pLayer->ZOrder);

                    S3RenderEngine *pRenderEning = NULL;
                    LPDIRECT3DDEVICE9 pd3dDevice = NULL;
                    
                    m_pMixer->GetRenderEngineOwner(&pRenderEning);
                    pRenderEning->Get3DDevice(&pd3dDevice);
#ifndef PLAYER_DUMMY
                    hr = pContent->pObject->InitDeviceObjects(pd3dDevice);

                    pd3dDevice->Release();
#else
                    CClientChecker& clientChecker = ((CS3SignageApp*)AfxGetApp())->m_ClientChecker;
                    LOG(g_pLogger, INFO) << _T("Preloading content file:");
                    LOG(g_pLogger, INFO) << _T("\ttype: ") << pContent->Content.TypeName.GetString();
                    if (pContent->Content.TypeName == _T("ScrollTextFile"))
                    {
                        LOG(g_pLogger, INFO) << _T("\tTextFile: ");

                        auto it = pContent->Content.TextFile.begin(), ite = pContent->Content.TextFile.end();
                        for (; it != ite; ++it)
                        {
                            LOG(g_pLogger, INFO) << _T("\t  File name = ") << it->Filename.GetString();
                            clientChecker.IsFileExist(it->Filename.GetString());
                            LOG(g_pLogger, INFO) << _T("\t  Text file scale = ") << it->TextFileScale;
                        }

                    }
                    {
                        LOG(g_pLogger, INFO) << _T("\tAttribute list:");
                        auto it = pContent->Content.Attribute.begin(), ite = pContent->Content.Attribute.end();
                        for (; it != ite; ++it)
                        {
                            LOG(g_pLogger, INFO) << _T("\t  ") << it->first.GetString() << _T(" = ") << it->second.GetString();
                            if (pContent->Content.TypeName == _T("Movie") && it->first == _T("FileName"))
                            {
                                clientChecker.IsFileExist(it->second.GetString());
                            }
                        }
                    }
#endif
                }

                if( pContent && pContent->pObjectForNextLoop == NULL &&
                    ((pContent->Content.StartTime - S3S_PRELOAD_TIME < NextLoopTime) &&
                    (pContent->Content.StartTime +  pContent->Content.Duration > NextLoopTime)))
                {
                    pContent->pObjectForNextLoop = new S3ObjectContainer(pContent->Content, m_Scale, m_nFPS, m_RotationDegree);
                    pContent->pObjectForNextLoop->SetZOrder(pLayer->ZOrder);

                    S3RenderEngine *pRenderEning = NULL;
                    LPDIRECT3DDEVICE9 pd3dDevice = NULL;
                    
                    m_pMixer->GetRenderEngineOwner(&pRenderEning);
                    pRenderEning->Get3DDevice(&pd3dDevice);

#ifndef PLAYER_DUMMY
                    hr = pContent->pObjectForNextLoop->InitDeviceObjects(pd3dDevice);

                    pd3dDevice->Release();
#else
                    LOG(g_pLogger, INFO) << _T("Preloading content file(next loop):");
                    LOG(g_pLogger, INFO) << _T("\ttype:") << pContent->Content.TypeName.GetString();
                    LOG(g_pLogger, INFO) << _T("\tAttribute list:");
                    auto it = pContent->Content.Attribute.begin(), ite = pContent->Content.Attribute.end();
                    for (; it != ite; ++it)
                    {
                        LOG(g_pLogger, INFO) << _T("\t  ") << it->first.GetString() << _T(" = ") << it->second.GetString();
                        // TODO: check file
                    }
#endif // 

                }

            }
        }
    }
    return S_OK;
}

HRESULT S3RenderScheduler::SchedulerObjects()
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;
    HRESULT hr;

    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();

    int CurrentTime = timeGetTime();

    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
            BOOL bLayerLooped = FALSE;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();
            // loop layers
            while((CurrentTime - pLayer->StartTime) > pLayer->Duration * 1000)
            {
                pLayer->StartTime += pLayer->Duration * 1000;
                bLayerLooped = TRUE;
            }

            int CurrentLayerTime = (int)(CurrentTime - pLayer->StartTime) / 1000;

            S3ObjectContainer *pExpireObject = NULL;
            S3ObjectContainer *pNewObject = NULL;

            S3SIGNAGE_CONTENT_CONTAINER *pNewContent = NULL;

            if(bLayerLooped)
            {
                for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
                {
                    S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;
                    pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);

                    // remove expire content
                    if( pContent && pContent->pObject != NULL) 
                    {
                        if(pExpireObject != NULL)
                        {
                            //remove the current first, because the preview ExpireObject may still rendering, it need to be replaced by new object
                            DbgMsg(_T("Warning: Two expire content found when layer %s loop"), pLayer->Name);
                            m_pMixer->RemoveRenderableObject(pContent->pObject);
                            pContent->pObject->DeleteDeviceObjects();
                            delete pContent->pObject; 
                            pContent->pObject = NULL;
                            pContent->bRendering = FALSE;
                        }else
                        {
                            pExpireObject = pContent->pObject;
                            pContent->bRendering = FALSE;
                            pContent->pObject = NULL;    
                        }

                    }

                    if( pContent && pContent->pObjectForNextLoop)
                    {
                        pContent->pObject = pContent->pObjectForNextLoop;
                        pContent->pObjectForNextLoop = NULL;
                    }
                }  
            }


            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);

                // remove expire content
                if( pContent && pContent->pObject != NULL && 
                    (pContent->Content.StartTime +  pContent->Content.Duration <= CurrentLayerTime)) 
                {
                    if(pExpireObject != NULL)
                    {
                        //remove the current first, because the preview ExpireObject may still rendering, it need to be replaced by new object

                        DbgMsg(_T("Warning: Two expire content found in layer %s"), pLayer->Name);
                        m_pMixer->RemoveRenderableObject(pContent->pObject);
                        pContent->pObject->DeleteDeviceObjects();
                        delete pContent->pObject; 
                        pContent->pObject = NULL;
                        pContent->bRendering = FALSE;
                    }else
                    {
                        pExpireObject = pContent->pObject;
                        pContent->bRendering = FALSE;
                        pContent->pObject = NULL;
                    }

                }

                // add new content
                if( pContent && pContent->pObject != NULL && pContent->bRendering == FALSE &&
                    pContent->bHided == FALSE &&
                    (pContent->Content.StartTime <= CurrentLayerTime) &&
                    (pContent->Content.StartTime +  pContent->Content.Duration > CurrentLayerTime)) 
                {
                    pNewObject = pContent->pObject;
                    pContent->bRendering = TRUE;
                    pNewContent = pContent;
                }
            }

            if(pExpireObject && pNewObject)
            {
                hr = pNewObject->Start();
                if(FAILED(hr))
                {
                    pNewContent->bRendering = FALSE;

                    m_pMixer->RemoveRenderableObject(pExpireObject);
                    pExpireObject->DeleteDeviceObjects();
                    delete pExpireObject; 
                    pExpireObject = NULL;
                    pLayer->VisibleCount --;

                }else
                {
                    m_pMixer->ReplaceRenderableObject(pExpireObject, pNewObject);
                    pExpireObject->DeleteDeviceObjects();
                    delete pExpireObject; 
                    pExpireObject = NULL; 
                }

            }else if(pExpireObject && pNewObject == NULL)
            {
                m_pMixer->RemoveRenderableObject(pExpireObject);
                pExpireObject->DeleteDeviceObjects();
                delete pExpireObject; 
                pExpireObject = NULL;

                pLayer->VisibleCount --;
            }else if(pExpireObject == NULL && pNewObject)
            {
                hr = pNewObject->Start();
                if(FAILED(hr))
                {
                    pNewContent->bRendering = FALSE;
                }else
                {
                    m_pMixer->AddRenderableObject(pNewObject);
                    pLayer->VisibleCount ++;
                }
            }


            if(pLayer->VisibleCount > 1)
            {
                DbgMsg(_T("Error: time range overlap in layer %s"), pLayer->Name);
            }
        }
    }
    return S_OK;
}


VOID S3RenderScheduler::InitAllLayerStartTime(DWORD Time)
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;

    DWORD CurrentTime = timeGetTime();

    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer )
        {
            pLayer->StartTime = Time;
        }
    }
    return ;
}


VOID S3RenderScheduler::ResetAllContentZOrder()
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;

    m_pMixer->Lock();

    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
            S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if( pContent && pContent->pObject != NULL)
                {
                    pContent->pObject->SetZOrder(pLayer->ZOrder);
                }
            }
        }
    }
    m_pMixer->Unlock();
}


HRESULT S3RenderScheduler::Pause()
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;

    if(!m_bPaused) return S_OK;

    m_bPaused = TRUE;

    DWORD CurrentTime = timeGetTime();

    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
            S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

            pLayer->StartTime = CurrentTime - pLayer->StartTime;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if( pContent )
                {
                    if(pContent->pObject)
                    {
                        pContent->pObject->Pause();
                    }
                }
            }
        }
    }

    if(m_Canvas.pBackgroundObj)
    {
        m_Canvas.pBackgroundObj->Pause();
    }
    return S_OK;
}


HRESULT S3RenderScheduler::Resume()
{
    list<S3SIGNAGE_LAYER *>::iterator LayerStart, LayerEnd, LayerIt;
    S3SIGNAGE_LAYER *pLayer = NULL;

    if(m_bPaused) return S_OK;

    m_bPaused = FALSE;

    DWORD CurrentTime = timeGetTime();

    LayerStart = m_Canvas.Layers.begin();
    LayerEnd = m_Canvas.Layers.end();
    for( LayerIt=LayerStart; LayerIt!=LayerEnd; LayerIt++)
    {
        pLayer = (S3SIGNAGE_LAYER*)(*LayerIt);
        if( pLayer  && (pLayer->Contents.size() > 0))
        {
            list<S3SIGNAGE_CONTENT_CONTAINER *>::iterator ContentStart, ContentEnd, ContentIt;
            S3SIGNAGE_CONTENT_CONTAINER *pContent = NULL;

            pLayer->StartTime = CurrentTime - pLayer->StartTime;

            ContentStart = pLayer->Contents.begin();
            ContentEnd = pLayer->Contents.end();

            for( ContentIt=ContentStart; ContentIt!=ContentEnd; ContentIt++)
            {
                pContent = (S3SIGNAGE_CONTENT_CONTAINER*)(*ContentIt);
                if( pContent )
                {
                    if(pContent->pObject)
                    {
                        pContent->pObject->Resume();
                    }
                }
            }
        }
    }

    if(m_Canvas.pBackgroundObj)
    {
        m_Canvas.pBackgroundObj->Resume();
    }
    return S_OK;
}

BOOL S3RenderScheduler::amdInitialize()
{
	ASSERT(m_hMixer == NULL);

	// get the number of mixer devices present in the system
	m_nNumMixers = ::mixerGetNumDevs();

	m_hMixer = NULL;
	::ZeroMemory(&m_mxcaps, sizeof(MIXERCAPS));

	m_dwMinimum = 0;
	m_dwMaximum = 0;
	m_dwVolumeControlID = 0;

	// open the first mixer
	// A "mapper" for audio mixer devices does not currently exist.
	if (m_nNumMixers != 0)
	{
		if (::mixerOpen(&m_hMixer,
						0,
						NULL,
						NULL,
						MIXER_OBJECTF_MIXER)
			!= MMSYSERR_NOERROR)
		{
			return FALSE;
		}

		if (::mixerGetDevCaps(reinterpret_cast<UINT>(m_hMixer),
							  &m_mxcaps, sizeof(MIXERCAPS))
			!= MMSYSERR_NOERROR)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL S3RenderScheduler::amdUninitialize()
{
	BOOL bSucc = TRUE;

	if (m_hMixer != NULL)
	{
		bSucc = (::mixerClose(m_hMixer) == MMSYSERR_NOERROR);
		m_hMixer = NULL;
	}

	return bSucc;
}

BOOL S3RenderScheduler::amdGetMasterVolumeControl()
{
	if (m_hMixer == NULL)
	{
		return FALSE;
	}

	// get dwLineID
	MIXERLINE mxl;
    memset(&mxl, 0, sizeof(MIXERLINE));
	mxl.cbStruct = sizeof(MIXERLINE);
	mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
	MMRESULT mr = ::mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_hMixer),
						   &mxl,
						   MIXER_OBJECTF_HMIXER |
						   MIXER_GETLINEINFOF_LINEID);
    if( mr != MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	// get dwControlID
	MIXERCONTROL mxc;
	MIXERLINECONTROLS mxlc;
	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	if (::mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(m_hMixer),
							   &mxlc,
							   MIXER_OBJECTF_HMIXER |
							   MIXER_GETLINECONTROLSF_ONEBYTYPE)
		!= MMSYSERR_NOERROR)
	{
		return FALSE;
	}

	// store dwControlID
	m_dwMinimum = mxc.Bounds.dwMinimum;
	m_dwMaximum = mxc.Bounds.dwMaximum;
	m_dwVolumeControlID = mxc.dwControlID;

	return TRUE;
}

BOOL S3RenderScheduler::amdGetMasterVolumeValue(DWORD &dwVal) const
{
	if (m_hMixer == NULL)
	{
		return FALSE;
	}

	MIXERCONTROLDETAILS_UNSIGNED mxcdVolume;
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = m_dwVolumeControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = &mxcdVolume;
	if (::mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),
								 &mxcd,
								 MIXER_OBJECTF_HMIXER |
								 MIXER_GETCONTROLDETAILSF_VALUE)
		!= MMSYSERR_NOERROR)
	{
		return FALSE;
	}
	
	dwVal = mxcdVolume.dwValue;

	return TRUE;
}

BOOL S3RenderScheduler::amdSetMasterVolumeValue(DWORD dwVal) const
{
	if (m_hMixer == NULL)
	{
		return FALSE;
	}

	MIXERCONTROLDETAILS_UNSIGNED mxcdVolume = { dwVal };
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = m_dwVolumeControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = &mxcdVolume;
	if (::mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(m_hMixer),
								 &mxcd,
								 MIXER_OBJECTF_HMIXER |
								 MIXER_SETCONTROLDETAILSF_VALUE)
		!= MMSYSERR_NOERROR)
	{
		return FALSE;
	}
	
	return TRUE;
}
