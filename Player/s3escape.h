/*****************************************************************************
| File:		s3escape.h
|
| Purpose:
|	This file contains function prototypes and defines for s3escape
|	service routines.
|
| Copyright (c) 1996-2001 by S3, Inc. All Rights Reserved. S3 Confidential.
| Copyright (c) 2001-2003 S3 Graphics, Inc. All rights reserved.
| Copyright (c) 2004-2010 S3 Graphics Co., Ltd. All Rights Reserved.
|
| This is UNPUBLISHED PROPRIETARY SOURCE CODE of S3 Graphics, Inc.;
| the contents of this file may not be disclosed to third parties, copied or
| duplicated in any form, in whole or in part, without the prior written
| permission of S3 Graphics, Inc.
|
| RESTRICTED RIGHTS LEGEND:
| Use, duplication or disclosure by the Government is subject to restrictions
| as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
| and Computer Software clause at DFARS 252.227-7013, and/or in similar or
| successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
| rights reserved under the Copyright Laws of the United States.
|
*****************************************************************************/
/*****************************************************************************
| Version:	1.30
| History:
|  04-Oct2008-DukeDu (v1.30)
|	Added escape(215) to let driver decide if can set primary.
|  06-Mar2008-Wyoung (v1.29)
|	Added defines for DisplayPort 1 to 4, which we'll need soon.
|  25-Feb2008-DukeDu (v1.28)
|	Correct S3F_RecommendV2(4) and Add S3F_Event_SCREENSAVER feature.
|  22-Feb2008-DukeDu (v1.27)
|	Add PrepareForInvalidateVidPn() and WaitInvalidateVidPnFinish() to fix the confirm messagebox
|	is cutted issue and can't swap primary issue.
|  14-Dec2007-Wyoung (v1.26)
|	Shortened S3_HDMI_SIGNAL_CURRENT_USED to S3_HDMI_SIGNAL_CURRENT.
|	Shortened S3_HDMI_FORMAT_CURRENT_USED to S3_HDMI_FORMAT_CURRENT.
|  12-Dec2007-Wyoung (v1.25)
|	Added S3_UTIL_DEVICE_TV1 so S3_UTIL_DEV_ALL_TVS macro works.
|  15-Nov2007-Wyoung (v1.24)
|	Added a struct for Set Driver Event, to make sure we can constrain
|		the event name size to 128 bytes.
|  13-Nov2007-Wyoung (v1.23)
|	Added S3_UTIL_Sub_SetDriverEventHandle for new escape for HPD.
|  03-Oct2007-Wyoung (v1.22)
|	Added some S3_UTIL_DEV_ALL** defines for convenience, so it's easier
|		to check for a type of device (all CRTs, all LCDs, etc.)
|  05-Sep2007-Wyoung (v1.21)
|	Changed S3DRVNAMES2 struct's Metal driver to DX10 driver for bug11974(?)
|  16-Aug2007-DukeDu (v1.20)
|	Added define for "Support 1080P" feature.
|  19-Jul2007-Wyoung (v1.19)
|	Fixed spelling of S3_TVCAPS_CUSOMIZE_MODE to S3_TVCAPS_CUSTOM_MODES.
|  18-Jul2007-Wyoung (v1.18)
|	Updated copyright year of this file.
|	Fixed spelling of S3_CONSTRACTION_FACTOR to S3_CONTRACTION_FACTOR.
|  05-Jul2007-Wyoung (v1.17)
|	Added the driver event and rotation defines, which are needed by
|		more than one utility.
|  27-Jun2007-DukeDu (v1.16)
|	Added IsAnotherAdaptorView() to check if a specific view in on the other adaptor.
|  26-Jun2007-DukeDu (v1.15)
|	Check escape(185) Ver1 support before call escape(185): Set Mode.
|  25-Jun2007-DukeDu (v1.14)
|	Added S3_UTIL_NEED_INVALIDATEVIDPN flag for return value for vista only.
|	Utility will call InvalidateVidPn() with the same parameters
|	if driver returned this flag.
|  12-Dec2006-Wyoung (v1.13)
|	Added SECAM TV standard defines (for D2).
|  07-Dec2006-DukeDu (v1.12)
|	Added S3_UTIL_GetCachedConnectionStatus() to get cached connection status.
|  08-Nov2006-Wyoung (v1.11)
|	Renamed S3DRVNAMES2 fields to make it clear what they are in Vista.
|  03-Nov2006-Wyoung (v1.10)
|	Split into new file s3esc_display.h, leave: S3_UTIL_GetDisplayControl,
|		S3_UTIL_SetDisplayControl, S3_UTIL_GetConnectionStatus*,
|		S3_UTIL_GetPrimaryDevice, S3_UTIL_SetPrimaryDevice,
|		S3_UTIL_GetBIOSVersion, S3_UTIL_GetVideoMemSize,
|		S3_UTIL_SetAppHWND, S3_UTIL_QueryDeviceSupport,
|		S3_UTIL_QueryLargeDesktopSupport, S3_UTIL_GetDeviceInfo,
|		S3_UTIL_GetChipInfo, S3_UTIL_GetDevBitsOnIGA, S3_UTIL_GetBIOSDate,
|		S3_UTIL_GetDriverFileNames, S3_UTIL_GetDriverRegKeyName,
|		S3_UTIL_GetHardwareInfo, S3_UTIL_GetDriverEvents,
|		S3_UTIL_GetDeviceUsable, S3_UTIL_GetIGAMask,
|		S3_UTIL_GetCurrentSMAMemSize, S3_UTIL_SetCurrentSMAMemSize
|  30-Oct2006-Wyoung (v1.09)
|	Added 0x3000 subfunction codes from S3PowerWise/Turbo/Thermal.
|  30-Oct2006-Wyoung (v1.09)
|	Moved contents for 0x3000 escape from S3Display's sys_call.h.
|  09-Nov2005-Wyoung (v1.08)
|	Fixed DVI4 define to be 2000h, not 1000h (1000h is DVI3).
|  04-Nov2005-Wyoung (v1.07)
|	Added history section to file.
|	Removed the obsolete connect and support bit defines.
|	Added DVI3 define.  Also add placeholder define for DVI4, since
|		it sounds like future hardware may support it.
|  17-Jun2005-Wyoung (v1.01)
|	Fixed some Longhorn stuff in code side (s3escape.cpp).
|  16-Jun2005-Wyoung (v1.00)
|	Initial shared version of s3escape.h.
+---------------------------------------------------------------------------*/

#ifndef _S3ESCAPE_H_
#define _S3ESCAPE_H_

#ifdef _MSC_VER   
#pragma warning(disable:4201)
#endif

//#include "s3displayconfig.h"
//**********************************************
//	LONGHORN-specific stuff
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                          (0x00000000L) // ntsubauth
#endif

#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)
#endif

#ifndef NT_SUCCESS
#define NTSTATUS LONG
#define NT_SUCCESS(status) ((NTSTATUS)(status)>=0)
#endif

#ifdef _DEBUG
#define _DEBUG_SIMUL
#endif

#define ESC_SIG		0x5554494c	// 'UTIL' signature used in Vista escapes

extern BOOL
	g_fLHgood;

extern BOOL
	g_fW7good;

//**********************************************
// Structure used to pass escape requests to driver
//   in Longhorn.  This is really the same as
//   an existing structure called VIDEO_REQUEST_PACKET,
//   but renamed to be sure there won't be conflicts.
typedef struct _S3ESC_REQUEST_PACKET
{

	// The IO control code passed to driver by caller.
	//   In our case, it's the escape function (i.e. 0x3000)
	ULONG
		IoControlCode;

	// Pointer to a status block provided by the caller. This should be
	//   filled out by the callee with the appropriate information.
	//   Probably unused for utility escape calls.
	LPVOID		// Should really be PSTATUS_BLOCK
		StatusBlock;

	// Pointer to an input buffer which contains the information
	//   passed in by the caller.
	PVOID
		InputBuffer;

	// Size of the input buffer
	ULONG
		InputBufferLength;

	// Pointer to an output buffer into which the data returned
	//   to the caller should be stored.
	PVOID
		OutputBuffer;

	// Length of the output buffer. This buffer can not be grown
	//   by the callee.
	ULONG
		OutputBufferLength;

} S3ESC_REQUEST_PACKET, *PS3ESC_REQUEST_PACKET;

typedef UINT	D3DDDI_VIDEO_PRESENT_SOURCE_ID;

//**********************************************
// Enhanced S3ESC_REQUEST_PACKET Structure
//   This structure enhanced by append a vidpn source id
typedef struct _S3ESC_REQUEST_PACKET_EX
{

	// The IO control code passed to driver by caller.
	//   In our case, it's the escape function (i.e. 0x3000)
	ULONG
		IoControlCode;

	// Pointer to a status block provided by the caller. This should be
	//   filled out by the callee with the appropriate information.
	//   Probably unused for utility escape calls.
	LPVOID		// Should really be PSTATUS_BLOCK
		StatusBlock;

	// Pointer to an input buffer which contains the information
	//   passed in by the caller.
	PVOID
		InputBuffer;

	// Size of the input buffer
	ULONG
		InputBufferLength;

	// Pointer to an output buffer into which the data returned
	//   to the caller should be stored.
	PVOID
		OutputBuffer;

	// Length of the output buffer. This buffer can not be grown
	//   by the callee.
	ULONG
		OutputBufferLength;

	// Should always be "UTIL" if call from utility side
	DWORD
		dwTag;

	// VidPn source id
	D3DDDI_VIDEO_PRESENT_SOURCE_ID
		VidPnSourceId;

	// Used to sync with driver for invalidatevidpn() issue.
	BYTE
		bToken;

	// Reserved for future use.
	BYTE
		dwReserved[3];

} S3ESC_REQUEST_PACKET_EX, *PS3ESC_REQUEST_PACKET_EX;


typedef UINT	D3DKMT_HANDLE;
typedef UINT	D3DDDI_VIDEO_PRESENT_SOURCE_ID;

typedef enum _KMTQUERYADAPTERINFOTYPE
{
	KMTQAITYPE_UMDRIVERPRIVATE = 0,
	KMTQAITYPE_UMDRIVERNAME    = 1,
	KMTQAITYPE_UMOPENGLINFO    = 2,
	KMTQAITYPE_GETSEGMENTSIZE  = 3,
	KMTQAITYPE_ADAPTERGUID     = 4,
	KMTQAITYPE_FLIPQUEUEINFO   = 5,
} KMTQUERYADAPTERINFOTYPE;

typedef struct _D3DKMT_QUERYADAPTERINFO
{
	D3DKMT_HANDLE		hAdapter;
	KMTQUERYADAPTERINFOTYPE	Type;
	VOID*			pPrivateDriverData;
	UINT			PrivateDriverDataSize;
} D3DKMT_QUERYADAPTERINFO;

typedef struct _D3DDDI_ESCAPEFLAGS
{
	union
	{
		struct
		{
			UINT HardwareAccess : 1;	// 0x00000001
			UINT Reserved :31;		// 0xFFFFFFFE
		};
		UINT Value;
	};
} D3DDDI_ESCAPEFLAGS;


typedef enum _D3DKMT_ESCAPETYPE
{
	D3DKMT_ESCAPE_DRIVERPRIVATE	= 0,
	D3DKMT_ESCAPE_VIDMM		= 1,
	D3DKMT_ESCAPE_TDRDBGCTRL	= 2,
	D3DKMT_ESCAPE_VIDSCH		= 3,
} D3DKMT_ESCAPETYPE;

typedef struct _D3DKMT_ESCAPE
{
	D3DKMT_HANDLE		hAdapter;               // in: adapter handle
	D3DKMT_HANDLE		hDevice;                // in: device handle [Optional]
	D3DKMT_ESCAPETYPE	Type;                   // in: escape type.
	D3DDDI_ESCAPEFLAGS	Flags;			// in: flags (added 19-Jan2006)
	VOID*			pPrivateDriverData;     // in/out: escape data
	UINT			PrivateDriverDataSize;  // in: size of escape data
	D3DKMT_HANDLE		hContext;               // in: context handle [Optional]
} D3DKMT_ESCAPE;


typedef struct _D3DKMT_OPENADAPTERFROMHDC
{
	HDC				hDc;
	D3DKMT_HANDLE			hAdapter;
	LUID			AdapterLuid;    // out: adapter LUID
	D3DDDI_VIDEO_PRESENT_SOURCE_ID	VidPnSourceId;
} D3DKMT_OPENADAPTERFROMHDC;


typedef struct _D3DKMT_CLOSEADAPTER
{
	D3DKMT_HANDLE			hAdapter;
} D3DKMT_CLOSEADAPTER;

typedef struct _D3DKMT_INVALIDATEACTIVEVIDPN
{
	D3DKMT_HANDLE		hAdapter;               // in: adapter handle
	VOID*			pPrivateDriverData;     // in/out: escape data
	UINT			PrivateDriverDataSize;  // in: size of escape data
} D3DKMT_INVALIDATEACTIVEVIDPN;

// Structure for use in D3DKMTPollDisplayChildren(), to detect which
//   devices are connected in Windows 7 (should also work in Vista).
typedef struct _D3DKMT_POLLDISPLAYCHILDREN
{
	D3DKMT_HANDLE	hAdapter;
	UINT		NonDestructiveOnly: 1;		// If TRUE, do poll with no screen flicker
	UINT		SynchronousPolling: 1;		// If TRUE, limit call to 1 second
	UINT		DisableModeReset: 1;		// If TRUE, miniport shouldn't do anything.
	UINT		PollAllAdapters: 1;		// If TRUE, poll all adapters, not just specified one
	UINT		PollInterruptible: 1;		// Allow polling of HPD-aware devices
	UINT		Reserved: 27;
} D3DKMT_POLLDISPLAYCHILDREN;

//************************************************
// Internal structure for Set Driver Event
typedef struct _s3SetDrvEvent
{
	DWORD
		dwSubFunc,
		dwSetOp,
		dwType;
	WCHAR
		wszEventName[128];
} S3SETDRVEVENT, FAR* LPS3SETDRVEVENT;

typedef struct tagS3DevConnectInfo	// Connector info block
{
	DWORD
		dwRealMonitorType,	// True monitor type (if this is zero,
		//   it means the driver has no info)
		dwConnectorType;	// Type of connector used for device
	//   (If zero, driver doesn¡¯t know).
	DWORD
		dwRealMonitorIndex;	// need S3F_QueryRealDeviceIndex, otherwise should be reserved.
	DWORD
		dwReserved[61];	// Reserved, currently unused.
} S3DEVCONNECTINFO, FAR* LPS3DEVCONNECTINFO;


typedef struct tagHDMISignal
{
	DWORD	Signal;	// Signal
	//	00 RGB signal
	//	01 YcbCr 4:2:2 signal
	//	10 YcbCr 4:4:4 signal
	//	11 Not used, for future use
}HDMISignal, *PHDMISignal;


typedef struct tagHDMIFormat
{
	DWORD	Number;			// Format number
	WORD 	XRes;			// X resolution
	WORD 	YRes;			// Y resolution
	WORD    Interlace;		// Interlace
	WORD	RefRate[2];		// Refresh rate
	WORD 	AspectRatio;		// 0 is 4:3, 1 is 16:9
	DWORD 	dwFlags;		// bit0 = 0 means CEA mode, bit0=1 means PC mode, other bits reserved
}HDMIFormat, *PHDMIFormat;


typedef struct tagDeviceSetting
{
	DWORD	dwDeviceBit;	// S3_UTIL_DEV**
	DWORD	dwFunctionID;	// Function ID
	DWORD	dwSettingID;	// Setting ID
	DWORD	dwReserved;	// Reserved member for future use.
} DEVICE_SETTING, *LPDEVICE_SETTING;


typedef struct tagSettingRange
{
	DWORD	dwMin;
	DWORD	dwMax;
	DWORD	dwDefault;
	DWORD	dwStep;
	DWORD	dwDenominator;
	DWORD	dwReserved1;
	DWORD	dwReserved2;
} SETTING_RANGE, *LPSETTING_RANGE;

typedef struct tagVideoWallLayout
{
	DWORD	dwLayoutID;
	DWORD	dwXNativeRes;		// Source mode X resolution without tuning, border.
	DWORD	dwYNativeRes;		// Source mode Y resolution without tuning, border.
	DWORD	dwMaxXRes;		// Max source mode X resolution after tuning.
	DWORD	dwMaxYRes;		// Max source mode Y Resolution after tuning.
	BYTE	szLayoutName[64];	// ANSI, Non-unicode
	DWORD	dwReserved[10];
} VIDEO_WALL_LAYOUT, *LPVIDEO_WALL_LAYOUT;

typedef struct tagVideoWallCellPosition
{
	DWORD	dwFields;
	DWORD	dwCellID;    	// Used to query all cells information.
	RECT	rectPosition;	// Position related with oriental.
	DWORD	dwLeftBorder;	// Left border size.
	DWORD	dwTopBorder;	// Top border size.
	DWORD	dwRightBorder;	// Right border size.
	DWORD	dwBottomBorder;	// Bottom border size.
	DWORD	dwXStep;	// each step, the x position can change.
	DWORD	dwYStep;	// each step, the x position can change.
	DWORD	dwXRatio;	// the ratio that a cell enlarge in width.
	DWORD	dwYRatio;	// the ratio that a cell enlarge in height.
	DWORD	dwFieldsDisallowChange;	// see dwFields.
	DWORD	dwReserved[2];
} VIDEO_WALL_CELL_POSITION, *LPVIDEO_WALL_CELL_POSITION;

typedef struct tagNewInterfaceEscape
{
	DWORD	dwMainFunction : 16;
	DWORD	dwSubFunction : 16;
	DWORD	dwReserved;
} NEW_INTERFACE_ESCAPE, *LPNEW_INTERFACE_ESCAPE;

#pragma region ExternalFunctionPrototypes

typedef HRESULT (WINAPI *PFND3DKMT_POLLDISPLAYCHILDREN)( IN CONST D3DKMT_POLLDISPLAYCHILDREN*);
typedef HRESULT (WINAPI *PFND3DKMT_ESCAPE)(D3DKMT_ESCAPE*);
typedef HRESULT (WINAPI *PFND3DKMT_OPENADAPTERFROMHDC)(D3DKMT_OPENADAPTERFROMHDC*);
typedef HRESULT (WINAPI *PFND3DKMT_CLOSEADAPTER)(D3DKMT_CLOSEADAPTER*);
typedef HRESULT (WINAPI *PFND3DKMT_QUERYADAPTERINFO)(CONST D3DKMT_QUERYADAPTERINFO*);
typedef HRESULT (WINAPI *PFND3DKMT_INVALIDATEACTIVEVIDPN)(CONST D3DKMT_INVALIDATEACTIVEVIDPN*);

// 17-Jul2008-dukedu: Maybe will call external s3escape.dll someday.
typedef HDC (WINAPI *PFNCREATEDC)( LPCSTR, LPCSTR, LPCSTR, CONST DEVMODEA* );
typedef BOOL (WINAPI *PFNDELETEDC)( HDC );
typedef int (WINAPI *PFNEXTESCAPE)(	HDC, int, int, LPCSTR, int, LPSTR );
typedef LONG (WINAPI *PFCHANGEDISPLAYSETTINGS)( LPDEVMODEA, DWORD );
typedef LONG (WINAPI *PFNCHANGEDISPLAYSETTINGSEX)( LPCSTR, LPDEVMODEA, HWND, DWORD, LPVOID );
typedef BOOL (WINAPI *PFNENUMDISPLAYSETTINGS)( LPCSTR, DWORD, LPDEVMODEA );
typedef BOOL (WINAPI *PFNENUMDISPLAYSETTINGSEX)( LPCSTR, DWORD, LPDEVMODEA, DWORD );
typedef BOOL (WINAPI *PFNENUMDISPLAYDEVICE)(LPCSTR, DWORD, PDISPLAY_DEVICEA, DWORD);

#pragma endregion ExternalFunctionPrototypes
#ifdef	__cplusplus
extern "C" {
#endif

/*---------------------[ Defines: Escape function codes ]--------------------*/
// MessageId: STATUS_GRAPHICS_NO_RECOMMENDED_FUNCTIONAL_VIDPN
// Miniport does not have any recommendation regarding the request to provide a functional VidPN given the current display adapter configuration.
#define STATUS_GRAPHICS_NO_RECOMMENDED_FUNCTIONAL_VIDPN 0xC01E0323

	//************************************************
	// Possible return codes from driver escapes
#define S3_UTIL_FAILURE		0
#define S3_UTIL_SUCCESS		1
#define S3_UTIL_READONLY	0x10
#define S3_UTIL_NOT_IMPLEMENTED	0xFFFFFFFF
#define S3_UTIL_DRIVER_HANDLED	0x40000000	// for vista only. indicate driver has processed.
#define S3_UTIL_NEED_ACCESSHW	0x04000000	// for vista only. indicate driver need access hardware.
#define S3_UTIL_NEED_INVALIDATEVIDPN	0x00000800	// vista only, if driver return this flag,
							// call invalidatevidpn() using the same parameter again.

	// Possible return codes from SetDisplayControl
#define S3_UTIL_ERRORCODE	( S3_UTIL_CONFIGPROBLEM | S3_UTIL_TEMPPROBLEM | S3_UTIL_APPBLOCK \
	| S3_UTIL_HIDEMODE | S3_UTIL_BAD_ROTATE | S3_UTIL_BAD_BPP_ROTATE \
	| S3_UTIL_CLOSE_PAGES | S3_UTIL_BUFFER_TOO_SMALL )
#define S3_UTIL_CONFIGPROBLEM	0x0002
#define S3_UTIL_TEMPPROBLEM	0x0004
#define S3_UTIL_APPBLOCK	0x0008		// Rotation blocked by DDraw/D3D/OGL, etc.
#define S3_UTIL_HIDEMODE	0x0010		// Rotation blocked by "hide mode".

#define S3_UTIL_NEEDMODESET	0x0100		// May be OR'd with other bits
#define S3_UTIL_NONEEDMODESET	0x0200		// Should not do mode set after switch device
#define S3_UTIL_NEEDREBOOT	0x0400		// If need reboot to take effect after call escape.
#define S3_UTIL_BAD_ROTATE	0x08000000	// Target mode is too small for rotation
						//  make it easier to show msgs
#define S3_UTIL_BAD_BPP_ROTATE	0x20000000	// Disallow rotation in 8bpp color depth
		// Returned from GetDisplayControl.  If this bit is set,
		//   something has happened that makes the driver think the
		//   Display Property Pages must be closed and re-opened.
#define S3_UTIL_CLOSE_PAGES	0x80000000

		// S3_UTIL_Sub_GetActiveTopology
#define S3_UTIL_BUFFER_TOO_SMALL	0x0008	// Output buffer is too small.

	//Information type, for escape(247)
#define S3_UTIL_INFO_LUID			0x00000001

	//************************************************
	// HDMI format & signal structure size.
#define HDMI_FORMAT_STRUCT_SIZE (sizeof(HDMIFormat))
#define HDMI_SIGNAL_STRUCT_SIZE (sizeof(HDMISignal))

#ifndef MAKEDWORD
#define MAKEDWORD(a, b)	((DWORD) (((WORD) ((DWORD_PTR)(a) & 0xffff) ) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16) )
#endif

#define BufSize(input, output)	(MAKEDWORD(input, output))

	//************************************************
	// Main escape function codes
#define S3_VIDEOGAMMA_FUNCTION		0x2666		// Video gamma (VT code)
#define S3_COLOR_FUNCTION               0x2880		// S3Color
#define S3_OVERLAY_FUNCTION             0x2884		// S3Overlay

#define S3ESC_REQUEST_FROM_UTIL 0x5554494c //it is ASCII value of "UTIL"


#define S3_UTIL_FUNCTION		0x3000		// Display switch/general info
#define S3_GAMMA_FUNCTION		0x3002		// Gamma-related
#define S3_SUPPORT_FUNCTION		0x3003		// Used to signal OS/WDDM compatibility
#define	S3_CRT_FUNCTION			0x300C		// CRT calibration
#define S3_CFG3D_FUNCTION		0x303D		// 3D process detection (S3Desktoys)
#define S3_CHROMO_FUNCTION		0x3200		// Chromovision


#define S3_VISTA_ENALBE_CLONE_PRIMAY	0x0000
#define S3_VISTA_DISABLE_SAMM			0x0001
#define S3_VISTA_ENABLE_SAMM			0x0002
#define S3_WIN7_ENABLE_CLONE_PRIMARY	S3_VISTA_ENALBE_CLONE_PRIMAY
#define S3_WIN7_ENABLE_SAMM_PRIMARY		0x0003

	//************************************************
	// For internal debugging support
#define S3_UTIL_SUPPORT_NONE		0
#define S3_UTIL_SUPPORT_YES		1
#define S3_UTIL_SUPPORT_HW		2
#define S3_UTIL_SUPPORT_STUBBED		4



	//************************************************
	// Defines for 	Get Connected IDs
#define	GETID_TARGET			0
#define GETID_SOURCE			1


	//************************************************
	// Defines for direction when translate target id and device bit
#define	DEVICEBIT_TO_TARGETID		0
#define	TARGETID_TO_DEVICEBIT		1


	//************************************************
	// Defines for CCD API wrappers
	// 27-Mar2009-Wyoung: Issue 45228.  We're not correctly initializing in
	//   MAMM, because S3_CCD_InfoValid() fails.  This is because these
	//   constants, originally hardcoded by Tom Liu, weren't enough for MAMM.
	//   Increase to 2 paths per adapter, and 2 modes per path.  Just to be safe,
	//   I'll allow enough for 4 adapters.
#define	MAX_CCD_PATH_ARRAY	8	// Basically, the maximum number of displays
	//   we can handle.  In MAMM with 2 adapters,
	//   this should be 4 (2 * number of adapters).
#define	MAX_CCD_MODE_ARRAY	16	// I think this is generally going to be
	//   2 * number of supported paths.

	//************************************************
	// Defines for escape(233), Max device combinations
#define MAX_DEVICE_COMBINATION	128

//************************************************
// Defines for escape(263), Max escape count
#define MAX_NEW_INTERFACE_PREFERRED_ESCAPE	128

	//************************************************
	// Defines for S3_UTIL_Sub_I2CReceive
#define	I2C_NO_FLAGS				0
#define I2C_DEVICE_TRANSMITS_DATA_LENGTH	1


	//************************************************
	// Defines for S3_UTIL_Sub_GetDeviceCombination
#define S3_COMBINATION_DEVICE_SWITCH	1
#define S3_COMBINATION_HOTKEY			2
#define S3_COMBINATION_HOTPLUG			4
#define S3_COMBINATION_SIMULTANEOUS	8

//************************************************
// Defines query hint for S3_UTIL_GetHybridChromeMode()
// and S3_UTIL_GetHybridChromeSwitchMethod().
#define		S3HYBRID_HINT_CURRENT			1
#define		S3HYBRID_HINT_DEFAULT			2

//************************************************
// Defines Hybrid Chrome Mode for S3_UTIL_GetHybridChromeMode()
// and S3_UTIL_GetHybridChromeSwitchMethod().
#define		S3HYBRID_HIGHPERFORMANCE			1
#define		S3HYBRID_POWERSAVING					2

//************************************************
// Defines Switch method for S3_UTIL_GetHybridChromeMode()
// and S3_UTIL_GetHybridChromeSwitchMethod().
#define		S3HYBRID_AUTO_METHOD			1
#define		S3HYBRID_MANUAL_METHOD		2

//************************************************
// Defines query result for S3_UTIL_CanChangeHybridChromeEngineStatus()
#define		S3HYBRID_NOT_ALLOW_CHANGE_ENGINE		0x00000000
#define		S3HYBRID_ALLOW_CHANGE_ENGINE				0x00000001

//************************************************
// Defines switch result for S3_UTIL_EndHybridChromeSwitch()
#define		S3HYBRID_FINISHED_FAILED							0x00000000
#define		S3HYBRID_FINISHED_SUCCESS						0x00000001

//************************************************
// Defines query result for S3_UTIL_CanSetPrimary()
#define		S3_SET_PRIMARY_NOT_ALLOW		0x00000000
#define		S3_SET_PRIMARY_ALLOW				0x00000001

//************************************************
// Defines hybrid engine
#define		S3HYBRID_HIGHPERFORMANCE_ENGINE		0x00000001
#define		S3HYBRID_POWERSAVING_ENGINE				0x00000002

//************************************************
// Defines hybrid engine status
#define		S3HYBRID_ENGINE_OFF									0x00000000
#define		S3HYBRID_ENGINE_ON									0x00000001
#define		S3HYBRID_ENGINE_IS_CHANGING_STATUS	0x00000002

//************************************************
// Defines span mode caps
#define	S3_UTIL_SPAN_MODE_CAPS_DISABLE		0x00000001
#define	S3_UTIL_SPAN_MODE_CAPS_VERTICAL		0x00000002
#define	S3_UTIL_SPAN_MODE_CAPS_HORIZONTAL		0x00000004

//************************************************
// Defines span mode
#define	S3_UTIL_SPAN_MODE_DISABLE		0x00000000
#define	S3_UTIL_SPAN_MODE_VERTICAL		0x00000001
#define	S3_UTIL_SPAN_MODE_HORIZONTAL		0x00000002

//************************************************
// Defines simultaneous state flag.
#define		SIMULTANEOUS_STATE_CURRENT		0
#define		SIMULTANEOUS_STATE_DEFAULT		1

// Defines for S3_SUPPORT_FUNCTION escapes
#define	S3_SUPP_Sub_SetUtilSupportLvl		1

// Device settings, Function IDs
#define S3_UTIL_DEV_PWM_SETTINGS		0x0050574D	// ¡°PWM¡±

// Device settings, Setting IDs
#define	S3_UTIL_DEV_PWM_BACKLIGHT		0x424B4C54	// ¡°BKLT¡±

// Per-cell video wall settings
#define S3_UTIL_ALL_CELLS		0xFFFFFFFF	// All cells in the video wall.
#define S3_UTIL_CURRENT_CELLS	0x00000000	// All cells in the video wall.

// Predefined layout ID
#define S3_UTIL_CURRENT_LAYOUTID		0x00000000	// Current layout id in the video wall.

// Video wall cell position flag
#define 	S3_UTIL_VIDEO_WALL_POSITION_CURRENT		0
#define 	S3_UTIL_VIDEO_WALL_POSITION_DEFAULT		1
#define 	S3_UTIL_VIDEO_WALL_POSITION_MIN				2
#define 	S3_UTIL_VIDEO_WALL_POSITION_MAX				3
#define 	S3_UTIL_VIDEO_WALL_POSITION_RANGE			150

	//************************************************
	// Defines for Get/Set DISPLAY, Get CONNECTION
	//   and DEVICE SUPPORTED escape calls.
#define S3_UTIL_Sub_GetPanelInfo		0
#define S3_UTIL_Sub_GetDisplayControl		1
#define S3_UTIL_Sub_SetDisplayControl		2
#define S3_UTIL_Sub_GetHorizontalState		3
#define S3_UTIL_Sub_SetHorizontalState		4
#define S3_UTIL_Sub_GetVerticalState		5
#define S3_UTIL_Sub_SetVerticalState		6
#define S3_UTIL_Sub_GetConnectionStatus		7
#define S3_UTIL_Sub_GetConnectionStatusEx	8	// Allows nondestructive polling
#define S3_UTIL_Sub_GetCRTPanningRes		9	// Returns error
#define S3_UTIL_Sub_SetCRTPanningRes		10	// Returns error

#define S3_UTIL_Sub_GetHotkeyOption		11
#define S3_UTIL_Sub_SetHotkeyOption		12
#define S3_UTIL_Sub_GetPrimaryDevice		15
#define S3_UTIL_Sub_SetPrimaryDevice		16
#define S3_UTIL_Sub_GetTVFFilterStatus		17
#define S3_UTIL_Sub_SetTVFFilterStatus		18
#define S3_UTIL_Sub_GetTVUnderscanStatus	19
#define S3_UTIL_Sub_SetTVUnderscanStatus	20
#define S3_UTIL_Sub_GetTVSignalType		21
#define S3_UTIL_Sub_SetTVSignalType		22
#define S3_UTIL_Sub_GetTVPosition		23
#define S3_UTIL_Sub_SetTVPosition		24
#define S3_UTIL_Sub_GetTVCenteringOption	25
#define S3_UTIL_Sub_SetTVCenteringOption	26

#define S3_UTIL_Sub_GetResolutionTable		27
#define S3_UTIL_Sub_GetChipID			28
#define S3_UTIL_Sub_GetBIOSVersion		29

#define S3_UTIL_Sub_GetVideoOutDevices		32
#define S3_UTIL_Sub_SetVideoOutDevices		33
#define S3_UTIL_Sub_GetTVStandard		34
#define S3_UTIL_Sub_SetTVStandard		35
#define S3_UTIL_Sub_GetVideoMemSize		36
#define S3_UTIL_Sub_SetAppHWND			37
#define S3_UTIL_Sub_CenterNow			38
#define S3_UTIL_Sub_GetUseDDCInfo		48
#define S3_UTIL_Sub_SetUseDDCInfo		49
#define S3_UTIL_Sub_QueryDeviceSupport		60
#define	S3_UTIL_Sub_QueryLargeDesktopSupport	61
#define S3_UTIL_Sub_ValidateHardwareOverlay	62

#define S3_UTIL_Sub_GetDevRefresh		65
#define S3_UTIL_Sub_SetDevRefresh		66

#define S3_UTIL_Sub_GetDeviceEdidInfo		67
#define S3_UTIL_Sub_GetContrast			69
#define S3_UTIL_Sub_SetContrast			70
#define S3_UTIL_Sub_SetDefaultContrast		71
#define S3_UTIL_Sub_GetSaturation		72
#define S3_UTIL_Sub_SetSaturation		73
#define S3_UTIL_Sub_SetDefaultSaturation	74
#define S3_UTIL_Sub_GetHue			75
#define S3_UTIL_Sub_SetHue			76
#define S3_UTIL_Sub_SetDefaultHue		77
#define S3_UTIL_Sub_GetBrightness		78
#define S3_UTIL_Sub_SetBrightness		79
#define S3_UTIL_Sub_SetDefaultBrightness	80
#define S3_UTIL_Sub_SetDefaultFFilter		81
#define S3_UTIL_Sub_GetSharpness		82
#define S3_UTIL_Sub_SetSharpness		83
#define S3_UTIL_Sub_SetDefaultSharpness		84
#define S3_UTIL_Sub_GetTextIntensity		85
#define S3_UTIL_Sub_SetTextIntensity		86
#define S3_UTIL_Sub_SetDefaultTextIntensity	87
#define S3_UTIL_Sub_GetHorzCtrl			88
#define S3_UTIL_Sub_SetHorzCtrl			89
#define S3_UTIL_Sub_SetDefaultHorzCtrl		90
#define S3_UTIL_Sub_GetVertCtrl			91
#define S3_UTIL_Sub_SetVertCtrl			92
#define S3_UTIL_Sub_SetDefaultVertCtrl		93
#define S3_UTIL_Sub_QueryHorzVertCtrl		94

#define S3_UTIL_Sub_GetChipInfo			95
#define	S3_UTIL_Sub_GetDisplayType		96
#define S3_UTIL_Sub_GetCrtPanningState		97
#define S3_UTIL_Sub_GetTvMaxLevels		98
#define S3_UTIL_Sub_GetTvMaxFFilterLevels	99
#define S3_UTIL_Sub_QueryTVSupportCaps		100
#define S3_UTIL_Sub_GetDFPInfo			101
#define S3_UTIL_Sub_GetBIOSDate			102
#define S3_UTIL_Sub_GetDriverFileNames		103

#define S3_UTIL_Sub_GetDriverRegKeyName		105
#define S3_UTIL_Sub_GetHardwareInfo		106

#define S3_UTIL_Sub_GetDriverEvents		107
#define S3_UTIL_Sub_GetDeviceUsable		108
#define S3_UTIL_Sub_GetIGAMask			110

#define S3_UTIL_Sub_GetLCDRefresh		113	// 21-May2001-Wyoung: Added
#define S3_UTIL_Sub_GetCurrentSMAMemSize	115
#define S3_UTIL_Sub_SetCurrentSMAMemSize	116

#define S3_UTIL_Sub_SaveTVSettings		117

#define S3_UTIL_Sub_GetDVIInfo			118	// 06-Jun2001-Wyoung: Added for DVI support
#define S3_UTIL_Sub_GetDVIExpansion		119
#define S3_UTIL_Sub_SetDVIExpansion		120
#define S3_UTIL_Sub_GetTvMaxRes			121

#define S3_UTIL_Sub_GetIGAOrientation		122	// 15-Aug2001-Wyoung
#define S3_UTIL_Sub_SetIGAOrientation		123	// 15-Aug2001-Wyoung
#define S3_UTIL_Sub_GetOrientationCaps		124	// 27-Sep2001-Wyoung

#define S3_UTIL_Sub_GetPowerModesInfo		125
#define S3_UTIL_Sub_GetPowerModeList		126
#define S3_UTIL_Sub_SetPowerMode		127

#define S3_UTIL_Sub_GetTVFeatureState		128	// 07-May2002-Wyoung
#define S3_UTIL_Sub_SetTVFeatureState		129
#define S3_UTIL_Sub_SetTVFeatureDefault		130
#define S3_UTIL_Sub_QueryTVPortList		131	// 10-May2002-Wyoung
#define S3_UTIL_Sub_GetTVPortEx			132	// 14-May2002-Wyoung
#define S3_UTIL_Sub_SetTVPortEx			133
#define S3_UTIL_Sub_GetTVAdaptFFilter		134
#define S3_UTIL_Sub_SetTVAdaptFFilter		135

#define S3_UTIL_Sub_GetClockRange		136	// 20-May2002-Wyoung
#define S3_UTIL_Sub_GetClockSpeed		137
#define S3_UTIL_Sub_SetClockSpeed		138

#define S3_UTIL_Sub_GetPanelInfoEx		139	// 26-Mar2003-Wyoung
#define S3_UTIL_Sub_GetPanelExpansion		140
#define S3_UTIL_Sub_SetPanelExpansion		141
#define S3_UTIL_Sub_GetScalingModeInfo		142

#define	S3_UTIL_Sub_GetHdtvNumSubModes		145
#define	S3_UTIL_Sub_GetHdtvSubModeList		146
#define S3_UTIL_Sub_SetHdtvSubMode		147
#define S3_UTIL_Sub_GetHotPlugState			 148   // RainLin [2004/06/16]: For hot-plug.
#define S3_UTIL_Sub_NotifyDriverWithEvent    149   // RainLin [2005/07/13]

#define S3_UTIL_Sub_GetRefLockState		150
#define S3_UTIL_Sub_SetRefLockState		151

#define S3_UTIL_Sub_GetChromoVisSupport		152	// 11-Dec2003-Wyoung
#define S3_UTIL_Sub_GetChromoVisState		153
#define S3_UTIL_Sub_SetChromoVisState		154
#define S3_UTIL_Sub_GetExpectedCPUPowerState 155   // JanusHsieh [2004/08/17]

#define S3_UTIL_Sub_GetThermalCaps		156	// Check for thermal sensor support
#define S3_UTIL_Sub_GetThermalRanges		157
#define S3_UTIL_Sub_GetThermalValues		158
#define S3_UTIL_Sub_SetThermalValues		159

#define S3_UTIL_Sub_QueryPanelOverdriveCaps	162
#define S3_UTIL_Sub_QueryHDMIFormatCaps_Number	163	// Query the driver to determine how many formats 
							// are supported
#define S3_UTIL_Sub_QueryHDMIFormatCaps	164		// Query the driver to retrieve the format information.
#define S3_UTIL_Sub_GetHDMIFormat 		165	// Query the driver to determine which HDMI 
							// format is currently used or specified.
#define S3_UTIL_Sub_SetHDMIFormat 		166	// Set HDMI format
#define S3_UTIL_Sub_QueryHDMISignalCaps_Number	167	// Query the driver to determine how many signals 
							// are supported
#define S3_UTIL_Sub_QueryHDMISignalCaps	168		// Query the driver to retrieve the signal information.
#define S3_UTIL_Sub_GetHDMISignal		169	// Query the driver to determine which HDMI 
							// signal is currently used or specified.
#define S3_UTIL_Sub_SetHDMISignal 		170	// Set HDMI Signal
#define S3_UTIL_Sub_QueryDeviceCaps 		171	// Query the device's caps
#define S3_UTIL_Sub_GetDeviceState 		172	// Get the device's state
#define S3_UTIL_Sub_SetDeviceState 		173	// Set the device's state

#define S3_UTIL_Sub_GetConnectedIDs		175	// Get connected id
#define S3_UTIL_Sub_GetActiveTopology		176	// Get active toptology
#define S3_UTIL_Sub_SetActiveTopology		177	// Set active toptology

#define S3_UTIL_Sub_AddCustomMode		178	// Tell the driver to add a customize mode
#define S3_UTIL_Sub_RemoveCustomMode		179	// Tell the driver to delete a customize mode
#define S3_UTIL_Sub_GetCustomModeCount		180	// Query driver to get the total count of customize mode
#define S3_UTIL_Sub_QueryCustomModeList		181	// Query driver to get customize mode list
#define S3_UTIL_Sub_SetViewPortSize		182	// Set view port size for HDTV/HDMI device
#define S3_UTIL_Sub_GetDeviceZoomStep		183	// Get the device¡¯s contraction step which means that, 
							// when zoom in/out the device¡¯s view port size, it must use the step
							// Now the step is zoomed in 1000 times

#define S3_UTIL_Sub_GetDeviceZoomResolution	184	// Get the device's current resolution after adjust size

#define S3_UTIL_Sub_SetMode			185	// Tell Vista to invalidate ViPdn because of device settings changing

#define S3_UTIL_Sub_I2COpen			186	// Open I2C bus
#define S3_UTIL_Sub_I2CClose			187	// Close I2C bus
#define S3_UTIL_Sub_I2CTransmit			188	// Write data to I2C bus
#define S3_UTIL_Sub_I2CReceive			189	// Read data from I2C bus
#define S3_UTIL_Sub_TargetID_to_DeviceBit	190	// Translate vidpn target id to device bit.
#define S3_UTIL_Sub_Get_SourceID		191	// Get source id for current view.
#define S3_UTIL_Sub_RecommendMonitorMode	192	// Get recommend mode for a monitor(device).
#define S3_UTIL_Sub_Get_Feature			193	// Get feature bits.
#define S3_UTIL_Sub_GetForceDetectedDevices	194	// Get the force detected devices
#define S3_UTIL_Sub_GetDeviceCombination	195	// Get the device combination information
#define S3_UTIL_Sub_GetDeviceHistoricState	196	// Get device states in history.
#define S3_UTIL_Sub_SetDefaultResolution   197		// Set default resolution for specified device.
#define S3_UTIL_Sub_LockStatus              198     //Lock-Unlock internal states
#define S3_UTIL_Sub_GetHotplugDeviceCombinationHistory              199     // Get hot plug enabled device history.
#define S3_UTIL_Sub_SetDisplayControlEx     200     // Enhanced
#define S3_UTIL_Sub_SetDriverEvent		201	// Add/Delete an event handle for driver notifications
#define S3_UTIL_Sub_GetHotplugStateEx       202     // Get hotplug state enhanced version.
#define S3_UTIL_Sub_RequestSyncToken        203     // Reqest a sync token from driver.
#define S3_UTIL_Sub_QuerySyncToken          204     // Query to see if a sync token raised.
#define S3_UTIL_Sub_PrepareSetDisplayControl	205	// Prepare set display control.
#define S3_UTIL_Sub_SetConfiguration	206 // restore mode and topology for known configuration
#define S3_UTIL_Sub_PowerSettingChanging	207 // Let driver change the power settings,
																		// however how to change, depend on the driver.
#define S3_UTIL_Sub_GetHybridChromeMode	208 // Query Hybrid Chrome mode, High performance or Power saving mode.
#define S3_UTIL_Sub_SetHybridChromeMode	209 // Set Hybrid Chrome mode, High performance or Power saving mode.
#define S3_UTIL_Sub_GetHybridChromeSwitchMethod	210 // Query Hybrid Chrome switch method, Auto or manual method.
#define S3_UTIL_Sub_SetHybridChromeSwitchMethod	211 // Set Hybrid Chrome switch method, Auto or manual method.
#define S3_UTIL_Sub_CanChangeHybridChromeEngineStatus	212 // Query driver to see if can change hybrid chrome engine status.
#define S3_UTIL_Sub_ChangeHybridChromeEngineStatus	213 // 	Call driver to change hybrid chrome engine status,
																						//if the hybrid chrome engine is on or off, then set hybrid chrome mode.
#define S3_UTIL_Sub_GetHybridChromeEngineStatus	214 // 	Query driver to see if a specified engine is ready.
#define S3_UTIL_Sub_CanSetPrimary					215 // Check with driver to see if can set primary device currently.
#define S3_UTIL_Sub_BeginHybridChromeSwitch	216 // Let driver know hybrid chrome switch started.
#define S3_UTIL_Sub_EndHybridChromeSwitch	217 // Let driver know hybrid chrome switch end with success or failure.
#define S3_UTIL_Sub_GetHybridChromeOutput	218 // Get hybrid chrome output connect to high performance or power saving.
#define S3_UTIL_Sub_SetHybridChromeOutput	219 // Set hybrid chrome output to high performance or power saving.
#define S3_UTIL_Sub_TurnOnMonitor				220 // Turn on monitor
#define S3_UTIL_Sub_TurnOffMonitor				221 // Turn off monitor
#define S3_UTIL_Sub_QueryGfxSystemInfo		222	// 27-Feb2009-Wyoung: Get more driver/hardware info
#define S3_UTIL_Sub_GetDevRefreshEx				223	// 07-Apr2009-Wyoung: New refresh rate call for Win7
#define S3_UTIL_Sub_OpenScript						224	// Open script to get script handle and size of script buffer.
#define S3_UTIL_Sub_QueryScriptContent			225	// Let driver fill the script buffer.
#define S3_UTIL_Sub_CloseScript						226	// Tell driver execute finished and let driver clean the script.
#define S3_UTIL_Sub_ReservedForScript			232	// Defined by driver, never use by utility.

#define S3_UTIL_Sub_GetTargetDevicesInfo		230	// 
#define S3_UTIL_Sub_SetTargetDevicesInfo		231	// 
#define S3_UTIL_Sub_GetDeviceCombinationEx	233	// Get the device combination to indicate which devices can be enabled
																				// at the same time, Enhanced escape(195).
#define S3_UTIL_Sub_GetSimultaneousDevices	234	// Get the simultaneous devices combo
#define S3_UTIL_Sub_SetSimultaneousDevices	235	// Set the simultaneous devices combo

#define S3_UTIL_Sub_FlexibleTimer					236	// Build up a flexible timer, 
														// driver can tell utility the interval and if have script to execute.
#define S3_UTIL_Sub_Query_Hotplug_Devices	237	// Query which devices can do hotplug

#define S3_UTIL_Sub_Query_Driver_Favorite_Windows_Message	238	// Query windows message array that driver favorite.
#define S3_UTIL_Sub_Send_Favorite_Windows_Message	239	// Send driver favorite windows message to driver.
#define S3_UTIL_Sub_Query_Device_Settings_Support	240	// Query driver which how many device settings is supported
#define S3_UTIL_Sub_Query_Device_Settings_Range		241	// Query device settings range.
#define S3_UTIL_Sub_Get_Current_Device_Settings		242	// Get current device settings.
#define S3_UTIL_Sub_Set_Current_Device_Settings		243	// Set current device settings.
#define S3_UTIL_Sub_Query_Video_Wall_Cells				244	// Query the video wall cell count and cell ids.
#define S3_UTIL_Sub_Get_Current_CellID						245	// Get current video wall cell id we are currently tuning on.
#define S3_UTIL_Sub_Set_New_CellID							246	// Set new video wall cell id we will tune on.
#define S3_UTIL_Sub_Set_Generic_Information				247	//Set Generic information to driver. e.g. LUID information in Win7.
#define S3_UTIL_Sub_Query_Video_Wall_Layouts			248	// Query video wall layout count and layout ids.
#define S3_UTIL_Sub_Get_Video_Wall_Resolution			249	// Query video wall layout resolution layout id after tuning
#define S3_UTIL_Sub_Get_Video_Wall_Layout				250	// Get video wall layout current in use and default layout
#define S3_UTIL_Sub_Set_New_Video_Wall_Layout		251	// Set new video wall layout to be used.
#define S3_UTIL_Sub_Get_Video_Wall_Cell_Positions		252	// Get video wall cell position for a specific layout.
#define S3_UTIL_Sub_Set_Video_Wall_Cell_Position		253	// Get video wall cell position for a specific layout.
#define S3_UTIL_Sub_New_Interface_Preferred_Escape_Map		263	// Query driver which escape will use new interface preferred.
#define S3_UTIL_Sub_Query_SpanModeCaps								264	// Query driver, which span mode is supported.
#define S3_UTIL_Sub_Get_SpanMode											265	// Query driver, which span mode is currently in use.
#define S3_UTIL_Sub_Set_SpanMode											266	// Set span mode to driver.
#define S3_UTIL_Sub_Get_Video_Wall_Connections     267
#define S3_UTIL_Sub_Query_Video_Wall_Engines						254	//	Query video wall engines
#define S3_UTIL_Sub_Get_Video_Wall_Status							255	// Get video wall engine status.
#define S3_UTIL_Sub_Set_Video_Wall_Status							256	// Set new status to video wall engine.

#define S3_UTIL_Sub_Settings_Allow_Change			293		//Query driver which setting allow change.
#define S3_UTIL_Sub_DisplayPortTestTiming		320

/*------------------------[ Defines: Related defines ]-----------------------*/

	// 23-Jun2005-Wyoung: I'm changing the names of these defines to reduce
	//   the current 3 sets (Mode, Connect, Support) down to just 1 set (Device).
	//   The original 3 sets all had the same values anyway, so this doesn't change
	//   anything but the names, and it simplifies things.

	// NOTE: If any new devices are added, make sure to update the
	//   S3_UTIL_DEV_ALL_ defines below!!!
#define S3_UTIL_DEVICE_NONE		0x0000
#define S3_UTIL_DEVICE_AUTO		0x0000
#define S3_UTIL_DEVICE_CRT1		0x0001
#define S3_UTIL_DEVICE_LCD		0x0002
#define S3_UTIL_DEVICE_TV1		0x0004
#define S3_UTIL_DEVICE_DUOVIEW		0x0008
#define S3_UTIL_DEVICE_DVI1		0x0010	// 12-Oct98-Wyoung: added for M7
#define S3_UTIL_DEVICE_CRT2		0x0020	// 12-Oct98-Wyoung: added for M7
#define S3_UTIL_DEVICE_HDTV		0x0040
#define S3_UTIL_DEVICE_DVI2		0x0200	// 28-Feb2003-Wyoung
#define S3_UTIL_DEVICE_LCD2		0x0400	// 13-Mar2003-Wyoung
#define S3_UTIL_DEVICE_HDMI1		0x0800	// 06-Mar2006-ShaominLi
#define S3_UTIL_DEVICE_DVI3		0x1000	// 04-Nov2005-Wyoung: UI name is "DVI A"
#define S3_UTIL_DEVICE_DVI4		0x2000	//    UI name is "DVI B"?
#define S3_UTIL_DEVICE_TV2		0x4000	// 24-Apr2006-DukeDu: We will disable s3disply
						//   when dual devices found
#define S3_UTIL_DEVICE_HDMI2		0x8000	// 30-Apr2006-DukeDu
#define S3_UTIL_DEVICE_HDMI3		0x10000	// 30-Apr2006-DukeDu
#define S3_UTIL_DEVICE_HDMI4		0x20000	// 30-Apr2006-DukeDu
	// 06-Mar2008-Wyoung: Add defines we'll need for DisplayPort 1 to 4
#define S3_UTIL_DEVICE_DP1		0x100000
#define S3_UTIL_DEVICE_DP2		0x200000
#define S3_UTIL_DEVICE_DP3		0x400000
#define S3_UTIL_DEVICE_DP4		0x800000

#define S3_UTIL_DEVICE_TV		S3_UTIL_DEVICE_TV1	// Alias so older utils can still
								//   use DEVICE_TV

#define S3_UTIL_DEVICE_DISCONNECTED	0x10000	// 27-Mar2001-Wyoung: Needed for combo droplist

#define S3_UTIL_DEVICE_LCD_CRT	( S3_UTIL_DEVICE_LCD | S3_UTIL_DEVICE_CRT1 )
#define S3_UTIL_DEVICE_LCD_TV	( S3_UTIL_DEVICE_LCD | S3_UTIL_DEVICE_TV )

	// Bitmasks for ALL of a type of device (all CRTs, all LCDs, etc.)
#define S3_UTIL_DEV_ALL_CRTS	( S3_UTIL_DEVICE_CRT1 | S3_UTIL_DEVICE_CRT2 )
#define S3_UTIL_DEV_ALL_LCDS	( S3_UTIL_DEVICE_LCD | S3_UTIL_DEVICE_LCD2 )
#define S3_UTIL_DEV_ALL_TVS	( S3_UTIL_DEVICE_TV1 | S3_UTIL_DEVICE_TV2 )
#define S3_UTIL_DEV_ALL_DVIS	( S3_UTIL_DEVICE_DVI1 | S3_UTIL_DEVICE_DVI2	\
				| S3_UTIL_DEVICE_DVI3 | S3_UTIL_DEVICE_DVI4 )
#define S3_UTIL_DEV_ALL_HDMIS	( S3_UTIL_DEVICE_HDMI1 | S3_UTIL_DEVICE_HDMI2	\
				| S3_UTIL_DEVICE_HDMI3 | S3_UTIL_DEVICE_HDMI4 )
#define S3_UTIL_DEV_ALL_DPS	( S3_UTIL_DEVICE_DP1 | S3_UTIL_DEVICE_DP2	\
				| S3_UTIL_DEVICE_DP3 | S3_UTIL_DEVICE_DP4 )
#define S3_UTIL_DEV_ALL_CEAS	( S3_UTIL_DEV_ALL_DVIS | S3_UTIL_DEV_ALL_HDMIS | S3_UTIL_DEV_ALL_DPS )

	// Bitmask for all devices, without Duoview bit
#define S3_UTIL_DEVICE_ALL	( S3_UTIL_DEV_ALL_CRTS | S3_UTIL_DEV_ALL_LCDS		\
				| S3_UTIL_DEV_ALL_TVS	 | S3_UTIL_DEVICE_HDTV	\
				| S3_UTIL_DEV_ALL_DVIS			\
				| S3_UTIL_DEV_ALL_HDMIS		\
				| S3_UTIL_DEV_ALL_DPS)

#define S3_UTIL_DEVICE_ALL_MONITORS		0xFFFFFFFF

#define MAX_NUM_DEVS		8

//************************************************
// Defines for real display type
#define S3_DEVTYPE_UNKNOWN		0x00000000	// Unknown type of output
#define S3_DEVTYPE_TV					0x00000001	// TV connected via other type of output
#define S3_DEVTYPE_VGA					0x00000002	// D-Sub(CRT) connected via other type of output
#define S3_DEVTYPE_DVI					0x00000004	// DVI connected via other type of output
#define S3_DEVTYPE_HDMI				0x00000008	// HDMI connected via other type of output
#define S3_DEVTYPE_DP					0x00000010	// DisplayPort connected via other type of output
#define S3_DEVTYPE_HDTV				0x00000020	// HDTV connected via other type of output
#define S3_DEVTYPE_PANEL				0x00000040	// panel (LCD) connected via other type of output
#define S3_DEVTYPE_LEGACY			0xffffff00 // Defined by device bit, refer to escape(1)

	//************************************************
	// Defines for display mode
#define S3_UTIL_VIEW_SINGLE			0x0000		// should be the same with S3DSP_SINGLE
#define S3_UTIL_VIEW_EXTENDED		0x0001		// should be the same with S3DSP_EXTENDED
#define S3_UTIL_VIEW_DUOVIEW		0x0002		// should be the same with S3DSP_DUOVIEW
#define S3_UTIL_VIEW_OTHERS			0x0004		// for example: there are 3 devices on one adapter

#define S3_UTIL_VIEW_COUNT_SAMM				2
#define S3_UTIL_VIEW_COUNT_DUOVIEW_SINGLE	1

	//************************************************
	// Defines for iga mask, see escape(110)
#define S3_UTIL_IGA1_MASK		0x0001
#define S3_UTIL_IGA2_MASK		0x0002
#define S3_UTIL_IGA3_MASK		0x0004
#define S3_UTIL_ALL_IGA_MASK	0x0007

//************************************************
// Maximum number of IGAs supported in any of our products
#define MAX_IGA_NUM	2

	//************************************************
	// Defines for 0 based iga index.
#define S3_UTIL_IGA1		0x0000
#define S3_UTIL_IGA2		0x0001
#define S3_UTIL_IGA3		0x0002
	//************************************************
	// Defines for Get Connection Status Ex
#define S3_DEV_POLL_DESTRUCTIVE			0x0000
#define S3_DEV_POLL_NONDESTRUCTIVE		0x0001

	/* Video BIOS type definition */
#define BIOS_MOBILE		0
#define BIOS_DESKTOP		1


	// New BIOS version type defines
#define BIOS_VER_LONG		0	// Long version string
#define BIOS_VER_SHORT		1	// Short version string


#define	REFRESH_TABLE_SIZE		32	// Number of DWORDs in ref table

	//************************************************
	// Defines for Get EDID Info
#define S3_DEVINFO_EDID			1
#define S3_DEVINFO_VIEWPORT_ORIGIN	2	
#define S3_DEVINFO_ASCNAME		3	// Not used, for possible future use.
#define S3_DEVINFO_CONNECTOR	4	// Return the connector used for device

/* TV flicker filter */
#define S3_TV_FFILTER_OFF		0
#define S3_TV_FFILTER_ON		1

	// CR54884.  Special value means level can never be adjusted
#define S3_TV_FFILTER_NORANGE		0xFFFFFFFF


/* TV flicker filter Interpolative Threshold */
#define S3_TV_SIT_OFF		0
#define S3_TV_SIT_ON		1

/* ChromoVision states */
#define S3_CRMVIS_OFF		0
#define S3_CRMVIS_ON		1

	
/* ChromeView (non-linear scaling) defines */
#define S3_CVIEW_NORMAL		0x0000	// Display 4:3 content on 16:9 screen
#define S3_CVIEW_ZOOM		0x0001	// Display letterboxed 4:3 on 16:9 screen
#define S3_CVIEW_STRETCH	0x0002	// Stretch 4:3 content to fill 16:9 screen
#define S3_CVIEW_NONLINEAR	0x0004	// Non-linear stretch 4:3 content to fill 16:9
	


	//************************************************
	// Defines for Get/Set TV STANDARD TYPES.
#define S3_TV_STD_UNKNOWN		0
#define S3_TV_STD_NTSC			1
#define S3_TV_STD_PAL			2
#define S3_TV_STD_NTSC_JPN		3
#define S3_TV_STD_PAL_M			4
#define S3_TV_STD_PAL_B			5
#define S3_TV_STD_PAL_D			6
#define S3_TV_STD_PAL_G			7
#define S3_TV_STD_PAL_H			8
#define S3_TV_STD_PAL_I			9
#define S3_TV_STD_PAL_K			10
#define S3_TV_STD_PAL_K1		11
#define S3_TV_STD_PAL_N			12
#define S3_TV_STD_PAL_NC		13
#define S3_TV_STD_PAL_60		14
#define S3_TV_STD_SECAM			15

	// HDTV stds must fall within hibyte range.
#define S3_TV_STD_HD_1080P		0x100
#define S3_TV_STD_HD_1080I		0x200
#define S3_TV_STD_HD_720P		0x300
#define S3_TV_STD_HD_480P		0x400
#define S3_TV_STD_HD_480I		0x500
	// 24-May2006-Wyoung: Add European HDTV standards (50Hz)
#define S3_TV_STD_HD_1080P_50HZ		0x600
#define S3_TV_STD_HD_1080I_50HZ		0x700
#define S3_TV_STD_HD_720P_50HZ		0x800
#define S3_TV_STD_HD_576P		0x900
#define S3_TV_STD_HD_576I		0xA00

#define S3_SDTV_STD_MASK		0x00FF
#define S3_HDTV_STD_MASK		0xFF00


	//************************************************
	// Defines for SUPPORTED TV STANDARDS.
#define S3_TV_SUPPORTS_NTSC		0x0000001
#define S3_TV_SUPPORTS_PAL		0x0000002
#define S3_TV_SUPPORTS_NTSC_JPN		0x0000004
#define S3_TV_SUPPORTS_PAL_M		0x0000008
#define S3_TV_SUPPORTS_PAL_B		0x0000010
#define S3_TV_SUPPORTS_PAL_D		0x0000020
#define S3_TV_SUPPORTS_PAL_G		0x0000040
#define S3_TV_SUPPORTS_PAL_H		0x0000080
#define S3_TV_SUPPORTS_PAL_I		0x0000100
#define S3_TV_SUPPORTS_PAL_K		0x0000200
#define S3_TV_SUPPORTS_PAL_K1		0x0000400
#define S3_TV_SUPPORTS_PAL_N		0x0000800
#define S3_TV_SUPPORTS_PAL_NC		0x0001000
#define S3_TV_SUPPORTS_PAL_60		0x0002000

#define S3_TV_SUPPORTS_480P		0x0004000
#define S3_TV_SUPPORTS_720P		0x0008000
#define S3_TV_SUPPORTS_1080I		0x0010000
#define S3_TV_SUPPORTS_1080P		0x0020000
#define S3_TV_SUPPORTS_480I		0x0040000
	// 24-May2006-Wyoung: Add European HDTV standards (50Hz)
#define S3_TV_SUPPORTS_576P		0x0080000
#define S3_TV_SUPPORTS_720P_50HZ	0x0100000
#define S3_TV_SUPPORTS_1080I_50HZ	0x0200000
#define S3_TV_SUPPORTS_1080P_50HZ	0x0400000
#define S3_TV_SUPPORTS_576I		0x0800000
#define S3_TV_SUPPORTS_SECAM		0x1000000

#define S3_SDTV_SUPPORT_MASK	0x1003FFF
#define S3_HDTV_SUPPORT_MASK	(S3_TV_SUPPORTS_480P | S3_TV_SUPPORTS_720P | S3_TV_SUPPORTS_1080I \
				| S3_TV_SUPPORTS_1080P | S3_TV_SUPPORTS_480I | S3_TV_SUPPORTS_576P \
				| S3_TV_SUPPORTS_720P_50HZ | S3_TV_SUPPORTS_1080I_50HZ \
				| S3_TV_SUPPORTS_1080P_50HZ | S3_TV_SUPPORTS_576I)


	//************************************************
	// Defines for TV OUTPUT PORTS.
#define S3_TVPORT_COMPOSITE		0
#define S3_TVPORT_SVIDEO		1
#define S3_TVPORT_COMPONENT		2
#define S3_TVPORT_COMPOSITE_SVIDEO	3
#define S3_TVPORT_UNKNOWN1		4	// BIOS value currently not mapped to specific config
#define S3_TVPORT_UNKNOWN2		5	// BIOS value currently not mapped to specific config
#define S3_TVPORT_ALL			6	// All ports activate at same time
#define S3_TVPORT_COMPOSITE_SVIDEO_COMPONENT       7 

#define S3_TVPORT_HAS_UNKNOWN		0x0000
#define S3_TVPORT_HAS_COMPOSITE		0x0001
#define S3_TVPORT_HAS_SVIDEO		0x0002
#define S3_TVPORT_HAS_SCART		0x0004
#define S3_TVPORT_HAS_COMPONENT		0x0008
#define S3_TVPORT_HAS_SDTV_RGB		0x0010
#define S3_TVPORT_HAS_SDTV_COMPONENT	0x0020
#define S3_TVPORT_HAS_SVIDEO2		0x0040
// 02-Jun2004-RainLin: Add HDTV Ports
#define S3_TVPORT_HAS_HDTV_RGB		0x0400
#define S3_TVPORT_HAS_HDTV_COMPONENT	0x0800
#define S3_TVPORT_HAS_UNKNOWN1		0x1000
#define S3_TVPORT_HAS_UNKNOWN2		0x2000
#define S3_TVPORT_HAS_HDTV		0x4000

	// TV Support Capability bits
#define S3_TVCAPS_FFILTER	0x00001		// TV supports has flicker filter
#define S3_TVCAPS_SHARPNESS	0x00002		// TV supports has sharpness
#define S3_TVCAPS_BRIGHTNESS	0x00004		// TV supports has brightness
#define S3_TVCAPS_CONTRAST	0x00008		// TV supports has contrast
#define S3_TVCAPS_COLOR		0x00010		// TV supports has color/saturation
#define S3_TVCAPS_TINT		0x00020		// TV supports has tint/hue
#define S3_TVCAPS_SIZE		0x00040		// TV supports has horz/vert sizing
#define S3_TVCAPS_ASPECTLOCK	0x00080		// TV supports has aspect ratio lock
#define S3_TVCAPS_POSITION	0x00100		// TV supports has positioning
#define S3_TVCAPS_TEXTENH	0x00200		// TV supports has text enhancement (intensity)
#define S3_TVCAPS_ADPFFILTER	0x00400		// TV supports has adaptive flicker filter
#define S3_TVCAPS_DOTCRAWL	0x00800		// TV supports has dot crawl control
#define S3_TVCAPS_LUMAFILTER	0x01000		// TV supports has luma filter
#define S3_TVCAPS_CHROMAFILTER	0x02000		// TV supports has chroma filter
#define S3_TVCAPS_HDTV_SIZE	0x04000		// HDTV supports software scaling/contraction
#define S3_TVCAPS_HDTV_POS	0x08000		// Driver supports HDTV positioning,
						//   and TV pos escapes need device type.
#define S3_TVCAPS_CHROMEVIEW	0x10000		// ChromeView (non-linear scaling on HDTV)
#define S3_TVCAPS_CUSTOM_MODES	0x020000	// HDTV/HDMI supports customize mode
#define S3_TVCAPS_HDTV_AUTORESIZE 0x40000	// Don't show resize checkbox. After change HDTV type, 
						//  must resize the descktop.

#define S3_TVCAPS_SATURATION	S3_TVCAPS_COLOR
#define S3_TVCAPS_HUE		S3_TVCAPS_TINT

#define S3_TVCAPS_ALWAYSAPPLY	0x80000000	// Metacaps bit - Always apply TV settings

	// 16-Feb2001-Wyoung: Add some extra flags that aren't really
	//   part of driver.  We need these for handling extra items
	//   in tray menu ( in dwTrayTVExtras flag).
#define S3_TVCAPS_CENTER	0x20000		// Really means positioning, but
						//   need extra flag for tray menu
#define S3_TVCAPS_DEFAULT	0x40000
#define S3_TVCAPS_CHROMOVIS	0x80000		// ChromoVision

#define S3_TVCAPS_ALL		0xFFFFFF


#define S3_CONTRACTION_FACTOR 1000	// Contraction step multiplier
					//   for HDMI/HDTV customized mode handling.
// JanusHsieh [2004/08/17]: Get expected CPU power state.
#define S3_CPU_POWER_L2_ENABLED 0x0001
#define S3_CPU_POWER_L3_ENABLED 0x0002
// RainLin [2005/07/13]: Add events to notify driver.
#define S3_UTIL_NOTIFYEVENT_APM_SUSPEND   0x0001
#define S3_UTIL_NOTIFYEVENT_SHUT_DOWN     0x0002


	// 07-Feb2007-DukeDu: Refresh Rate Override state definitions
#define S3_RRATE_NO_OVERRIDE		0x000000
#define S3_RRATE_SAME_AS_DESKTOP	0x000001
#define S3_RRATE_HIGHEST_CAPABLE	0x000002
#define S3_RRATE_OVERRIDE_VALUE		0x000004
#define S3_RRATE_ONLY_IF_MINIMUM	0x008000
#define S3_RRATE_BAD_MONITOR		0x010000


	//******* Bit mask definitions for Driver Events *********//
#define DE_DVD_PLAYING			0x0001
#define DE_SP1_IN_USE			0x0002
#define DE_SP2_IN_USE			0x0004
#define DE_LOW_POWER			0x0010
#define DE_CP_NEEDS_MODESET		0x0020		// Copy protected content playing.
#define DE_HK_ROTATION			0x0100		// Retate hotkey event
#define DE_HK_MASK				0x0f00		// Hotkey Event Mask
#define DE_IN_DOSFULL			0x10000		//In dos full screen mode.
#define DE_D3D_ON				0x20000		//D3D is running.
#define DE_OGL_ON				0x40000		//OGL is running.
#define DE_IN_SAMM				0x80000		//In SAMM mode.
#define DE_MULTIG_ON			0x100000	//MultiG enabled
#define DE_HK_SWITCHDEVICE		0x200000	//ACPI switch device hotkey event
#define DE_HK_DISABLE_SHADOW	0x400000	//Disable shadow menu in Vista
#define DE_HK_SWITCHMODE		0x800000	//Driver need set a mode, for example driver found EDID become bad.

	//******* Bit mask definitions for Event Intents *********//
#define	EI_ROTATE_TYPE_MASK		1		// Rotate type, relative or absolute
#define	EI_ROTATE_RELATIVE		0		// Relative rotate type
#define	EI_ROTATE_ABSOLUTE		1		// Absolute rotate type

#define	EI_ROTATE_IGA_MASK		0xff		// Target iga (currently have 2 igas)
#define	EI_ROTATE_IGA1			1		// Target is iga1
#define	EI_ROTATE_IGA2			2		// Target is iga2
#define	EI_ROTATE_IGA12			3		// Target is iga1 and iga2

#define	EI_ROTATE_DIRECTION_MASK	0xf		// Relative rotate direction
#define	EI_ROTATE_CLOCKWISE		0		// Rotate clockwise when hotkey pressed. (Default)
#define	EI_ROTATE_COUNTERCLOCKWISE	1		// Rotate counterclockwise when hotkey pressed.

#define	EI_ROTATE_STEP_MASK		0xf		// Relative rotate step mask
#define	EI_STEP_90			0		// Rotate step is 90 degree. (Default)
#define	EI_STEP_180			1		// Rotate step is 180 degree
#define	EI_STEP_270			2		// Rotate step in 270 degree

#define	EI_ROTATE_ABSOLUTE_MASK		0x1f		// Absolute style
#define	EI_ROTATE_ZERO			0		// Rotate to ZERO degree when hotkey pressed.
#define	EI_ROTATE_HFLIP			0x01		// Horizontal flip when hotkey pressed.
#define	EI_ROTATE_VFLIP			0x02		// Vertical flip when hotkey pressed.
#define	EI_ROTATE_90			0x10		// Rotate to 90 degree when hotkey pressed.
#define	EI_ROTATE_180			0x03		// Rotate to 180 degree when hotkey pressed.
#define	EI_ROTATE_270			0x13		// Rotate to 270 degree when hotkey pressed.

	//************************************************
	// Defines for Get/Set CLOCK calls.
#define S3_CLOCK_MCLK			1
#define S3_CLOCK_ECLK			2
#define S3_CLOCK_ICLK			4

	//******* Bit mask definitions for hotkey behavior *********//
#define HK_FROM_HOTKEY			0x0001	// indicates that it is from hotkey.
#define HK_QUIET_MODE			0x0100	// Don't display any message box.


	//***** Bitmask definitions for display/IGA orientation ****//
#define S3_DO_NORMAL		0x0000
#define S3_DO_VNRM_HREV		0x0001
#define S3_DO_VREV_HNRM		0x0002
#define S3_DO_VREV_HREV		0x0003
#define S3_DO_ROTATE180		S3_DO_VREV_HREV
#define S3_DO_ROTATE90		0x0010
#define S3_DO_ROTATE270		0x0013

	// Bitmask definitions for display orientation support caps //
#define S3_OR_HORZFLIP		0x0001
#define S3_OR_VERTFLIP		0x0002
#define S3_OR_ROTATE90		0x0010
#define S3_OR_ROTATE180		0x0020
#define S3_OR_ROTATE270		0x0040
#define S3_OR_XPSP1SUPPORT	0x8000

	// Bitmask definitions for Set Driver Event Handle
#define S3_EVNT_OP_ADD			0x00000001	// Add an event handle
#define S3_EVNT_OP_DELETE		0x00000002	// Remove an event handle
#define S3_SCREENSAVER_ON		0x00000004
#define S3_SCREENSAVER_OFF		0x00000008

#define S3_EVNT_TYPE_CONNECT		0x00000001	// Hot Plug Connect
#define S3_EVNT_TYPE_DISCONNECT		0x00000002	// Hot Disconnect
#define S3_EVNT_TYPE_POWERWISE		0x00000004	// Powerwise setting change
#define S3_EVNT_TYPE_REFRESHMODELIST	0x00000008	// Refresh mode list
#define S3_EVNT_TYPE_COMMON_SCRIPT		0x00000010	// Common script event
#define	S3_EVNT_TYPE_SETTING_CHANGED	0x00000020	// Setting is Changed e.x. Overlay page

// Historic device state flags
#define HSF_NONE						0x00000000	// None of the state is available
#define HSF_DISPLAYMODE					0x00000001	// Display Mode is available
#define HSF_PRIMARYDEV					0x00000002	// Primary device is available

// Historic device mode
#define DPM_SINGLE						0x00000001	// Single view(Single device)
#define DPM_DUOVIEW						0x00000002	// Duoview(Clone view)
#define DPM_SAMM						0x00000003	// SAMM(Extended view)

//************************************************
// Defines for Thermal Sensor calls.
#define  S3_THERMAL_NONE	0x0000		// No thermal sensor support
#define  S3_THERMAL_LOW		0x0001		// Low temperature range can be queried/set
#define  S3_THERMAL_NORMAL	0x0002		// Normal temperature range can be queried/set
#define  S3_THERMAL_WARNING	0x0004		// Warning temperature range can be queried/set
#define  S3_THERMAL_OVERHEAT	0x0008		// Overheat) temperature range can be queried/set
#define  S3_THERMAL_FANRPM	0x0010		// Fan speed can be reported and set in RPM
#define  S3_THERMAL_FANPERCENT	0x0020		// Fan speed can be reported and set in percentages
#define  S3_THERMAL_CELSIUS	0x0040		// Temperatures are Celsius;  otherwise, Fahrenheit.
#define  S3_THERMAL_HISTORY	0x0080		// Driver can retain a short history of past temperatures
#define  S3_THERMAL_STATUS	0x0100		// Get/Set the current threshold status (use or don’t use)
#define  S3_THERMAL_AUTO	0x0200		// Driver handles everything automatically
#define  S3_THERMAL_APPROXTEMP	0x0800		// Cannot report current temp, only approximate range
//  (low, normal etc.)
#define  S3_THERMAL_AUTO_ONLY	0x10000		// There is only an auto setting, don’t show any UI.
#define  S3_THERMAL_ECLK	0x20000		// Combined with other bits to query Eclk settings.
#define  S3_THERMAL_MCLK	0x40000		// Combined with other bits to query Mclk settings.
#define  S3_THERMAL_HISTOSIZE	0x80000		// For querying histogram buffer size.
#define  S3_THERMAL_HISTOTIME	0x100000	// For querying histogram polling interval.
#define  S3_THERMAL_PEAKTEMP	0x200000	// For peak observed temperature.

#define DHS_DEVICESWITCH				0x00000000	// History for device switch and hotplug
#define DHS_HOTKEY						0x00000001	// History for hotkey


#define LS_SaveDisplayModeHistory       0x00000001  //Lock-Unlock : save display mode history
#define LS_SaveEnabledDeviceHistory     0x00000002  //Lock-Unlock : save display device combination history
#define S3_State_Lock                   1           //Lock
#define S3_State_Unlock                 0           //Unlock

#define DPU_DEVICESWITCH				1			// used for switch device, default.
#define DPU_SWAPPRIMARY					2			// used for swap primary device.

//***** Bitmask definitions for escape(202) ****//
#define 		S3_UTIL_HOTPLUG_FLAGS_FORCE_DEVICE_CHANGE		0x0001


//***** Bitmask definitions for escape(215) ****//
#define S3DM_SINGLE						0x00000001	// Single view(Single device)
#define S3DM_DUOVIEW					0x00000002	// Duoview(Clone view, or simultaneous mode)
#define S3DM_SAMM						0x00000003	// SAMM(Extended view)

#define S3DEVICE_CHANGE_INVOKE_CHROME3	0x00000000
#define S3DEVICE_CHANGE_INVOKE_HOTKEY	0x00000001

//***** Action flag for escape(236) ****//
#define S3FLEXIBLETIMER_IDLE							0x00000000
#define S3FLEXIBLETIMER_EXECUTE_SCRIPT		0x00000001
#define S3FLEXIBLETIMER_END							0x00000002

//***** Status mask for escape(236) ****//
#define	S3SS_MASK_ACLINE				0x00000001
#define	S3SS_MASK_PROCESSRUNNING		0x00000002

//***** Status bit for escape(236) ****//
#define	S3SS_ACLINE						0x00000001
#define	S3SS_PROCESSRUNNING				0x00000002

//************************************************
// Defines for CCD_EZ_QueryDisplayConfig extra flags
#define CCDEZ_OPT_GDINAMES	1		// Return GDI device names (e.g. "\\.\DISPLAY1")
#define CCDEZ_OPT_EDIDNAMES	2		// Return EDID names (e.g. "JC199D")


//************************************************
// Defines for common script command
#define S3SCRIPT_SIMPLE_ESCAPE						0x00000001
#define S3SCRIPT_SIMPLE_SETMODE						0x00000002
#define S3SCRIPT_REFRESH_MODE_LIST					0x00000003
#define S3SCRIPT_SYSTEMPARAMETERSINFO				0x00000004
#define S3SCRIPT_CREATE_FLEXIBLE_TIMER				0x00000005
#define S3SCRIPT_PATCH_FORCELOWMODEFORVIDEO			0x00010000
#define S3SCRIPT_PATCH_RESTOREHIGHMODEFORVIDEO		0x00010001
#define S3SCRIPT_PATCH_SETSECONDARYVIEWPOSITION		0x00010010


//************************************************
// Defines for common script excute result
#define S3SCRIPT_PASS		0x00000000
#define S3SCRIPT_FAIL		0x00000001
#define S3SCRIPT_BUSY		0x00000002


//Define const for escape(293)
#define S3ST_VIDEOCOLOR		100		//Driver allows change video color.
#define S3ST_CHROMOTION		101		//Driver allows change chromotion.
#define S3ST_CHANGERESOLUTION		102	//Driver allows change resolution.
/*--------------------------[ Structure Declarations ]-------------------------*/

	//************************************************
	// Structure for querying LCD panel size.
typedef struct _s3PanelInfo
{
	WORD	PanelXRes;	// Panel X resolution
	WORD	PanelYRes;	// Panel Y resolution
	WORD	PanelType;	// Panel type (S3_PANEL_TYPE_** defines)
} S3PANELINFO, FAR* LPS3PANELINFO;


	//************************************************
	// Structure for querying DFP (flat panel) info.
	// 12-Jan2004-Wyoung: No longer used.
typedef struct _s3DfpInfo
{
	WORD	PanelXRes;	// Panel X resolution
	WORD	PanelYRes;	// Panel Y resolution
	WORD	PanelType;	// Panel type (S3_PANEL_TYPE_** defines)
	WORD	wFreqencyType;	// Single freq or multi-sync?
	WORD	wConnectState;	// First time attached?
} S3DFPINFO, FAR* LPS3DFPINFO;


	//************************************************
	// Extended Structure for getting panel
	//   (DVI -normal or CRT- or LCD) info.
typedef struct _s3PanelInfov2
{
	WORD	PanelXRes;	// Panel X resolution
	WORD	PanelYRes;	// Panel Y resolution
	WORD	PanelType;	// Panel type (S3_PANEL_TYPE_** defines)
	WORD	wDVIcaps;	// Capabilities bitmask
	WORD	wFrequencyType;	// Single freq or multi-sync?
	WORD	wCurrFrequency;	// Current refresh frequency
	WORD	wConnectState;	// First time attached?
	WORD	wReserved[9];	// Reserved for future expansion
} S3PANELINFOV2, FAR* LPS3PANELINFOV2;




	//************************************************
	// Structures for querying HDTV sub-modes.
typedef struct _tagHdtvSubMode
{
	WORD	wHeight;
	WORD	wWidth;
} HDTVSUBMODE, FAR* LPHDTVSUBMODE;

#define MAX_HDTV_SUBMODES	16


	//************************************************
	// Structure for querying BIOS version info.
typedef struct tagSTBIOSVersionInfo
{
	WORD	wBIOS_AA;
	WORD	wBIOS_BB;
	WORD	wBIOS_CC;
	WORD	wBIOS_DD;
	WORD	wBIOSType;
	WORD	wReserved;
} STBIOSVersionInfo, FAR* LPSTBIOSVersionInfo;

typedef struct tagSTBIOSVersionInfoEx
{
	WORD	wBIOS_AA;		// Core BIOS version
	WORD	wBIOS_BB;		// Revision number of controller   
	WORD	wBIOS_CC;		// Major BIOS Production version 
	WORD	wBIOS_DD;		// Minor BIOS Production version 
	WORD	wBIOSType;		// (Mobile = 0/Desktop = 1)
	WORD	wBIOS_L;		// OEM customer letter
	WORD	wBIOS_EE;		// OEM BIOS major version 
	WORD	wBIOS_FF;		// OEM BIOS minor version 
	WORD	wVerFlags;		// New bit flags, where bits 0-2:
					//   0x01 - 3-part version
					//   0x02 - 4-part version
					//   0x03 - 5-part version
					//   0x04 - 6-part version
} STBIOSVersionInfoEx, FAR* LPSTBIOSVersionInfoEx;


// extra string for the version. Total 10 BYTEs.
typedef struct tagExtraBIOSVersion
{
	WORD	fAvailable:1;		// Must set to 1 to indicate it available.
	// 0 means not available, 1 means available.
	WORD	fPrefix:1;			// BIOSVER_PREFIX or BIOSVER_APPENDIX
	// 1 means prefix, 0 means postfix
	WORD	nIndex:4;			// Index of extra version
	WORD	nReserve:10;		// Reserve for future use.
	BYTE	szExtraVersion[8];	// Extra version string. Multibyte characters.
	// Not wide characters.
} EXTRABIOSVERSION, FAR *LPEXTRABIOSVERSION;

typedef struct tagSTBIOSVersionInfoEx2
{
	WORD	wBIOS_AA;		// Core BIOS version
	WORD	wBIOS_BB;		// Revision number of controller   
	WORD	wBIOS_CC;		// Major BIOS Production version 
	WORD	wBIOS_DD;		// Minor BIOS Production version 
	WORD	wBIOSType;		// (Mobile = 0/Desktop = 1)
	WORD	wBIOS_L;		// OEM customer letter
	WORD	wBIOS_EE;		// OEM BIOS major version 
	WORD	wBIOS_FF;		// OEM BIOS minor version 
	WORD	wVerFlags;		// New bit flags, where bits 0-2:
	//   0x01 - 3-part version
	//   0x02 - 4-part version
	//   0x03 - 5-part version
	//   0x04 - 6-part version
	//   0x05 - 7-part version
	EXTRABIOSVERSION ExtraBIOSVersion[2];	// Extra version info.
} STBIOSVersionInfoEx2, FAR* LPSTBIOSVersionInfoEx2;


// BIOS date info structure
typedef struct tagSTBIOSDateInfo
{
	WORD	wBIOS_Month;	// Month
	WORD	wBIOS_Day;	// Day
	WORD	wBIOS_Year;	// Year
	WORD	wReserved;	// Reserved for future use
} STBIOSDateInfo, FAR* LPSTBIOSDateInfo;


// Get driver file names struct
typedef struct _S3DRVNAMES
{
	BYTE
		szDisplayDriver[32],
		szDDrawDriver[32],
		szMiniport[32],
		szOpenGLDriver[32],
		szMetalDriver[32],		// Now unused
		szMCIDriver[32],
		szBIOSDriver[32];
} S3DRVNAMES, FAR* LPS3DRVNAMES;

	// Get driver file names struct, version 2.
	//   Struct is expanded to allow up to 64 WCHARs
	//   because driver names in Vista are longer.
	// 08-Nov2006-Wyoung: Changed display and miniport driver field
	//   names so it's more clear they are also UM or KM drivers.
typedef struct _S3DRVNAMES2
{
	BYTE
		szDisplayUMDriver[128],		// Display driver (User-mode in Vista)
		szDDrawDriver[128],
		szMiniportKM[128],		// Miniport driver (kernel-mode in Vista)
		szOpenGLDriver[128],
		szDX10Driver[128],		// DX10 driver name (only in Vista)
		szMCIDriver[128],
		szBIOSDriver[128];
} S3DRVNAMES2, FAR* LPS3DRVNAMES2;



// For LargeDesk query call
typedef struct _largedeskinfo
{
	DWORD	dwFlags;		// Bit0=1 if LargeDesk ON, 0 if off,
					// Bit1=1 if Multi-mon enabled
	DWORD	dwLayout;		// Bits0-2=0 if wide, 1 if tall
					// Bits3-5=0 if LCD on left/top, 1 if right/bottom
	DWORD	dwPrimaryRes;		// loword=vertical res, hiword=horz res (works on Win9x)
	DWORD	dwReserved1;
	DWORD	dwSecondRes;		// (works on Win9x)
	DWORD	dwReserved2;
} LDESKINFO, FAR* LPLDESKINFO;


	// For S3_UTIL_GetDeviceHistoricState().
typedef struct tagHistoricState
{
	DWORD	dwStateFlags;	// Historic state flag (HSF_***), indicate which state is available.
	DWORD	dwDisplayMode;	// Display Mode (DPM_***), Single, DuoView or SAMM.
	DWORD	dwPrimaryDevice;// Primary device(S3_UTIL_DEV_* define)
	DWORD	dwReserved[62];	// Reserved for future use.
} HISTORIC_STATE, *LPHISTORIC_STATE;


	// For TV Aperture Control calls
typedef struct _aperturectrl
{
	DWORD	dwCurrent;			// -1 means error, not supported.
	DWORD	dwLowThreshold;
	DWORD	dwMidThreshold;		// 70h
	DWORD	dwHighThreshold;
	DWORD	dwInverseFlag;
} APCTRL, FAR* LPAPCTRL;



typedef struct tagS3HardwareInfo
{
	WORD	wChipID;		// Chip ID Number (e.g.,  8A20)
	WORD	wReserved0;		// Must be 0.
	WORD	wSubsysVendor;		// Subsystem Vendor ID
	WORD	wSubsysDevice;		// Subsystem Device ID
	WORD	wBusDevFuncNum;		// Bus number and device/function number
					//   devnum is bits 7-3, funcnum is bits 2-0
					//   busnum is bits 15-8
	WORD	wRevision;		// Revision number
	WORD	wReserved[2];		// Reserved for future use
} S3HARDWAREINFO, FAR* LPS3HARDWAREINFO;


typedef struct tagS3HardwareInfo2
{
	WORD	wChipID;		// Chip ID Number (e.g.,  8A20)
	WORD	wReserved0;		// Must be 0.
	WORD	wSubsysVendor;		// Subsystem Vendor ID
	WORD	wSubsysDevice;		// Subsystem Device ID
	WORD	wBusDevFuncNum;		// Bus number and device/function number
					//   devnum is bits 7-3, funcnum is bits 2-0
					//   busnum is bits 15-8
	WORD	wRevision;		// Revision number
	WORD	wFeatures;		// Bus type and version
	WORD	wReserved;		// Reserved for future use
	DWORD	dwHwSupports;		// Supported features (AGP Status bits,
					//   or PCI Express link caps)
	DWORD	dwHwEnabled;		// Enabled features (AGP Command bits,
					//   or PCI Express link status)
	DWORD	dwReserved;		// Reserved
} S3HARDWAREINFO2, NEAR* PS3HARDWAREINFO2, FAR* LPS3HARDWAREINFO2;


typedef struct tagDeviceCombination
{
	DWORD		dwDevBits;           // Target device bit (S3_UTIL_DEV_*),
	DWORD		dwPrimaryDevBit;     // Adapter that the device is on, (indicate which hardware)
	DWORD		dwReserve[4];
} DEVICE_COMBINATION, *LPDEVICE_COMBINATION;



#ifndef LONIBBLE
#define LONIBBLE(w)           ((BYTE)(w & 0x0F))
#endif
#ifndef HIBIBBLE
#define HINIBBLE(w)           ((BYTE)(((w) >> 4) & 0x0F))
#endif

	// Bus type defines, after shifting from bits 10_8
#define BUSTYPE_PCI	0
#define BUSTYPE_AGP	1
#define BUSTYPE_PCI_E	2	// PCI Express


typedef struct tagModeInfo
{
	WORD		XRes;		// X resolution
	WORD		YRes;		// Y resolution
	WORD		Bpp;		// Bits per pixel
	WORD		RefRate;	// Refresh rate
} MODE_INFO, FAR* LPMODE_INFO;


	// Defines for HDMI format and signal use
#define S3_HDMI_FORMAT_CURRENT		1
#define S3_HDMI_FORMAT_DEFAULT		0
#define S3_HDMI_FORMAT_OPTIMIZED	((DWORD)-1)
#define S3_HDMI_FORMAT_LAST_GOOD	((DWORD)-2)

#define S3_HDMI_SIGNAL_CURRENT		1
#define S3_HDMI_SIGNAL_DEFAULT		0
#define S3_HDMI_SIGNAL_OPTIMIZED	((DWORD)-1)
#define S3_HDMI_SIGNAL_LAST_GOOD	((DWORD)-2)

#define S3_HDMI_PC_MODE			1		// 05-May2008-DukeDu: for PHDMIFormat->dwFlags

	// dwFlags of structure HDMIFormat
#define HDMI_FLAG_PCMODE		0x00000001

#define HDMI_SIGNAL_RGB			0x0001
#define HDMI_SIGNAL_YCBCR_422		0x0002
#define HDMI_SIGNAL_YCBCR_444		0x0004

#define HDMI_SIGNAL_TEXT_RGB		TEXT("RGB")
#define HDMI_SIGNAL_TEXT_YCBCR_422	TEXT("YCbCr 4:2:2")
#define HDMI_SIGNAL_TEXT_YCBCR_444	TEXT("YCbCr 4:4:4")

	// Device caps definition
#define S3_DEV_CAPS_HDMI_AUDIO		0x00000001	// Query HDMI audio caps supported
#define S3_DEV_CAPS_CEA861B		0x00000002	// Device supports CEAB61B features.
							//  Also used to identify CEA861B
							//  and normal devices.
#define S3_DEV_CAPS_3DSHRINK		0x0008		// 3D shrink for customize mode.
#define S3_DEV_CAPS_LCDMONITOR		0x0010		// Set LCD monitor caps
#define S3_DEV_CAPS_ALLOW_PCMODE	0x0020		// If allow pc mode toggle.
#define S3_DEV_CAPS_SUPPORT1080P	0x0080		// If allow the end user decide if support 1080P
#define S3_DEV_CAPS_LOCKASPECTRATIO	0x0100		// if the AspectRatio if locked
#define S3_DEV_CAPS_EDID_PREFERED_MODE	0x0200


	// Device's states to query
#define S3_DEV_STATE_HDMI_AUDIO		0x0001		// Set HDMI audio state
#define S3_DEV_STATE_HDMI_AUTOMODE	0x0002		// Set HDMI auto mode state, Obsolete settings. No driver support now.
#define S3_DEV_STATE_3D_SHRINK		0x0004		// Set HDTV/HDMI 3D shrink state
#define S3_DEV_STATE_SHRINK			0x0008		// Set device shrink state
#define S3_DEV_STATE_LCDMONITOR		0x0010		// Set device to LCD monitor
#define S3_DEV_STATE_ALLOW_PCMODE	0x0020		// If allow pc mode.
#define S3_DEV_STATE_SUPPORT1080P	0x0080		// If allow the end user decide if support 1080P
#define S3_DEV_STATE_LOCKASPECTRATIO	0x0100  // set the AspectRatioLock status
#define S3_DEV_STATE_EDID_PREFERED_MODE	0x0200
#define S3_DEV_STATE_HDTV_AUTOMODE	0x0400

	// Device state value definition
#define S3_DEV_STATE_VAL_DISABLED	0		// Device state to get is disabled.
#define S3_DEV_STATE_VAL_ENABLED	1		// Device state to get is enabled.
#define S3_DEV_SHRINK_RESTORE		2		// Restore the device shrink state.
#define S3_DEV_STATE_VAL_DEFAULT	0x10	// Device state to default, only for escape(173): set state

//************************************************
//	Sync token Functions
#define S3_SYNC_INVALIDATEVIDPN			0x00000001

//************************************************
//	Sync token status
#define S3_TK_DOING		0x00000000
#define S3_TK_FINISHED	0x00000001


	// Define feature bit
typedef enum tagFeature
{
	S3F_ACCESSHW107 = 0,		//  Utility won¡¯t set HardwareAccess when call escape(107) under vista,
								//		unless driver returned S3_UTIL_NEED_ACCESSHW, For vista only.
	S3F_NEED_INVALID = 1,		// 	Utility will call InvalidateVidPn() with the same parameters if driver
								//		returned this flag, For vista only.
	S3F_ESC185V1 = 2,			// 	Indicate if driver has implemented escape(185) version 1, For vista only.
	S3F_NOESC185V0 = 3,			// 	Indicate driver hasn¡¯t implemented escape(185) version 0, For vista only.
	S3F_RecommendV2 = 4,		// 	Indicate escape(192) version 2 has been implemented.
	S3F_KeepHistoricState = 5,	// 	Indicate escape(196) has been implemented.
	S3F_GetHotPlug = 6,			//	Indicate escape(148) has been implemented.
	S3F_GetHotPlugEx = 7,		//	Indicate escape(202) has been implemented.
	S3F_GetDeviceCombinationV2 =8, // indicate escape (195) version 2 has been implemented.
	S3F_SyncToken = 9,			//	Indicate escape(203/204) has been implemented to sync with driver.
	S3F_SetDisplayControlEx = 10,	//	Indicate escape(200) has been implemented.
	S3F_RecommendV3 = 11,			//	Indicate escape(192) version 3 has been implemented.
	S3F_Event_SCREENSAVER=  12,  // Indicate escape (201) support send event to driver to detect screensaver.
	S3F_PrepareSetDisplayControl = 13, // Indicate escape(205) prepare set display control is supported.
	S3F_WinXP_Interlace = 14, // Indicate driver support winxp interlace mode, dmDisplayFrequency/=2, 
												// dmDisplayFlags &= ~DM_INTERLACED, when set mode under WinXP.
	S3F_HotPlugDeviceCombinationHistory = 15, // 08-May2008-DukeDu: Indicate escape(199) has been implemented.
	S3F_RecommendV4 = 16,			//	Indicate escape(192) version 4 has been implemented.
	S3F_50HzPALTV= 17,				//	Indicate refresh rate for PAL is 50Hz, not 60Hz.
	S3F_SetConfiguration = 18,		// Indicate escape(206) has been implemented.
	S3F_HybridChrome = 19,		// Indicate escape(208), (209) has been implemented.
	S3F_HybridChromeSwitchMethod = 20,		// Indicate escape(210), (211) has been implemented.
	S3F_CanSetPrimaryDevice = 21,		// Indicate escape(215) has been implemented.
	S3F_HybridNeedChangeEngine = 22, // Indicate escape(212), (213), (214) has been implemented.
	S3F_TurnOnOffMonitor = 23, // Indicate escape(220), (221) has been implemented.
	S3F_GetDeviceRefreshTableEx = 24,
	S3F_RotateDuoviewDevice = 25,	// Indicate escape(122)~(124) support rotate a single device under Duoview.
	S3F_CommonScript		= 26,	// If set escape(224)~(226) should be implemented.
	S3F_MultiDevicesOnOneView = 27,		// Indicate escape(230)~(231) has been implemented.
	S3F_SimultaneousConfig = 28,	// Indicate escape(233)~(236) has been implemented.
	S3F_FlexibleTimer = 29,		// Indicate escape(227) has been implemented.
	S3F_QueryRealDeviceIndex = 30,		// Can query real device index in escape(67)
	S3F_QueryHotplugDevices = 31,		// Can query driver, which device can do hot plug.
	S3F_DriverFavoriteWindowsMessage = 33,		// Support driver favorite windows message.
	S3F_DeviceSettings = 34,		// Indicate escape(240) ~ (243) are implemented.
	S3F_VideoWallCellSettings = 35,		// Indicate escape(244) ~ (246) are implemented.
	S3F_SetGenericInformation = 36,	// Indicate escape(247) is implemented.
	S3F_NewInterfacePreferredEscape = 38,	// Indicate escape(263) is implemented.
	S3F_SettingsAllowChange = 39,			// Indicate escape(293) implemented.
	S3F_Win7TwoStepsSwapPrimary = 40, // When swap primary under clone view, need switch to single (primary device) first.
	S3F_NoDeviceSwitch = 41,			// Indicate device switch page is not supported.
	S3F_SpanMode = 42,							// Indicate escape(264) ~ (266) are implemented.
	S3F_VideoWallEngineConfig = 43,			// Indicate escape(254)~(256) are implemented.
	S3F_VideoWallEngineCombination = 44,			// Indicate allow change combination.
} S3FEATURE;

// Define device main feature
typedef enum tagDevFeature
{
	S3DF_ALL = 0,					// Reserved for future use
	S3DF_Expand = 1,				// Device support center/expand feature.
} S3D_FEATURE;

// Define device sub feature
typedef enum tagDevSubFeature
{
	S3DSF_ALL = 0,					// All sub features
	S3DSF_Center = 1,				// Device support center for expand feature.
} S3D_SUB_FEATURE;

typedef struct tagS3DisplaySettings
{
	int		cbSize;		// structure size in bytes.
	LPCTSTR	lptszDeviceName;	// lpszDeviceName parameter in ChangeDisplaySettingsEx()
	LPDEVMODE	lpDevMode;		// lpDevMode parameter in ChangeDisplaySettingsEx()
	HWND		hwnd;			// hwnd parameter in ChangeDisplaySettingsEx()
	DWORD		dwFlag;			// dwflags parameter in ChangeDisplaySettingsEx()
	LPVOID		lParam;			// lParam parameter in ChangeDisplaySettingsEx()
	LONG		lRet;			// Return value for ChangeDisplaySettingsEx() or ChangeDisplaySettings()
} S3DISPLAYSETTINGS, *LPS3DISPLAYSETTINGS;


typedef struct _S3Script
{
	DWORD     dwLineIndex;           // Line index, 0 based.
	DWORD     dwCommand;             // Command
	DWORD     dwParameterSize;       // Size in DWORD of the parameters, Minimum is 1
	DWORD     dwParameters[1];        // Parameters, the array size is dwParameterSize, the structure depend on dwCommand.
} S3SCRIPT, *LPS3SCRIPT;

typedef struct tagVideoWallEngine
{
	DWORD	dwEngineId;
	DWORD	dwReserved[3];
	BYTE	bEngineName[32];
} VIDEO_WALL_ENGINE, *LPVIDEO_WALL_ENGINE;

typedef struct tagVideoWallEngineStatus
{
	DWORD	dwEngineId;
	DWORD	dwGroupIndex;	// 0 based group index. Each group id of off engine is unique.
	DWORD	dwCellCount;
	DWORD	dwEngineStatus;
	DWORD	dwReserved[4];
} VIDEO_WALL_ENGINE_STATUS, *LPVIDEO_WALL_ENGINE_STATUS;

// Video wall engine status
#define	S3_UTIL_VIDEOWALL_ENGINE_OFF		0x00000000
#define	S3_UTIL_VIDEOWALL_ENGINE_ON		0x00000001

// dwFields for structure VIDEO_WALL_CELL_POSITION.
#define		S3_UTIL_VIDEO_WALL_POSITION_CELLID			0x1	// Mask of dwCellID member
#define		S3_UTIL_VIDEO_WALL_POSITION_LEFTBORDER	0x4	// Mask of dwLeftBorder member
#define		S3_UTIL_VIDEO_WALL_POSITION_TOPBORDER		0x8	// Mask of dwTopBorder member
#define		S3_UTIL_VIDEO_WALL_POSITION_RIGHTBORDER	0x10	// Mask of dwRightBorder member
#define		S3_UTIL_VIDEO_WALL_POSITION_BOTTOMBORDER	0x20	// Mask of dwBottomBorder member
#define		S3_UTIL_VIDEO_WALL_POSITION_XSTEP			0x40		// Mask of dwXStep member
#define		S3_UTIL_VIDEO_WALL_POSITION_YSTEP			0x80		// Mask of dwYStep member
#define		S3_UTIL_VIDEO_WALL_POSITION_LEFT			0x100	// Mask of rectPosition.left member
#define		S3_UTIL_VIDEO_WALL_POSITION_TOP				0x200	// Mask of rectPosition.top member
#define		S3_UTIL_VIDEO_WALL_POSITION_RIGHT			0x400	// Mask of rectPosition.right member
#define		S3_UTIL_VIDEO_WALL_POSITION_BOTTOM		0x800	// Mask of rectPosition.bottom member
#define		S3_UTIL_VIDEO_WALL_POSITION_XRATIO		0x1000	// Mask of dwXRatio member
#define		S3_UTIL_VIDEO_WALL_POSITION_YRATIO		0x2000	// Mask of dwYRatio member
#define		S3_UTIL_VIDEO_WALL_POSITION_ALLFIELDS	0xffffffff	// Mask of dwCellID member

#ifndef tagS3DeviceInfo
#define  tagS3DeviceInfo

// S3DeviceInfo also defined in s3DspAPI.h, please notice that.
typedef struct tagS3DeviceInfo
{
	DWORD      dwDevBit;	// Target device bit (S3_UTIL_DEV_*),
	DWORD      dwAdapter;  	// Adapter that the device is on (indicate which hardware),
	RECT       ViewRect;   	// View rectangle to display from the source view (Refer to MSDN).
} S3DEVICEINFO, FAR * LPS3DEVICEINFO;

#endif

typedef struct tag_QUERY_VIDEO_WALL_ADAPTER_MONITOR_POSITION_INFO
{
	DWORD                                   dwAdapterCount;
	DWORD                                   dwMonitorCount;
} QUERY_VIDEO_WALL_ADAPTER_MONITOR_POSITION_INFO,*LPQUERY_VIDEO_WALL_ADAPTER_MONITOR_POSITION_INFO;


#ifdef	__cplusplus
}
#endif

#endif _S3ESCAPE_H_
