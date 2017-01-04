#include "stdafx.h"
#include "S3TextFileObj.h"
#include "AttributeParser.h"
#include "atlimage.h"
#include "S3Signage.h"
#include "CommonLib/base64.h"
#include <iterator>

UINT  MAX_TEXTURE_SIZE = 0;

S3TextFileObj::S3TextFileObj(int iWidth, int iHeight, float fScaleRate, AttribList  &Attribute, S3SIGNAGE_TEXTFILE &TextFile,  int nFPS)
	:S3RenderableObject(iWidth, iHeight, fScaleRate, Attribute)
{
	CString DirectionString;
	CString LayoutString;

	DirectionString = GetStringAttrib(m_Attribute, _T("Direction"), _T("None"));
	m_Speed = GetFloatAttrib(m_Attribute, _T("Speed"), 0.0f);
	m_Speed *= fScaleRate;

	m_TextFileInfo = TextFile;
	LayoutString = GetStringAttrib(m_Attribute, _T("Layout"), _T("Horizontal"));

	m_TextDirection = S3S_TEXT_DIRECTION_NONE;

	if(DirectionString.CompareNoCase(_T("LeftRight")) == 0)
	{
		m_TextDirection = S3S_TEXT_DIRECTION_LEFT_RIGHT;
	}

	if(DirectionString.CompareNoCase(_T("RightLeft")) == 0)
	{
		m_TextDirection = S3S_TEXT_DIRECTION_RIGHT_LEFT;
	}

	if(DirectionString.CompareNoCase(_T("UpDown")) == 0)
	{
		m_TextDirection = S3S_TEXT_DIRECTION_UP_DOWN;
	}

	if(DirectionString.CompareNoCase(_T("DownUp")) == 0)
	{
		m_TextDirection = S3S_TEXT_DIRECTION_DOWN_UP;
	}

	if(DirectionString.CompareNoCase(_T("Random")) == 0)
	{
		if (LayoutString.CompareNoCase(_T("Horizontal")) == 0)
		{
			S3S_TEXT_DIRECTION D[4] = {S3S_TEXT_DIRECTION_LEFT_RIGHT,
									   S3S_TEXT_DIRECTION_RIGHT_LEFT,
									   S3S_TEXT_DIRECTION_UP_DOWN,
									   S3S_TEXT_DIRECTION_DOWN_UP};
			m_TextDirection = D[rand()%4];
		}
		else if (LayoutString.CompareNoCase(_T("Vertical")) == 0)
		{
			S3S_TEXT_DIRECTION D[2] = {S3S_TEXT_DIRECTION_UP_DOWN,
									   S3S_TEXT_DIRECTION_DOWN_UP};
			m_TextDirection = D[rand()%2];
		}
	}

	m_bPaused = FALSE;

	if(m_Speed == 0.0f)
	{
		m_TextDirection = S3S_TEXT_DIRECTION_NONE;
	}

	if(m_TextDirection == S3S_TEXT_DIRECTION_NONE)
	{
		m_Speed = 0.0f;
	}

	m_bVerticalLayout = FALSE;
	if(LayoutString.CompareNoCase(_T("Vertical")) == 0)
	{
		m_bVerticalLayout = TRUE;
	}

	if(m_TextDirection == S3S_TEXT_DIRECTION_LEFT_RIGHT || m_TextDirection == S3S_TEXT_DIRECTION_RIGHT_LEFT)
	{
		m_bVerticalLayout = FALSE;
	}

	m_nFPS = nFPS;
}


S3TextFileObj::~S3TextFileObj(void)
{
	Stop();
	DeleteDeviceObjects();
}


HRESULT S3TextFileObj::InitDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
	S3RenderableObject::InitDeviceObjects(pd3dDevice);

	if(MAX_TEXTURE_SIZE == 0)
	{
		IDirect3D9 *pDirect3D = NULL;
		HRESULT hr  = pd3dDevice->GetDirect3D(&pDirect3D);
		if(FAILED(hr)) return hr;

		D3DCAPS9 DeviceCaps;

		pDirect3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &DeviceCaps);

		MAX_TEXTURE_SIZE = DeviceCaps.MaxTextureWidth;
	}


	return S_OK;
}



HRESULT S3TextFileObj::UploadText(TEXT_INFO &TextInfo, CImage &TextImage)
{
	HRESULT hr;
	TextInfo.ImageWidth = TextImage.GetWidth();
	TextInfo.ImageHeight = TextImage.GetHeight();

	TextInfo.TextureWidth = min(TextInfo.ImageWidth, MAX_TEXTURE_SIZE);
	TextInfo.WrappedLineCount = (TextInfo.ImageWidth + TextInfo.TextureWidth - 1)/TextInfo.TextureWidth;

	int TextureHeight = TextInfo.WrappedLineCount * TextInfo.ImageHeight;

	LPDIRECT3DTEXTURE9 pShadowTexture = NULL;

	hr = m_pd3dDevice->CreateTexture( TextInfo.TextureWidth, TextureHeight, 1,
		D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pShadowTexture, NULL );

	if(FAILED(hr))
	{
		return hr;
	} 

	hr = m_pd3dDevice->CreateTexture(TextInfo.TextureWidth, TextureHeight, 1, D3DUSAGE_RENDERTARGET, 
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &TextInfo.pTexture, NULL);

	if(FAILED(hr) || !TextInfo.pTexture)
	{
		SAFE_RELEASE(pShadowTexture);
		return hr;
	} 


	D3DLOCKED_RECT  myLockedRect;
	try{
		if(FAILED(pShadowTexture->LockRect(0, &myLockedRect, NULL, D3DLOCK_DISCARD)))
		{
			throw 1;
		}
		char* pSrc = (char*)TextImage.GetBits();
		int   srcPitch = TextImage.GetPitch();
		int   BPP = TextImage.GetBPP();
		switch(BPP)
		{
		case 4:
		case 8:
		case 16:
			throw 2;
			break;
		case 24:
			{
				DWORD InitValue = 0;
				char* ptisrc = pSrc;
				char* ptidst = (char *)&InitValue;

				*ptidst++ = *ptisrc++;
				*ptidst++ = *ptisrc++;
				*ptidst++ = *ptisrc++;
				*ptidst++ = 0;

				for(int y=0; y < TextureHeight; y++)
				{
					DWORD* pTempInit = (DWORD*)((char *)myLockedRect.pBits + y * myLockedRect.Pitch);
					for(int x = 0; x < TextInfo.TextureWidth; x++)
					{
						*pTempInit++ = InitValue;
					}
				}



				for(int i=0; i < TextInfo.ImageHeight; i++)
				{
					char* pTempDst = (char *)myLockedRect.pBits + i * myLockedRect.Pitch;
					char* pTempSrc = pSrc + i * srcPitch;
					int  WidthLeft = TextInfo.ImageWidth;
					for(int iSeg = 0; iSeg < TextInfo.WrappedLineCount; iSeg++)
					{
						int WidthCopy = min(WidthLeft, TextInfo.TextureWidth);

						char* pttsrc = pTempSrc;
						char* pttdst = pTempDst;
						for(int j=0; j<WidthCopy; j++)
						{
							*pttdst++ = *pttsrc++;
							*pttdst++ = *pttsrc++;
							*pttdst++ = *pttsrc++;
							*pttdst++ = 0;
						}

						WidthLeft -= WidthCopy;
						pTempSrc += WidthCopy * 4;
						pTempDst += myLockedRect.Pitch * TextInfo.ImageHeight;
					}
				}
			}
			break;
		case 32:
			{
				DWORD InitValue = 0;
				char* ptisrc = pSrc;
				char* ptidst = (char *)&InitValue;

				*ptidst++ = *ptisrc++;
				*ptidst++ = *ptisrc++;
				*ptidst++ = *ptisrc++;
				*ptidst++ = *ptisrc++;

				for(int y=0; y < TextureHeight; y++)
				{
					DWORD* pTempInit = (DWORD*)((char *)myLockedRect.pBits + y * myLockedRect.Pitch);
					for(int x = 0; x < TextInfo.TextureWidth; x++)
					{
						*pTempInit++ = InitValue;
					}
				}



				for(int i=0; i < TextInfo.ImageHeight; i++)
				{
					char* pTempDst = (char *)myLockedRect.pBits + i * myLockedRect.Pitch;
					char* pTempSrc = pSrc + i * srcPitch;
					int WidthLeft = TextInfo.ImageWidth;
					for(int iSeg = 0; iSeg < TextInfo.WrappedLineCount; iSeg++)
					{
						int WidthCopy = min(WidthLeft, TextInfo.TextureWidth);
						memcpy(pTempDst, pTempSrc, WidthCopy * 4);

						WidthLeft -= WidthCopy;
						pTempSrc += WidthCopy * 4;
						pTempDst += myLockedRect.Pitch * TextInfo.ImageHeight;
					}
				}
			}
		}

	}
	catch(int)
	{
		pShadowTexture->UnlockRect(0);
		return E_UNEXPECTED;
	}
	pShadowTexture->UnlockRect(0);

	hr = m_pd3dDevice->UpdateTexture(pShadowTexture, TextInfo.pTexture);

	pShadowTexture->Release();

	return S_OK;
}


HRESULT S3TextFileObj::Initalize()
{
	for(DWORD i=0; i < m_TextFileInfo.size(); i++)
	{
		CImage    m_theImage;
		HRESULT hr = S_OK;

		if(m_TextFileInfo[i].bResourceFile)
		{
			m_theImage.LoadFromResource(GetModuleHandle(NULL), m_TextFileInfo[i].ResourceID);    
		}else if(m_TextFileInfo[i].Filename != _T(""))
		{
			hr = m_theImage.Load(m_TextFileInfo[i].Filename);    
		}else if(m_TextFileInfo[i].Content != _T(""))
		{
			std::string PngContentString = CW2A(m_TextFileInfo[i].Content); 
			std::vector<BYTE> PngContent;
			MagicView::CommonLib::base64::decode(PngContentString.begin(), PngContentString.end(), std::back_inserter(PngContent));
			hr = E_FAIL;
			HGLOBAL hGlobal  =  GlobalAlloc(GMEM_MOVEABLE, PngContent.size() * sizeof(PngContent[0]));
			if(hGlobal != NULL)
			{
				void *  pData  =  GlobalLock(hGlobal);
				if(pData != NULL)
				{
					memcpy(pData, &PngContent[0], PngContent.size() * sizeof(PngContent[0]));
					GlobalUnlock(hGlobal); 
					IStream *  pStream  =  NULL;
					if  (CreateStreamOnHGlobal(hGlobal, FALSE,  & pStream)  ==  S_OK)
					{
						if (SUCCEEDED(m_theImage.Load(pStream)) && !m_theImage.IsNull())
						{
							hr = S_OK;
						}
						pStream -> Release();     
					}
				}
				GlobalFree(hGlobal);  
			}
		}else
		{
			hr = E_FAIL;
		}

		if(SUCCEEDED(hr))
		{
			TEXT_INFO NewText;
			HRESULT hr;
			NewText.TextFileScale = m_TextFileInfo[i].TextFileScale * m_ScaleRate;
			NewText.Filename = m_TextFileInfo[i].Filename;
			hr = UploadText(NewText, m_theImage);

			if(SUCCEEDED(hr))
			{   m_TextInfo.push_back(NewText);
			}
		}else
		{
			DbgMsg(_T("Error: failed to load text file %s"), m_TextFileInfo[i].Filename);
		}
	}

	m_CurrentText = -1;
	SwitchText(0);
	m_CurrentXPos = (float)m_TextStartXPos;
	m_CurrentYPos = (float)m_TextStartYPos;
	return S_OK;
}


HRESULT S3TextFileObj::DeleteDeviceObjects()
{
	for(DWORD i=0; i<m_TextInfo.size(); i++ )
	{
		SAFE_RELEASE(m_TextInfo[i].pTexture);
	}
	m_TextInfo.clear();
	return S_OK;
}

HRESULT S3TextFileObj::RestoreDeviceObjects(LPDIRECT3DDEVICE9 pDevice)
{
	InitDeviceObjects(pDevice);
	HRESULT hr;

	for(DWORD i=0; i < m_TextFileInfo.size(); i++)
	{
		CImage    m_theImage;
		HRESULT hr = S_OK;

		if(m_TextFileInfo[i].bResourceFile)
		{
			m_theImage.LoadFromResource(GetModuleHandle(NULL), m_TextFileInfo[i].ResourceID);    
		}else if(m_TextFileInfo[i].Filename != _T(""))
		{
			hr = m_theImage.Load(m_TextFileInfo[i].Filename);    
		}else if(m_TextFileInfo[i].Content != _T(""))
		{
			std::string PngContentString = CW2A(m_TextFileInfo[i].Content); 
			std::vector<BYTE> PngContent;
			MagicView::CommonLib::base64::decode(PngContentString.begin(), PngContentString.end(), std::back_inserter(PngContent));

			hr = E_FAIL;
			HGLOBAL hGlobal  =  GlobalAlloc(GMEM_MOVEABLE, PngContent.size() * sizeof(PngContent[0]));
			if(hGlobal != NULL)
			{
				void *  pData  =  GlobalLock(hGlobal);
				if(pData != NULL)
				{
					memcpy(pData, &PngContent[0], PngContent.size() * sizeof(PngContent[0]));
					GlobalUnlock(hGlobal); 
					IStream *  pStream  =  NULL;
					if  (CreateStreamOnHGlobal(hGlobal, TRUE,  & pStream)  ==  S_OK)
					{
						if (SUCCEEDED(m_theImage.Load(pStream)) && !m_theImage.IsNull())
						{
							hr = S_OK;
						}
						pStream -> Release();     
					}
				}

				GlobalFree(hGlobal);  
			}

		}else
		{
			hr = E_FAIL;
		}

		if(SUCCEEDED(hr))
		{
			TEXT_INFO NewText;
			HRESULT hr;
			NewText.TextFileScale = m_TextFileInfo[i].TextFileScale * m_ScaleRate;
			NewText.Filename = m_TextFileInfo[i].Filename;
			hr = UploadText(NewText, m_theImage);

			if(SUCCEEDED(hr))
			{   m_TextInfo.push_back(NewText);
			}
		}else
		{
			DbgMsg(_T("Error: failed to load text file %s"), m_TextFileInfo[i].Filename);
		}
	}

	return S_OK;
}

HRESULT S3TextFileObj::InvalidateDeviceObjects()
{
	for(DWORD i=0; i<m_TextInfo.size(); i++ )
	{
		SAFE_RELEASE(m_TextInfo[i].pTexture);
	}
	return S_OK;
}

HRESULT S3TextFileObj::RenderText(TEXT_INFO &TextInfo, float XPos, float YPos)
{
	RenderRect TextRect;

	// build basic render rect
	if(!m_bVerticalLayout)
	{
		float CurrentTv = 0;
		float LineHeight = 1.0f/TextInfo.WrappedLineCount;
		for(int i=0; i< TextInfo.WrappedLineCount; i++)
		{
			int CurrentWidth = min((TextInfo.ImageWidth - i * TextInfo.TextureWidth), TextInfo.TextureWidth);
			TextRect = RenderRect(RECTF(XPos, YPos, XPos + CurrentWidth * TextInfo.TextFileScale, YPos + TextInfo.ImageHeight * TextInfo.TextFileScale), RECTF(0, CurrentTv, (float)CurrentWidth / TextInfo.TextureWidth, CurrentTv + LineHeight)); 
			TextRect.bTransparent = TRUE;
			TextRect.pTexture = TextInfo.pTexture;
			TextRect.Clip(RECTF(0,0, (float)m_iWidth, (float)m_iHeight));

			if(TextRect.Width() > 0 && TextRect.Height() > 0)
			{
				m_RenderRect.push_back(TextRect); 
			}
			XPos += TextInfo.TextureWidth * TextInfo.TextFileScale;
			CurrentTv += LineHeight;
		}
	}else
	{
		float CurrentTv = 0;
		float LineHeight = 1.0f/TextInfo.WrappedLineCount;
		for(int i=0; i< TextInfo.WrappedLineCount; i++)
		{
			int CurrentWidth = min((TextInfo.ImageWidth - i * TextInfo.TextureWidth), TextInfo.TextureWidth);

			TextRect.Position = RECTF(XPos, YPos, XPos + TextInfo.ImageHeight * TextInfo.TextFileScale, YPos + CurrentWidth * TextInfo.TextFileScale);
			TextRect.TextureCoordinate[0].tu = (float)CurrentWidth / TextInfo.TextureWidth;
			TextRect.TextureCoordinate[0].tv = CurrentTv + LineHeight;

			TextRect.TextureCoordinate[1].tu = 0;
			TextRect.TextureCoordinate[1].tv = CurrentTv + LineHeight;


			TextRect.TextureCoordinate[2].tu = (float)CurrentWidth / TextInfo.TextureWidth;
			TextRect.TextureCoordinate[2].tv = CurrentTv;

			TextRect.TextureCoordinate[3].tu = 0;
			TextRect.TextureCoordinate[3].tv = CurrentTv;

			TextRect.bTransparent = TRUE;
			TextRect.pTexture = TextInfo.pTexture;

			TextRect.Clip(RECTF(0,0, (float)m_iWidth, (float)m_iHeight));

			if(TextRect.Width() > 0 && TextRect.Height() > 0)
			{
				m_RenderRect.push_back(TextRect); 
			}

			YPos += TextInfo.TextureWidth * TextInfo.TextFileScale;
			CurrentTv += LineHeight;

		}
	}
	return S_OK;
}


HRESULT S3TextFileObj::PrepareRender()
{
	m_RenderRect.clear();

	RECTF TextDisplayRect;

	DWORD CurrentTime = timeGetTime();

	if(m_CurrentText == -1) return E_UNEXPECTED;

	if(m_CurrentText >= 0)
	{

		switch(m_TextDirection)
		{
		case S3S_TEXT_DIRECTION_RIGHT_LEFT:
			if(!m_bPaused)
			{
				m_CurrentXPos = (m_TextStartXPos - m_Speed * (CurrentTime - m_MessageStartTime) / 1000.0f);
			}

			if(m_CurrentXPos + GetCurTextLength() < 0)
			{
				DWORD TimeDelta = -(m_CurrentXPos + GetCurTextLength()) * 1000 / m_Speed;
				SwitchText(TimeDelta);
				m_CurrentXPos = (float)m_TextStartXPos;
			}

			break;
		case S3S_TEXT_DIRECTION_LEFT_RIGHT:
			if(!m_bPaused)
			{
				m_CurrentXPos = (m_TextStartXPos + m_Speed * (CurrentTime - m_MessageStartTime) / 1000.0f);
			}
			if(m_CurrentXPos > m_iWidth)
			{
				DWORD TimeDelta = (m_CurrentXPos - m_iWidth) * 1000 / m_Speed;

				SwitchText(TimeDelta);
				m_CurrentXPos = (float)m_TextStartXPos;
			}

			break;
		case S3S_TEXT_DIRECTION_DOWN_UP:
			if(!m_bPaused)
			{
				m_CurrentYPos = (m_TextStartYPos - m_Speed * (CurrentTime - m_MessageStartTime) / 1000.0f);
			}

			if(!m_bVerticalLayout)
			{
				if(m_CurrentYPos + GetCurTextHeight() < 0)
				{
					DWORD TimeDelta = -(m_CurrentYPos + GetCurTextHeight())* 1000 / m_Speed;

					SwitchText(TimeDelta);
					m_CurrentYPos = (float)m_TextStartYPos;
				}
			}
			if(m_bVerticalLayout)
			{
				if(m_CurrentYPos + GetCurTextLength()  <  0)
				{
					DWORD TimeDelta = -(m_CurrentYPos + GetCurTextLength())* 1000 / m_Speed;

					SwitchText(TimeDelta);
					m_CurrentYPos = (float)m_TextStartYPos;
				}
			}
			break;
		case S3S_TEXT_DIRECTION_UP_DOWN:
			if(!m_bPaused)
			{
				m_CurrentYPos = (m_TextStartYPos + m_Speed * (CurrentTime - m_MessageStartTime) / 1000.0f);
			}

			if(m_CurrentYPos > m_iHeight)
			{
				DWORD TimeDelta = (m_CurrentYPos - m_iHeight) * 1000 / m_Speed;

				SwitchText(TimeDelta);
				m_CurrentYPos = (float)m_TextStartYPos;
			}

			break;
		case S3S_TEXT_DIRECTION_NONE:
			if(CurrentTime - m_MessageStartTime > 10000)
			{
				SwitchText(0);
			}
			break;
		}

		RenderText(m_TextInfo[m_CurrentText], m_CurrentXPos, m_CurrentYPos);

		if(!m_bVerticalLayout)
		{
			TextDisplayRect.left = (FLOAT)m_CurrentXPos;
			TextDisplayRect.right = (FLOAT)(m_CurrentXPos + GetCurTextLength());
			TextDisplayRect.top = (FLOAT)m_CurrentYPos;
			TextDisplayRect.bottom = (FLOAT)(m_CurrentYPos + GetCurTextHeight());
		}else
		{
			TextDisplayRect.left = (FLOAT)m_CurrentXPos;
			TextDisplayRect.right = (FLOAT)(m_CurrentXPos + GetCurTextHeight());
			TextDisplayRect.top = (FLOAT)m_CurrentYPos;
			TextDisplayRect.bottom = (FLOAT)(m_CurrentYPos + GetCurTextLength());
		}

	}else
	{
		TextDisplayRect.left = 0;
		TextDisplayRect.top = 0;
		TextDisplayRect.right = 0;
		TextDisplayRect.bottom = 0;
	}


	TextDisplayRect.Clip(RECTF(0,0, (float)m_iWidth, (float)m_iHeight));

	// fill outside blank
	if(TextDisplayRect.left > 0)
	{
		RenderRect MarginRenderRect(RECTF(0, 0, TextDisplayRect.left, (float)m_iHeight), RECTF(0.0f, 0.0f, 0.0f, 0.0f));
		MarginRenderRect.bTransparent = TRUE;
		MarginRenderRect.pTexture = m_TextInfo[m_CurrentText].pTexture;

		m_RenderRect.push_back(MarginRenderRect);
	}

	// fill outside blank
	if(TextDisplayRect.top > 0)
	{
		RenderRect MarginRenderRect(RECTF(TextDisplayRect.left, 0, (float)m_iWidth, TextDisplayRect.top), RECTF(0.0f, 0.0f, 0.0f, 0.0f));
		MarginRenderRect.bTransparent = TRUE;
		MarginRenderRect.pTexture = m_TextInfo[m_CurrentText].pTexture;

		m_RenderRect.push_back(MarginRenderRect);
	}

	if(TextDisplayRect.bottom  <  m_iHeight)
	{
		RenderRect MarginRenderRect(RECTF(TextDisplayRect.left, TextDisplayRect.bottom, (float)m_iWidth, (float)m_iHeight), RECTF(0.0f, 0.0f, 0.0f, 0.0f));
		MarginRenderRect.bTransparent = TRUE;
		MarginRenderRect.pTexture = m_TextInfo[m_CurrentText].pTexture;

		m_RenderRect.push_back(MarginRenderRect);
	}

	if(TextDisplayRect.right < m_iWidth)
	{
		RenderRect MarginRenderRect(RECTF(TextDisplayRect.right, TextDisplayRect.top, (float)m_iWidth, TextDisplayRect.bottom), RECTF(0.0f, 0.0f, 0.0f, 0.0f));
		MarginRenderRect.bTransparent = TRUE;
		MarginRenderRect.pTexture = m_TextInfo[m_CurrentText].pTexture;

		m_RenderRect.push_back(MarginRenderRect);
	}

	return S_OK;
}

HRESULT S3TextFileObj::EndRender()
{
	return S_OK;
}


HRESULT S3TextFileObj::SwitchText(DWORD TimeDelta)
{

	if(m_TextInfo.size() == 0) return E_UNEXPECTED;

	m_CurrentText = (m_CurrentText + 1) % m_TextInfo.size();

	m_MessageStartTime = timeGetTime();
	m_MessageStartTime -= TimeDelta;

	switch(m_TextDirection)
	{
	case S3S_TEXT_DIRECTION_RIGHT_LEFT:
		m_TextStartXPos = (int)m_iWidth;
		m_TextStartYPos = (int)((m_iHeight - GetCurTextHeight()) / 2 + 0.5);
		break;
	case S3S_TEXT_DIRECTION_LEFT_RIGHT:
		m_TextStartXPos = -(int)GetCurTextLength() ;
		m_TextStartYPos = (int)((m_iHeight - GetCurTextHeight()) / 2 + 0.5);

		break;
	case S3S_TEXT_DIRECTION_DOWN_UP:
		if(!m_bVerticalLayout)
		{
			if(m_iWidth > GetCurTextLength())
			{
				m_TextStartXPos = (int)((m_iWidth - GetCurTextLength()) / 2 + 0.5);
			}else
			{
				m_TextStartXPos = 0;
			}

			m_TextStartYPos = (int)m_iHeight;
		}

		if(m_bVerticalLayout)
		{
			m_TextStartXPos = (int)((m_iWidth - GetCurTextHeight()) / 2 + 0.5);
			m_TextStartYPos = (int)m_iHeight;
		}

		break;
	case S3S_TEXT_DIRECTION_UP_DOWN:
		if(!m_bVerticalLayout)
		{
			if(m_iWidth > GetCurTextLength())
			{
				m_TextStartXPos = (int)((m_iWidth - GetCurTextLength()) / 2 + 0.5);
			}else
			{
				m_TextStartXPos = 0;
			}

			m_TextStartYPos = -(int)GetCurTextHeight();;
		}

		if(m_bVerticalLayout)
		{
			m_TextStartXPos = (int)((m_iWidth - GetCurTextHeight()) / 2 + 0.5);
			m_TextStartYPos = -(int)GetCurTextLength();
		}

		break;
	case S3S_TEXT_DIRECTION_NONE:
		if(!m_bVerticalLayout)
		{

			m_TextStartXPos = (int)((m_iWidth - GetCurTextLength()) / 2 + 0.5);
			m_TextStartYPos = (int)((m_iHeight - GetCurTextHeight()) / 2 + 0.5);
		}

		if(m_bVerticalLayout)
		{
			m_TextStartXPos = (int)((m_iWidth - GetCurTextHeight()) / 2 + 0.5);
			if(m_iHeight > GetCurTextLength())
			{
				m_TextStartYPos = (int)((m_iHeight - GetCurTextLength()) / 2 + 0.5);
			}else
			{
				m_TextStartYPos = 0;
			}
		}        
		break;
	}
	return S_OK;
}


float S3TextFileObj::GetCurTextLength()
{
	return (m_TextInfo[m_CurrentText].ImageWidth * m_TextInfo[m_CurrentText].TextFileScale);
}

float S3TextFileObj::GetCurTextHeight()
{
	return (m_TextInfo[m_CurrentText].ImageHeight * m_TextInfo[m_CurrentText].TextFileScale);
}

HRESULT S3TextFileObj::Pause()
{
	if(!m_bPaused)
	{
		DWORD CurrentTime = timeGetTime();
		m_MessageStartTime = CurrentTime - m_MessageStartTime;
		m_bPaused = TRUE;
	}
	return S_OK;
}

HRESULT S3TextFileObj::Resume()
{
	if(m_bPaused)
	{
		DWORD CurrentTime = timeGetTime();
		m_MessageStartTime = CurrentTime - m_MessageStartTime;
		m_bPaused = FALSE;
	}
	return S_OK;
}


HRESULT S3TextFileObj::Start()
{
	m_CurrentText = -1;
	SwitchText(0);
	m_CurrentXPos = (float)m_TextStartXPos;
	m_CurrentYPos = (float)m_TextStartYPos;
	return S_OK;
}
