/************************************************************************/
///@brief	Implementation for Win32-Services (Startup-Code)
///@file
///@author	Ing. Markus Eisenmann (FRZEM)				
///@date	Created 2007-10-21 14:10
/// \n		Changed 2007-10-21 14:10
/************************************************************************/

/*-----------------------------------------------------------------------------
 *
 *  service.c - NT Service module
 *
 *    Copyright (c) 1998, SaberNet.net - All rights reserved
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307
 *
 *  $Id: service.c,v 1.3 2007/10/25 18:20:59 frzem Exp $
 *
 *  Comments:
 *	  This module is based on sample code by Craig Link - Microsoft 
 *    Developer Support
 *
 *  Revision history:
 *    02-Aug-98  JRR  Module completed
 *
 *----------------------------------------------------------------------------*/

#define	__BLDSRVRCODE			// Definition f・ Main-File
#include "service.h"
#include "message.h"
#include "ntsl.h"
#include <stdio.h>
#include <direct.h>
#include <time.h>

#if !defined(MSGINF_SRVR_STARTING) || !defined(MSGINF_SRVR_STOPPED)
#include "message.h"
#endif

#define	_tsizeof(_p)	(sizeof(_p) / sizeof(TCHAR))
#define	_MAX_ERRLEN	512		// Max. Textl舅ge ...

static	BOOL PASCAL __InstallService(void);
static	BOOL PASCAL __RemoveService(void);

static	BOOL PASCAL __InitServiceVar(void);
static	BOOL PASCAL __ChgServiceLog(void);

static long __ReadRegDword(HKEY hReg, const char *szKey);

/************************************************************************/

#ifdef	__cplusplus
extern "C" {
#endif

volatile HANDLE _hSrvrStopEvent = NULL;
static	int __iInitialized = 0;

TCHAR	*_pszProgPath = NULL;
TCHAR	*_pszLogFile = NULL;
TCHAR	*_pszSrvrName = NULL;
#ifdef _DEBUG
BOOL	_bConsole = TRUE;
#else
BOOL	_bConsole = FALSE;
#endif
BOOL	_bDbgMode = FALSE;

CRITICAL_SECTION _sSrvrCriSec;
SERVICE_STATUS _sSrvrStatus;
SERVICE_STATUS_HANDLE _hSrvrStatusHandle = NULL;

OSVERSIONINFO osvi;
BOOL bFQDNoutput = TRUE;
int iCharCode = 0;

#ifdef	__cplusplus
}
#endif

/************************************************************************/
/*	Basisfunktionen f・ Logging ...					*/
/************************************************************************/

#if !defined(__STDC__) && (defined(_MSC_VER) && (_MSC_VER >= 1400))
#pragma	warning(push)
#pragma	warning(disable : 4996)
#endif

void	_APIENTRY _vLogEventEx(WORD etype, DWORD dwId, unsigned nParams, va_list* args)
{
    HANDLE  hEvent;

    hEvent = RegisterEventSource(NULL, _pszSrvrName);
    ReportEvent(hEvent, etype, 0, dwId, NULL, (WORD)nParams, 0, (LPCTSTR*)args, NULL);
    DeregisterEventSource(hEvent);
}

void	_VARENTRY _LogEventEx2(WORD etype, DWORD dwId, unsigned nParams, ... )
{
    va_list args;

    va_start(args, nParams);
    _vLogEventEx(etype, dwId, nParams, (va_list*)args);
    va_end(args);
}

void	_VARENTRY _PrintMessage(FILE *fLog, const TCHAR *format, ... )
{
    va_list args;

    va_start(args, format);
    if( fLog != NULL )			// -> Ausgabe aug LOG-Datei
	_vftprintf(fLog, format, args);

    if( _bConsole != FALSE )
	_vtprintf(format, args)	;	// -> Ausgabe auf Bildschirm
    va_end(args);
}

void	_VARENTRY LogMessage(TCHAR chTyp, DWORD dwId,
				const TCHAR *szFile, int iLine, const TCHAR *szFunc,
				const TCHAR *format, unsigned nParams, ... )
{
    register DWORD dwRet;
    LPTSTR lpszTemp = NULL;
    va_list args;
    FILE *fLog = NULL;
    WORD  etype = EVENTLOG_SUCCESS;
    TCHAR   szPuff[_MAX_PATH+1] /*, *pChr*/;
    struct tm*	time_now;
    time_t  secs_now;
    size_t  nLen;

	if (!_bDbgMode) return;

    if( chTyp == _T('E') )		// Ausgabetyp festlegen
		etype = EVENTLOG_ERROR_TYPE;
    else if( chTyp == _T('W') )
		etype = EVENTLOG_WARNING_TYPE;
    else if( chTyp == _T('I') )
		etype = EVENTLOG_INFORMATION_TYPE;
    else if( chTyp == _T('S') )
		etype = EVENTLOG_AUDIT_SUCCESS;
    else if( chTyp == _T('F') )
		etype = EVENTLOG_AUDIT_FAILURE;
    else
    {
	if( ((unsigned)(dwId >> 30) & 0x03u) == 3u )
		chTyp = _T('E'), etype = EVENTLOG_ERROR_TYPE;
	else if( ((unsigned)(dwId >> 30) & 0x03u) == 2u )
		chTyp = _T('W'), etype = EVENTLOG_WARNING_TYPE;
	else if( ((unsigned)(dwId >> 30) & 0x03u) == 1u )
		chTyp = _T('I'), etype = EVENTLOG_INFORMATION_TYPE;
	else
		chTyp = _T('*');	// Kennung unbekannt !
    }

    va_start(args, nParams);
    dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			NULL, dwId, 0uL, (LPTSTR)&lpszTemp, 0, (va_list*)args);
    if( (long)dwRet > 0L )
    {
	if( etype != EVENTLOG_SUCCESS )
		_vLogEventEx(etype, dwId, nParams, (va_list*)args);
    }
    else
    {
	if( format != NULL )
		dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			format, dwId, 0uL, (LPTSTR)&lpszTemp, 0, (va_list*)args);
	if( !((long)dwRet > 0L) )
	{
	    int iLen = _stprintf(szPuff, TEXT("No Messagetext for RC=%c%.8lxh !"), chTyp, dwId);

	    lpszTemp = (LPTSTR)( (lpszTemp != NULL)?
		LocalReAlloc((HLOCAL)lpszTemp, (iLen+1) * sizeof(TCHAR), LMEM_MOVEABLE | LMEM_ZEROINIT) :
		LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, (iLen+1) * sizeof(TCHAR)) );
	    _tcscpy(lpszTemp, szPuff);
	}

	if( etype != EVENTLOG_SUCCESS )
	    _LogEventEx2(etype, dwId, 1u, lpszTemp, NULL);
    }
    va_end(args);

    fLog = _tfopen(_pszLogFile, "a+t");
    time(&secs_now);			// Aktuelle Uhrzeit ermitteln ...
    time_now = localtime(&secs_now);
    nLen = _tcsftime(szPuff, _tsizeof(szPuff), TEXT("%Y-%m-%d %H:%M:%S"), time_now);

    if( lpszTemp != NULL )
    {
	register size_t	i, n = 0;

	for( i = 0 ; lpszTemp[i] != _T('\0') ; i++ )
	{
	    if( lpszTemp[i] != _T('\r') )
	    {
		if( lpszTemp[i] != _T('\n') )
			lpszTemp[n++] = lpszTemp[i];
		else
		{
		    if( (n > 0) && (lpszTemp[n-1] != _T('\n')) )
			lpszTemp[n++] = _T('\n');
		}
	    }
	}
	while( (n > 0) && (lpszTemp[n-1] == _T('\n')) )
	    --n	;
	lpszTemp[n] = _T('\0');
    }
    _PrintMessage(fLog, TEXT("\n%c%.8lx: %s\t%s #%d [%s]\n%s\n"),
		chTyp, dwId, szPuff, szFile, iLine, szFunc, lpszTemp);
    if( !((long)dwRet > 0L) )
    {
	TCHAR	puffer[512];
	TCHAR	*pErg =	GetLastErrorText(puffer, _tsizeof(puffer));

	if( (*pErg) != _T('\0') )	// Zusatztext(e) ausgeben
		_PrintMessage(fLog, TEXT("SYSERR: %s\n"), pErg)	;
    }

    if( fLog != NULL )
	fclose( fLog );			// LOG-Datei schliessen ...
    if( lpszTemp != NULL )
	LocalFree((HLOCAL)lpszTemp);	// -> Speicher freigeben
}


/************************************************************************/

TCHAR*	_APIENTRY _GetErrorText(TCHAR* pszPuf, DWORD dwSize, DWORD dwErrCode)
{
    register DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			NULL, dwErrCode, LANG_NEUTRAL,
			(LPTSTR)&lpszTemp, 0, NULL);
    // supplied buffer is not long enough
    if( !dwRet || ((long)dwSize < (long)(dwRet+14)) )
	*pszPuf = _T('\0');
    else
    {					// remove \r (CR) and \n (LF)
	lpszTemp[lstrlen(lpszTemp)-2] =	_T('\0');
	_stprintf(pszPuf, TEXT("%s (0x%x)"), lpszTemp, dwErrCode);
    }

    if( lpszTemp != NULL )
	LocalFree( lpszTemp );
    return (pszPuf);
}

TCHAR*	_APIENTRY GetLastErrorText(TCHAR* pszPuf, DWORD dwSize)
{
    return _GetErrorText(pszPuf, dwSize, GetLastError());
}

DWORD	_APIENTRY _LogWinError(TCHAR chTyp, DWORD dwId, const TCHAR* pszTxt,
				DWORD dwErrCode, const TCHAR *szFile, int iLine, const TCHAR *szFunc)
{
    TCHAR puffer[_MAX_ERRLEN+1];
    TCHAR header[80+1];
    TCHAR *pszErr = _GetErrorText(puffer, _MAX_ERRLEN, dwErrCode);

    if( IS_INTRESOURCE(pszTxt) )
    {
	if( !(LoadString(GetModuleHandle(NULL),
	    (UINT)LOWORD(pszTxt), header, _tsizeof(header)-1) > 0) )
	    _stprintf(header, TEXT("Message %u"), (UINT)LOWORD(pszTxt));
	header[_tsizeof(header)-1] = _T('\0');
	pszTxt = header;
    }

    puffer[_tsizeof(puffer)-1] = _T('\0');
    LogMessage(chTyp, dwId, szFile, iLine, szFunc,
	TEXT("%1%nError: %2"), 2u, pszTxt, pszErr, NULL);
    return (dwErrCode);
}


/************************************************************************/
/*	Service - Funktionen !						*/
/************************************************************************/

BOOL	WINAPI	SrvrStatus(DWORD dwStatus, DWORD dwExitRC, DWORD dwWait)
{
    static  DWORD dwChckPoint = 1;
    BOOL    bRC = TRUE;

    if( dwStatus == SERVICE_START_PENDING )
	_sSrvrStatus.dwControlsAccepted = 0;
    else
	_sSrvrStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

    _sSrvrStatus.dwCurrentState =   dwStatus;
    _sSrvrStatus.dwWin32ExitCode =  dwExitRC;
    _sSrvrStatus.dwWaitHint =	    dwWait;

    if( (dwStatus == SERVICE_RUNNING) || (dwStatus == SERVICE_STOPPED) )
	_sSrvrStatus.dwCheckPoint = 0;
    else
	_sSrvrStatus.dwCheckPoint = dwChckPoint++ ;

    if( _bConsole == FALSE )
    {
	if( _hSrvrStatusHandle == NULL )
	    return FALSE;

	if( !(bRC = SetServiceStatus(_hSrvrStatusHandle, &_sSrvrStatus)) )
	{
	    DWORD dwErr = __Win32Log(MSGERR_SYSERROR6, TEXT("SetServiceStatus failed"));

	    if( dwErr == ERROR_INVALID_HANDLE )
			_hSrvrStatusHandle = NULL;
	}
    }
    return (bRC);
}

INT	WINAPI	SrvrIsHalting(DWORD dwTim)
{
    HANDLE  hObject = _hSrvrStopEvent;

    if( (hObject != NULL) &&
	(WaitForSingleObject(hObject, dwTim) == WAIT_TIMEOUT) )
	return (0);

//  Service gestoppt oder zu stoppen
    return ( (_hSrvrStopEvent == NULL)?  (-1) : (+1) );
}


VOID	WINAPI	SrvrControl(DWORD dwCtrlCode)
{
    register int iLoop = (int)(3000 / 110);

    switch( dwCtrlCode )
    {
	case SERVICE_CONTROL_STOP:
	    LogMessage(-1, MSGINF_SRVR_STOPPING, __FILE__, __LINE__, __FUNCTION__,
		TEXT("Shutdown of Service '%1' in progress"), 1u, _pszSrvrName, NULL);
	    SrvrStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);

	case SERVICE_CONTROL_SHUTDOWN:	// Set STOP-Event !!!
	    if( _hSrvrStopEvent != NULL )
		SetEvent( _hSrvrStopEvent );
	    SrvrStop(dwCtrlCode);

	    while( (_sSrvrStatus.dwCurrentState ==
			SERVICE_STOP_PENDING) && ((--iLoop) >= 0) )
		Sleep( 110-1 );		// Bis zu 3" warten ...
	    return;

	case SERVICE_CONTROL_INTERROGATE:
	    break;
    }
    SrvrStatus(_sSrvrStatus.dwCurrentState, NO_ERROR, 0);
}

/************************************************************************/

#pragma region "Service - Handler"
static	void _cdecl __ClrSrvr(void)
{
    HANDLE  hObject = _hSrvrStopEvent;

    _hSrvrStopEvent = NULL;
    if( hObject != NULL )		// EVENT-Handle lchen
	CloseHandle( hObject )	;

    if( (--__iInitialized) == 0 )
    {
	DeleteCriticalSection(&_sSrvrCriSec);
    }
}

static	int WINAPI  __RunService(int argc, TCHAR *argv[])
{
    register int iRC = -1;

    ZeroMemory(&_sSrvrStatus, sizeof(_sSrvrStatus));
    _sSrvrStatus.dwServiceSpecificExitCode = 0;
    _sSrvrStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;

    LogMessage(-1, MSGINF_SRVR_STARTING, __FILE__, __LINE__, __FUNCTION__,
	TEXT("Start of Service '%1' in progress"), 1u, _pszSrvrName, NULL);
    if( (_hSrvrStatusHandle = RegisterServiceCtrlHandler(
		_pszSrvrName, SrvrControl)) == NULL )
    {
	__Win32Log(MSGERR_SYSERROR6, TEXT("Could not register service"));
	goto _Cleanup;
    }

    if( !SrvrStatus(SERVICE_START_PENDING, NO_ERROR, 3000) )
	goto _Cleanup;
    else
    {
	if( (iRC = SrvrInit(argc, argv)) != 0 )
	    goto _Cleanup;
    }

    if( !SrvrStatus(SERVICE_RUNNING, NO_ERROR, 0) )
	goto _Cleanup;
    else
    {
	LogMessage(-1, MSGINF_SRVR_STARTED, __FILE__, __LINE__, __FUNCTION__,
		TEXT("Service '%1' started"), 1u, _pszSrvrName, NULL);

	iRC = SrvrRun(argc, argv);

	LogMessage(-1, MSGINF_SRVR_STOPPED, __FILE__, __LINE__, __FUNCTION__,
		TEXT("Service '%1' stopped"), 1u, _pszSrvrName, NULL);
    }

_Cleanup:				// Now Service is stopped !
    SrvrStatus(SERVICE_STOPPED, (DWORD)iRC, 0);
    if( __iInitialized > 0 )
		__ClrSrvr();		// Clear Service-Resource's
    return (iRC);
}
#pragma	endregion

/************************************************************************/

#pragma region "Console - Handler"
static	BOOL WINAPI __CtrlConsole(DWORD dwCtrl)
{
    switch( dwCtrl )
    {
	case CTRL_BREAK_EVENT:	// use Ctrl+C or Ctrl+Break to simulate
	case CTRL_C_EVENT:	// SERVICE_CONTROL_STOP in debug mode
	    _tprintf(TEXT("Stopping %s.\n"), _pszSrvrName);
	    if( _hSrvrStopEvent != NULL )
		SetEvent( _hSrvrStopEvent );
	    SrvrStop(SERVICE_CONTROL_STOP);
	    return TRUE;
    }
    return FALSE;
}

static	INT WINAPI  __RunConsole(int argc, TCHAR *argv[])
{
    register int iRC =	-1;
    DWORD   dwArgc;
    LPTSTR* lpszArgv;

#if defined(UNICODE) || defined(_UNICODE)
    lpszArgv =	CommandLineToArgvW(GetCommandLineW(), &dwArgc));
#else
    dwArgc =	(DWORD)argc;
    lpszArgv =	argv;
#endif

    ZeroMemory(&_sSrvrStatus, sizeof(_sSrvrStatus));
    _tprintf(TEXT("Initializing %s.\n"), _pszSrvrName);

#ifdef	_DEBUG
    if( _bDbgMode != FALSE )
	_tprintf(TEXT("Debugging %s.\n"), _pszSrvrName);
#endif
    SetConsoleCtrlHandler(__CtrlConsole, TRUE);

    if( (iRC = SrvrInit((int)dwArgc, lpszArgv)) != 0 )
    {
	_tprintf(TEXT("SrvrInit(): RC=%d\n"), iRC);
	goto _Cleanup;
    }

    _tprintf(TEXT("Running %s.\n"), _pszSrvrName);
    if( (iRC = SrvrRun((int)dwArgc, lpszArgv)) != 0 )
	_tprintf(TEXT("SrvrRun(): RC=%d\n"), iRC);

    _tprintf(TEXT("Stopped %s.\n"), _pszSrvrName);

_Cleanup:				// Now Service is stopped !
    if( __iInitialized > 0 )
		__ClrSrvr();		// Clear Service-Resource's
    return (iRC);
}
#pragma	endregion


/************************************************************************/
/*	Funktionen zur Service-Installation bzw. -Initialisierung	*/
/************************************************************************/

#pragma region "__InstallService: Als Service installieren"
static	BOOL PASCAL __AddMsgEvent(const TCHAR* pszSrvr, const TCHAR* pszPath)
{
    register BOOL bRC = TRUE;
    DWORD   dwData;
    HKEY    hk;
    TCHAR   key[256+1];

    _sntprintf(key, _tsizeof(key)-1,
	    TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s"), pszSrvr);
    key[_tsizeof(key)-1] = _T('\0');

    if( RegCreateKey(HKEY_LOCAL_MACHINE, key, &hk) )
    {	
	_tprintf(TEXT("Could not create the registry key!\n"));
	return FALSE;
    }

    if( RegSetValueEx(hk, TEXT("EventMessageFile"),
		0, REG_EXPAND_SZ,
		(LPBYTE)pszPath,
		(DWORD)_tcslen(pszPath) + 1) )
    {
	_tprintf(TEXT("Could not set the event message file!\n"));
	bRC = FALSE;			// Fehler aufgetretten
    }

    dwData = EVENTLOG_ERROR_TYPE |
			EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    if( RegSetValueEx(hk, TEXT("TypesSupported"),
		0, REG_DWORD,
		(LPBYTE)&dwData, sizeof(DWORD)) )
    {
	_tprintf(TEXT("Could not set the supported types!\n"));
	bRC = FALSE;			// Fehler aufgetretten
    }

    RegCloseKey(hk);
    return (bRC);
}

static	BOOL PASCAL __InstallService(void)
{
    register int iOkay = -1;
    SC_HANDLE	shManager;
    SC_HANDLE	shService;
    TCHAR	puffer[_MAX_ERRLEN+1];

    if( !__AddMsgEvent(_pszSrvrName, _pszProgPath) )
	return FALSE;			// Register EVENT-Source(s) failed

    shManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if( shManager == NULL )
    {
	_tprintf(TEXT("OpenSCManager failed - %s\n"),
		    GetLastErrorText(puffer, _MAX_ERRLEN));
	return FALSE;
    }

    shService = CreateService(shManager,
	CREATE_SERVICE_lpServiceName,	// name of service
	CREATE_SERVICE_lpDisplayName,	// name of display
	CREATE_SERVICE_dwDesiredAccess,	// desired access (SERVICE_ALL_ACCESS)
	CREATE_SERVICE_dwServiceType,	// type of service
	CREATE_SERVICE_dwStartType,	// start type
	CREATE_SERVICE_dwErrorControl,	// error control type
	_pszProgPath,			// service's binary
	CREATE_SERVICE_lpLoadOrderGroup,
	CREATE_SERVICE_lpdwTagId,	// no tag identifier
	CREATE_SERVICE_lpDependencies,	// dependencies
	CREATE_SERVICE_lpServiceAccount,
	CREATE_SERVICE_lpStartPassword	// User-ID and password
	);

    if( shService == NULL )
	_tprintf(TEXT("CreateService failed - %s\n"),
		    GetLastErrorText(puffer, _MAX_ERRLEN));
    else
    {
	iOkay = _tprintf(TEXT("%s installed.\n"), _pszSrvrName)	;
	CloseServiceHandle(shService);
    }

    CloseServiceHandle(shManager);
    return (BOOL)(!(iOkay < 0));
}
#pragma	endregion

#pragma region "__RemoveService: Diensteintrag entfernen"
static	BOOL PASCAL __RemoveService(void)
{
    register int iOkay = -1;
    SC_HANDLE	shManager;
    SC_HANDLE	shService;
    TCHAR	puffer[_MAX_ERRLEN+1];

    shManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if( shManager == NULL )
    {
	_tprintf(TEXT("OpenSCManager failed - %s\n"),
		    GetLastErrorText(puffer, _MAX_ERRLEN));
	return FALSE;
    }

    shService =	OpenService(shManager, _pszSrvrName, SERVICE_ALL_ACCESS);
    if( shService == NULL )
	_tprintf(TEXT("OpenService failed - %s\n"),
		    GetLastErrorText(puffer, _MAX_ERRLEN));
    else
    {
	if( ControlService(shService, SERVICE_CONTROL_STOP, &_sSrvrStatus) )
	{
	    _tprintf(TEXT("Stopping %s ."), _pszSrvrName);
	    Sleep(1000);

	    while( QueryServiceStatus(shService, &_sSrvrStatus) )
	    {
		if( _sSrvrStatus.dwCurrentState == SERVICE_STOP_PENDING )
		    _tprintf(TEXT("."));
		else
		    break;
		Sleep( 1000 );
	    }

	    if( _sSrvrStatus.dwCurrentState == SERVICE_STOPPED )
		_tprintf(TEXT("\n%s stopped.\n"), _pszSrvrName);
	    else
		_tprintf(TEXT("\n%s failed to stop.\n"), _pszSrvrName);
	}

	if( DeleteService(shService) )
	    iOkay = _tprintf(TEXT("%s removed.\n"), _pszSrvrName);
	else
	    _tprintf(TEXT("DeleteService failed - %s\n"),
			    GetLastErrorText(puffer, _MAX_ERRLEN));

	CloseServiceHandle(shService);
    }

    CloseServiceHandle(shManager);
    return (BOOL)( !(iOkay < 0) );
}
#pragma	endregion

/************************************************************************/

static long __ReadRegDword(HKEY hReg, const char *szKey)
{
    LONG    rc;
    DWORD   val, typ;
    DWORD   size = sizeof(val);

    rc = RegQueryValueEx(hReg, szKey, NULL, &typ, (BYTE*)&val, &size) ;
    if( (rc == ERROR_SUCCESS) && (typ == REG_DWORD) )
        return  (long)val ;
// Failure: Type or Value does not exist!
    return  -1L;
}

#pragma region "__InitServiceVar: Namen und Pfade ermitteln"
static	BOOL PASCAL __InitServiceVar(void)
{
    register unsigned tmp, help = 0;
    TCHAR szDrive[_MAX_DRIVE+1], szDir[_MAX_DIR+1],
	szName[_MAX_FNAME+1], szExt[_MAX_EXT+1]	;
    TCHAR szProg[_MAX_PATH+1], szTemp[_MAX_PATH+1];
	HKEY	hReg;
	long	rc, rv;

    if( GetModuleFileName(NULL, szProg, _tsizeof(szProg)) > 0 )
    {
	szProg[_tsizeof(szProg)-1] = _T('\0');
	_pszProgPath = _tcsdup( szProg );

	_tsplitpath(szProg, szDrive, szDir, szName, szExt);
	szDrive[_tsizeof(szDrive)-1] = szDir[_tsizeof(szDir)-1] =
	    szName[_tsizeof(szName)-1] = szExt[_tsizeof(szExt)-1] = _T('\0');

	_tmakepath(szTemp, szDrive, szDir, szName, ".Log");
	szTemp[_tsizeof(szTemp)-1] = _T('\0');
	_pszLogFile = _tcsdup( szTemp );
	_pszSrvrName = _tcsdup( _tcsupr(szName) );

/*
 *  Aktuelle Laufwerk bzw. verzeichnis setzen !
 */
	for( tmp=0 ; szTemp[tmp] != _T('\0') ; tmp++ )
	{
	    if( (szTemp[tmp] == _T('\\')) || (szTemp[tmp] == _T('/')) )
		help = tmp;
	}
	szTemp[help] = _T('\0');
	if( help > 0u )
	    _chdir(szTemp),
	    _chdrive((szTemp[0] - 'A' + 1) & 0x1f);
    }

	// Windows バージョン取得
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx (&osvi);

	// オプションの読み込み
	iCharCode = 0; // デフォルトはUTF8
	bFQDNoutput = true; // デフォルトはフルコンピュータ名出力

	rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, NTSYSLOG_REGISTRY_KEY, 0, KEY_READ, &hReg);
	if( rc == ERROR_SUCCESS )
	{
		rv = __ReadRegDword(hReg, "FullComputerName") ;
		if (rv == 1) bFQDNoutput = FALSE; else bFQDNoutput = true;
		rv = __ReadRegDword(hReg, "CharCode") ;
		if (rv != -1) iCharCode = (int)rv;
	}
	RegCloseKey(hReg);

    return (BOOL)((_pszProgPath != NULL) &&
	(_pszLogFile != NULL) && (_pszSrvrName != NULL));
}
#pragma	endregion

/************************************************************************/
/*	サービスメイン処理                                                  */
/************************************************************************/

int _tmain(int argc, TCHAR *argv[])
{
    BOOL bSetOK = __InitServiceVar();	// Program-, Service-Name setzen

    SERVICE_TABLE_ENTRY	dispatchTable[] =
    {
		{ _pszSrvrName, (LPSERVICE_MAIN_FUNCTION)__RunService },
		{ (TCHAR*)NULL, (LPSERVICE_MAIN_FUNCTION)NULL }
    };

	if( bSetOK == FALSE )
    {
	__Win32Log(MSGERR_RTLERROR7, TEXT("Allocation of Memory failed"));
	return (1);
    }

    if( argc > 1 )
    {
	register int tmp = 0;

	while( (++tmp) < argc )
	{
	    if( (argv[tmp][0] == '-') || (argv[tmp][0] == '/') )
	    {
		if( _tcsicmp(argv[tmp]+1, TEXT("install")) == 0 )
		{
		    if( !__InstallService() )
			exit( 2 );
		    return (0);
		}

		if( _tcsicmp(argv[tmp]+1, TEXT("remove")) == 0 )
		{
		    if( !__RemoveService() )
			exit( 2 );
		    return (0);
		}

		if( (_tcsicmp(argv[tmp]+1, TEXT("?")) == 0)
			|| (_tcsicmp(argv[tmp]+1, TEXT("help")) == 0) )
		{
			_tprintf("\n%s Version %s.%s\n%s\n\n", APP_NAME, VERSION_MAJOR, VERSION_MINOR, COPYRIGHT);
		    _tprintf(TEXT(  "Usage: %s [Param]\n"
				    "-install = install service\n"
				    "-remove =  remove service\n"
				    "-console = run as console application\n"
				    "-debug =   run in debug-mode\n"), _pszSrvrName);
		    return (0);
		}

		if( _tcsicmp(argv[tmp]+1, TEXT("console")) == 0 )
		    _bConsole =	TRUE;
		else if( _tcsicmp(argv[tmp]+1, TEXT("debug")) == 0 )
		    _bDbgMode = TRUE;
		else
		{
		    _tprintf(TEXT("%s: Unrecognized option: %s\n"), _pszSrvrName, argv[tmp]);
		    return (1);
		}
	    }
	}
    }

    InitializeCriticalSection(&_sSrvrCriSec);
    __iInitialized = 1;			/* Initial.-Flag    */
    atexit(__ClrSrvr);

    _hSrvrStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if( _hSrvrStopEvent == NULL )
    {
	__Win32Log(MSGERR_SYSERROR6, TEXT("CreateEvent failed"));
	exit( 1 );
    }

    if( _bConsole != FALSE )		/* Run as console ? */
	exit( __RunConsole(argc, argv) );

    _tprintf(TEXT(  "\nCalling StartServiceCtrlDispatcher()\n"
		    "\t... please wait.\n"));
    if( !StartServiceCtrlDispatcher(dispatchTable) )
	{
	__Win32Log(MSGERR_SYSERROR6, TEXT("Could not start service"));
	exit( 1 );
    }
    return (0);
}

#if !defined(__STDC__) && (defined(_MSC_VER) && (_MSC_VER >= 1400))
#pragma	warning(pop)
#endif
