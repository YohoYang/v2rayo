#include "stdafx.h"
#include "v2ray.h"
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <shellapi.h>
#include <tchar.h>
#include <Psapi.h>
#include <wininet.h>

bool switch_proxy = false;
bool V2Ray::Return_switch_proxy() {
	return switch_proxy;
}
V2Ray::~V2Ray()
{
	kill();
}

void V2Ray::init(HWND hWnd, std::string configFile)
{
	m_hWnd = hWnd;
	start();
}

void V2Ray::start()
{
	PROCESS_INFORMATION processInfo{};
	STARTUPINFO startUpInfo{};
	startUpInfo.cb = sizeof(STARTUPINFO);
	startUpInfo.dwFlags = STARTF_USESHOWWINDOW;
	startUpInfo.wShowWindow = SW_HIDE;
	
	if (!CreateProcess(NULL, _tcsdup(_T("v2ray.exe")), NULL, NULL, FALSE, NULL, NULL, _T("."), &startUpInfo, &processInfo))
	{
		_tcprintf(_T("cannot create process %d\n"), GetLastError());
	}
	m_hProcess = processInfo.hProcess;
	setProxy();
}

void V2Ray::kill()
{
	TerminateProcess(m_hProcess, 0);
}


void V2Ray::editConfig()
{
	ShellExecute(m_hWnd, NULL, _T("config.json"), NULL, NULL, SW_SHOW);
}

void V2Ray::setProxy()
{
	switch_proxy = true;
	INTERNET_PER_CONN_OPTION_LIST    List;
	INTERNET_PER_CONN_OPTION         Option[2];
	unsigned long                    nSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);

	Option[0].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
	Option[0].Value.pszValue = _T("http://localhost:5000");

	Option[1].dwOption = INTERNET_PER_CONN_FLAGS;
	Option[1].Value.dwValue = PROXY_TYPE_PROXY;

	List.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);
	List.pszConnection = NULL;
	List.dwOptionCount = 2;
	List.dwOptionError = 0;
	List.pOptions = Option;

	if (!InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &List, nSize))
		printf("InternetQueryOption failed! (%d)\n", GetLastError());
}

void V2Ray::setNoProxy()
{
	switch_proxy = false;
	INTERNET_PER_CONN_OPTION_LIST    List;
	INTERNET_PER_CONN_OPTION         Option[2];
	unsigned long                    nSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);

	Option[0].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
	Option[0].Value.pszValue = NULL;

	Option[1].dwOption = INTERNET_PER_CONN_FLAGS;
	Option[1].Value.dwValue = PROXY_TYPE_DIRECT;

	List.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);
	List.pszConnection = NULL;
	List.dwOptionCount = 2;
	List.dwOptionError = 0;
	List.pOptions = Option;

	if (!InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &List, nSize))
		printf("InternetQueryOption failed! (%d)\n", GetLastError());
}

void V2Ray::restart()
{
	kill();
	start();
}
