#include "StdAfx.h"
#include "S3Utility.h"

S3Utility::S3Utility(void)
{
    m_bInitCaptureDeivces = FALSE;
}

S3Utility::~S3Utility(void)
{
}

NTSTATUS S3Utility::CallEscape(PS3ESC_REQUEST_PACKET_EX pRequestPacket)
{
	HINSTANCE hInst = LoadLibrary(TEXT("gdi32.dll"));
	if (NULL == hInst)
	{
		return STATUS_UNSUCCESSFUL;
	}

	PFND3DKMT_OPENADAPTERFROMHDC pfnKTOpenAdapterFromHdc = NULL;
	PFND3DKMT_CLOSEADAPTER pfnKTCloseAdapter             = NULL;
	PFND3DKMT_ESCAPE pfnKTEscape                         = NULL;;

	pfnKTOpenAdapterFromHdc = (PFND3DKMT_OPENADAPTERFROMHDC) GetProcAddress((HMODULE)hInst, "D3DKMTOpenAdapterFromHdc" );
	pfnKTCloseAdapter       = (PFND3DKMT_CLOSEADAPTER)GetProcAddress((HMODULE)hInst, "D3DKMTCloseAdapter" );
	pfnKTEscape             = (PFND3DKMT_ESCAPE)GetProcAddress((HMODULE)hInst, "D3DKMTEscape" );


	if (NULL == pfnKTOpenAdapterFromHdc 
		|| NULL == pfnKTCloseAdapter
		|| NULL == pfnKTEscape)
	{
		return STATUS_UNSUCCESSFUL;
	}    


	DISPLAY_DEVICE dd;
	memset(&dd, 0, sizeof (dd));
	dd.cb = sizeof dd;

	for (int i = 0; EnumDisplayDevicesW(NULL, i, &dd, 0); ++i) 
	{
		if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) 
		{
			break;
		}
	}


	HDC hdc = CreateDC (NULL, dd.DeviceName, NULL, NULL);
	if (NULL == hdc) 
	{
		return STATUS_UNSUCCESSFUL;
	}

	D3DKMT_OPENADAPTERFROMHDC OpenAdapterData = {0};
	OpenAdapterData.hDc = hdc;
	NTSTATUS hRes = (*pfnKTOpenAdapterFromHdc)( &OpenAdapterData );

	if(STATUS_SUCCESS == hRes)
	{
		D3DKMT_ESCAPE LHescape = {0};

		LHescape.hAdapter              = OpenAdapterData.hAdapter;
		LHescape.Type                  = D3DKMT_ESCAPE_DRIVERPRIVATE;
		LHescape.pPrivateDriverData    = pRequestPacket;
		LHescape.PrivateDriverDataSize = sizeof(*pRequestPacket);

		//call real escape
		hRes = (*pfnKTEscape)( &LHescape );
		if (STATUS_SUCCESS == hRes && 0x40000001 == pRequestPacket->IoControlCode)
		{
			hRes = STATUS_SUCCESS;
		}
		else
		{
			hRes = STATUS_UNSUCCESSFUL;
		}

		//close adapter
		D3DKMT_CLOSEADAPTER CloseAdapterData;
		CloseAdapterData.hAdapter = OpenAdapterData.hAdapter;
		(*pfnKTCloseAdapter)( &CloseAdapterData );
	}
	else 
	{
		return STATUS_UNSUCCESSFUL;
	}

	DeleteDC(hdc);
	return hRes;
}

DWORD S3Utility::GetDisplayCount()
{
	DWORD nRet = 1;

	S3ESC_REQUEST_PACKET_EX RequestPacket = {0};
	ULONG EscapeSubId = S3_UTIL_Sub_Get_Video_Wall_Connections;

	int nOutputBufferSize = sizeof(QUERY_VIDEO_WALL_ADAPTER_MONITOR_POSITION_INFO) / sizeof(DWORD);
	LPDWORD lpdwTmpOutputBuffer = new DWORD[ nOutputBufferSize ];

	RequestPacket.IoControlCode = S3_UTIL_FUNCTION;
	RequestPacket.dwTag = S3ESC_REQUEST_FROM_UTIL;
	RequestPacket.InputBuffer = &EscapeSubId;
	RequestPacket.InputBufferLength = sizeof(EscapeSubId);
	RequestPacket.OutputBuffer = lpdwTmpOutputBuffer;
	RequestPacket.OutputBufferLength = sizeof(QUERY_VIDEO_WALL_ADAPTER_MONITOR_POSITION_INFO);
	RequestPacket.VidPnSourceId = 0;

	QUERY_VIDEO_WALL_ADAPTER_MONITOR_POSITION_INFO info;
	if(STATUS_SUCCESS == CallEscape(&RequestPacket))
	{		
		memcpy(&info, lpdwTmpOutputBuffer, sizeof(QUERY_VIDEO_WALL_ADAPTER_MONITOR_POSITION_INFO));
		nRet = info.dwMonitorCount;
	}	

	delete[] lpdwTmpOutputBuffer;
	return nRet;
}


BOOL S3Utility::GetCaptureDevice(UINT index, CaptureDevice& device)
{
    if (!m_bInitCaptureDeivces)
    {
        SysCall::EnumCaptureValidDevices(m_CaptureDevices);
        m_bInitCaptureDeivces = TRUE;
    }

    if (index < m_CaptureDevices.size())
    {
        device = m_CaptureDevices[index];
        return TRUE;
    }

    return FALSE;
}