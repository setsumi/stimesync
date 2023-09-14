// ---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <sstream>
#include <iostream>
#include <iomanip>
#include <psapi.h>
#include <System.DateUtils.hpp>

#include "tools.h"

// ---------------------------------------------------------------------------
String tl_GetModuleName()
{
	wchar_t buf[MAX_PATH];
	GetModuleFileName(NULL, buf, MAX_PATH);
	return (String)buf;
}

// ---------------------------------------------------------------------------
String tl_GetProgramPath() // path with trailing '\'
{
	static String Path = tl_GetModuleName();
	return Path.SubString(1, Path.LastDelimiter(L"\\"));
}

// ---------------------------------------------------------------------------
void tl_RunInMainThread(FTCdef FuncToCall, const String fP1)
{
	struct Args
	{
		String P1;
		FTCdef FTC;

		void __fastcall ExecFuncQueue()
		{
			try
			{
				FTC(P1);
			}
			__finally
			{
				delete this;
			}
		}
	};

	Args *args = new Args;
	args->P1 = fP1;
	args->FTC = FuncToCall;
	TThread::Queue(NULL, &args->ExecFuncQueue);
}

// ---------------------------------------------------------------------------
String BytesToHexStr(unsigned char *bytes, int count)
{
	if (count <= 0)
		return L"";

	std::wstringstream ss;
	ss << std::hex << std::uppercase;
	for (int i = 0; i < count; i++)
	{
		ss << std::setw(2) << std::setfill(L'0') << (int)bytes[i] << L" ";
	}
	String hexStr(ss.str().c_str());
	return hexStr.TrimRight();
}

// ---------------------------------------------------------------------------
String BytesToHexStr(const TIdBytes &bytes)
{
	if (bytes.Length <= 0)
		return L"";

	std::wstringstream ss;
	ss << std::hex << std::uppercase;
	for (int i = 0; i < bytes.Length; i++)
	{
		ss << std::setw(2) << std::setfill(L'0') << (int)bytes[i] << L" ";
	}
	String hexStr(ss.str().c_str());
	return hexStr.TrimRight();
}

// ---------------------------------------------------------------------------
String GetWindowClassPlus(HWND hwnd)
{
	wchar_t *tbuf = new wchar_t[2048];
	tbuf[0] = L'\0';
	GetClassName(hwnd, tbuf, 2047);
	String ret(tbuf);
	delete[]tbuf;
	return ret;
}

// ---------------------------------------------------------------------------
String GetWindowTitlePlus(HWND hwnd)
{
	wchar_t *tbuf = new wchar_t[2048];
	tbuf[0] = L'\0';
	GetWindowText(hwnd, tbuf, 2047);
	String ret(tbuf);
	delete[]tbuf;
	return ret;
}

// ---------------------------------------------------------------------------
DWORD PSAPI_EnumProcesses(std::list<DWORD>& listProcessIDs, DWORD dwMaxProcessCount)
{
	DWORD dwRet = NO_ERROR;
	listProcessIDs.clear();
	DWORD *pProcessIds = new DWORD[dwMaxProcessCount];
	DWORD cb = dwMaxProcessCount*sizeof(DWORD);
	DWORD dwBytesReturned = 0;

	// call PSAPI EnumProcesses
	if (::EnumProcesses(pProcessIds, cb, &dwBytesReturned))
	{
		// push returned process IDs into the output list
		const int nSize = dwBytesReturned / sizeof(DWORD);
		for (int nIndex = 0; nIndex < nSize; nIndex++)
		{
			listProcessIDs.push_back(pProcessIds[nIndex]);
		}
	}
	else
	{
		dwRet = ::GetLastError();
	}
	delete[]pProcessIds;
	return dwRet;
}

// ---------------------------------------------------------------------------
int t_DateTimeMs(const TDateTime &dtm)
{
	Word dummy, ms;
	DecodeDateTime(dtm, dummy, dummy, dummy, dummy, dummy, dummy, ms);
	return (int)ms;
}

// ---------------------------------------------------------------------------
String t_DoubleToStr(double dbl)
{
	String str;
	str.printf(L"%f", dbl);
	return str;
}

// ---------------------------------------------------------------------------
String t_MsDigit(const TDateTime &dt)
{
	String ms;
	ms.printf(L"%03d", t_DateTimeMs(dt));
	// base index 1
	return ms[1];
}

// ---------------------------------------------------------------------------
String t_MsFullDigits(const TDateTime &dt)
{
	String ms;
	ms.printf(L"%03d", t_DateTimeMs(dt));
	return ms;
}

// ---------------------------------------------------------------------------
String t_FormatDateTimeMs(const TDateTime &dt)
{
	String str;
	str.printf(L"%s.%s", FormatDateTime(L"dd.mm.yyyy hh:nn:ss", dt).w_str(),
		t_MsDigit(dt).w_str());
	return str;
}

// ---------------------------------------------------------------------------
String t_FormatTimeMs(const TDateTime &dt)
{
	String str;
	str.printf(L"%s.%s", FormatDateTime(L"hh:nn:ss", dt).w_str(),
		t_MsFullDigits(dt).w_str());
	return str;
}

// ---------------------------------------------------------------------------
