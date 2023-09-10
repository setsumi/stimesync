// ---------------------------------------------------------------------------

#include <vcl.h>
#include <System.DateUtils.hpp>
#include <System.Diagnostics.hpp>
#include <System.IniFiles.hpp>
#include <System.hpp>
#include <iphlpapi.h>

#pragma hdrstop

#include "Unit1.h"
#include "tools.h"
// ---------------------------------------------------------------------------
#pragma comment(lib, "iphlpapi.lib")
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

// ---------------------------------------------------------------------------
namespace config
{
    const String file = ChangeFileExt(Application->ExeName, L".ini");

	int server = 0;
	bool retry = false;
	bool autoSync = false;
	bool autoExit = false;
}

// ---------------------------------------------------------------------------
// Constructor
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner)
{
	Hint1 = NULL;
	syncNext = true;
	failedList = new TStringList();

	// set high process priority
	if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
	{
		String str;
		str.printf(L"Failed to set high process priority : %d", GetLastError());
		MessageBox(Handle, str.w_str(), Application->Title.w_str(), MB_OK | MB_ICONERROR);
	}

	Caption = Application->Title;
	UpdateUiTimezone();
	// suppress autoselect text in combobox
	PostMessage(cmbServer->Handle, CB_SETEDITSEL, (WPARAM) - 1, 0);
    LoadConfig();
	UpdateUiClear();
}

//---------------------------------------------------------------------------
void __fastcall TForm1::FormDestroy(TObject *Sender)
{
    SaveConfig();
	delete failedList;
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::btnSyncClick(TObject *Sender)
{
	syncNext = true;
	failedList->Clear();
	btnSync->Enabled = false;
	btnAbort->Enabled = true;

	Sync(cmbServer->Text);
}

//---------------------------------------------------------------------------
void __fastcall TForm1::btnAbortClick(TObject *Sender)
{
	syncNext = false;
	btnAbort->Enabled = false;
}

//---------------------------------------------------------------------------
void TForm1::SyncDone()
{
	btnSync->Enabled = true;
	btnAbort->Enabled = false;
}

//---------------------------------------------------------------------------
void TForm1::SyncNext()
{
	// start next
	tmrSyncNext->Enabled = true;
}

//---------------------------------------------------------------------------
void __fastcall TForm1::tmrSyncNextTimer(TObject *Sender)
{
	tmrSyncNext->Enabled = false;

	if (!syncNext)
	{
		SyncDone();
        return;
	}

	int index = cmbServer->ItemIndex + 1;
	if (index > cmbServer->Items->Count - 1)
	{
		index = 0;
	}

	if (failedList->IndexOf(cmbServer->Items->Strings[index]) != -1)
	{
		SyncDone();
		return;
	}

	cmbServer->ItemIndex = index;
	Sync(cmbServer->Text);
}

// ---------------------------------------------------------------------------
void TForm1::Sync(String server)
{
	TIdSNTP *sntp = new TIdSNTP(NULL);
	sntp->ReceiveTimeout = 1000;
	sntp->Host = server;

	UpdateUiClear();
	lblHostSync->Caption = L"...";
	Application->ProcessMessages();

	bool success = false;
	try
	{
		try
		{
			// resolve SNTP server hostname
			TStringList *ips = new TStringList();
			const bool resolved = ResolveHost(sntp->Host, ips);

			for (int i = 0; i < ips->Count; i++)
				Memo1->Lines->Add((*ips)[i]);
			delete ips;
			ScrollToTop(Memo1);
			Application->ProcessMessages();

			if (!resolved)
			{
				lblHostSync->Caption = L"";
				UpdateUiSyncError(L"Error", L"Unable to resolve server host");
			}
			else
			{
				// The date 30.12.1899 (zero), is used to represent empty dates
				TDateTime oldtime = Now();

				TStopwatch stopwatch;
				stopwatch.Reset();
				stopwatch.Start();
				const bool synced = sntp->SyncTime(); // SYNC THE TIME
				stopwatch.Stop();

				success = synced;
				TDateTime adj = sntp->AdjustmentTime;
				TDateTime adj_abs = fabs(adj.Val);
				TDateTime servtime = sntp->DateTime;

				// update with result
				lblHostSync->Caption = sntp->Host;
				UpdateUiSync(synced, stopwatch.ElapsedMilliseconds);
				String str;
				str.printf(L"%s%dd %s.%03d", adj.Val < 0 ? L"-" : L"+",
					DaysBetween(0, adj_abs), adj_abs.TimeString().w_str(), t_DateTimeMs(adj_abs));
				edtOffset->Text = str;
				if (servtime.Val == 0)
				{
					success = false;
					edtServerTime->Color = clYellow;
					edtServerTime->Font->Color = clRed;
				}
				edtLocalTime->Text = t_DoubleToStr(oldtime);
				edtServerTime->Text = t_DoubleToStr(servtime);
			}
		}
		catch (Exception &ex)
		{
			UpdateUiSyncError(L"Error", ex.Message);
		}
	}
	__finally
	{
		delete sntp;
	}

	if (success)
	{
		SyncDone();
	}
	else
	{
		failedList->Add(server);

		if (syncNext)
		{
			SyncNext();
		}
		else
		{
			SyncDone();
		}
	}
}

// ---------------------------------------------------------------------------
// Return iplist example:
// DNS
// 192.168.1.1
// HOST
// 162.159.200.1
// 162.159.200.123
// 79.160.225.150
// 91.189.182.32
// ---------------------------------------------------------------------------
bool TForm1::ResolveHost(String host, TStringList *iplist)
{
	iplist->Clear();
	TIdDNSResolver* dns = new TIdDNSResolver(NULL);
	dns->WaitingTime = 2000;
	dns->QueryType = TQueryType() << qtA;
	try
	{
		// Acquire system DNS info
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
		FIXED_INFO *pFixedInfo;
		ULONG ulOutBufLen;
		DWORD dwRetVal;
		IP_ADDR_STRING *pIPAddr;

		pFixedInfo = (FIXED_INFO*) MALLOC(sizeof(FIXED_INFO));
		if (pFixedInfo == NULL)
		{
			throw new Exception
				(L"Error allocating memory needed to call GetNetworkParams");
		}
		ulOutBufLen = sizeof(FIXED_INFO);

		// Make an initial call to GetNetworkParams to get
		// the necessary size into the ulOutBufLen variable
		if (GetNetworkParams(pFixedInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
		{
			FREE(pFixedInfo);
			pFixedInfo = (FIXED_INFO*) MALLOC(ulOutBufLen);
			if (pFixedInfo == NULL)
			{
				throw new Exception
					(L"Error allocating memory needed to call GetNetworkParams");
			}
		}

		if (dwRetVal = GetNetworkParams(pFixedInfo, &ulOutBufLen) != NO_ERROR)
		{
			FREE(pFixedInfo);
			String str;
			str.printf(L"GetNetworkParams failed : %d", dwRetVal);
			throw new Exception(str);
		}
		else
		{
			iplist->Add(L"DNS"); // header

			pIPAddr = &pFixedInfo->DnsServerList;
			while (pIPAddr)
			{
				iplist->Add(pIPAddr->IpAddress.String);
				pIPAddr = pIPAddr->Next;
			}

			FREE(pFixedInfo);
		}

		if (iplist->Count < 2) // header and at least one server
		{
			throw new Exception(L"System DNS servers not found");
		}

		// Resolve host using acquired DNS addresses
		const int max_index = iplist->Count - 1;
		int index = 1;
		iplist->Add(L"HOST"); // header

try_resolve:
		dns->Host = (*iplist)[index];
		try
		{
			dns->Resolve(host);

			for (int i = 0; i < dns->QueryResult->Count; i++)
			{
				TResultRecord *r = dns->QueryResult->Items[i];
				if (r->RecType == qtA)
				{
					iplist->Add(((TARecord*)r)->IPAddress);
				}
			}
		}
		catch (...)
		{
			index++;
			if (index <= max_index)
				goto try_resolve;
		}
	}
	__finally
	{
		delete dns;
	}

	const int header_index = iplist->IndexOf(L"HOST");
	if (header_index == -1 || !(iplist->Count - 1 > header_index))
		return false;

	return true;
}

// ---------------------------------------------------------------------------
void TForm1::UpdateUiClear()
{
	lblHostSync->Caption = L"";
	edtSynced->Clear();
	edtSynced->Color = clWindow;
	edtSynced->Hint = L"";
	edtOffset->Clear();
	edtLocalTime->Clear();
	edtServerTime->Clear();
	edtServerTime->Color = clWindow;
	edtServerTime->Font->Color = clWindowText;
	Memo1->Clear();
}

// ---------------------------------------------------------------------------
void TForm1::ScrollToTop(TMemo *memo)
{
	memo->SelStart = 0;
	PostMessage(memo->Handle, EM_SCROLLCARET, 0, 0);
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::tmrUiTimer(TObject *Sender)
{
	UpdateUiTimezone();
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::tmrUiMsTimer(TObject *Sender)
{
	UpdateUiDateTime();
}

// ---------------------------------------------------------------------------
void TForm1::UpdateUiTimezone()
{
	String str;
	str.printf(L"(%s) %s", TTimeZone::Local->Abbreviation.w_str(), TTimeZone::Local->DisplayName.w_str());
	lblTimezone->Caption = str;
}

// ---------------------------------------------------------------------------
void TForm1::UpdateUiDateTime()
{
	TDateTime now = Now();
	String ms = t_DateTimeMs(now);
	if (ms.Length() > 0)
		ms = ms[1]; // base index 1
	String str;
	str.printf(L"%s.%s", FormatDateTime(L"dddd, d mmmm yyyy, hh:nn:ss", now).w_str(), ms.w_str());
	lblDateTime->Caption = str;
}

// ---------------------------------------------------------------------------
void TForm1::UpdateUiSync(bool synced, __int64 elapsed)
{
	if (synced)
	{
		String str;
		str.printf(L"OK [%dms]", elapsed);
		edtSynced->Text = str;
		edtSynced->Color = clLime;
		edtSynced->Font->Color = clBlack;
	}
	else
	{
		String str;
		str.printf(L"Fail [%dms]", elapsed);
		UpdateUiSyncError(str, L"Could not synchronize the time");
	}
}

// ---------------------------------------------------------------------------
void TForm1::UpdateUiSyncError(String title, String hint)
{
	String str;
	str.printf(L"%s: %s", title.w_str(), hint.w_str());
	edtSynced->Text = str;
	edtSynced->Color = clRed;
	edtSynced->Font->Color = clYellow;
	edtSynced->Hint = hint; // error text
	edtSynced->Tag = 0; // error image
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::edtSyncedMouseLeave(TObject *Sender)
{
	if (Hint1)
	{
		delete Hint1;
		Hint1 = NULL;
	}
}

//---------------------------------------------------------------------------
void __fastcall TForm1::edtSyncedMouseEnter(TObject *Sender)
{
	if (Hint1)
	{
		delete Hint1;
		Hint1 = NULL;
	}

	TControl *control = (TControl*)Sender;
	if (!control->Hint.IsEmpty())
	{
		Hint1 = new TBalloonHint((TControl*)Sender);
		Hint1->Images = ImageList1;
		Hint1->Delay = 0;
		Hint1->ImageIndex = control->Tag; // hint image index
		Hint1->Description = control->Hint; // hint text
		Hint1->ShowHint((TControl*)Sender);
	}
}

//---------------------------------------------------------------------------
void __fastcall TForm1::FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
	switch(Key)
	{
		case VK_RETURN:
			if (Shift.Contains(ssCtrl)) // Ctrl+Enter
			{
				if (btnSync->Enabled)
				{
					btnSyncClick(btnSync);
				}
			}
			break;
		case VK_ESCAPE:
			if (btnAbort->Enabled)
			{
				btnAbortClick(btnAbort);
			}
			break;
		default:
			break;
	}
}

//---------------------------------------------------------------------------
void TForm1::LoadConfig()
{
	if (!FileExists(config::file))
	{
		SaveConfig();

		TIniFile *ini = new TIniFile(config::file);
		TStringList *list = new TStringList();
		list->Add(L"ntp.nict.jp");
		list->Add(L"pool.ntp.org");
		list->Add(L"time.google.com");
		list->Add(L"time.cloudflare.com");
		list->Add(L"time.windows.com");
		list->Add(L"time.apple.com");
		list->Add(L"time.facebook.com");
		list->Add(L"time.esa.int");
		for (int i = 0; i < list->Count; i++)
		{
			ini->WriteString(L"Servers", IntToStr(i), list->Strings[i]);
		}
		delete list;
		delete ini;
	}

	TIniFile *ini = new TIniFile(config::file);

	config::server = ini->ReadInteger(L"Options", L"Server", 0);
	config::retry    = ini->ReadBool(L"Options", L"Retry", false);
	config::autoExit = ini->ReadBool(L"Options", L"AutoExit", false);
	config::autoSync = ini->ReadBool(L"Options", L"AutoSync", false);

	for (int i = 0;; i++)
	{
		String server = ini->ReadString(L"Servers", IntToStr(i), L"");
		if (server.IsEmpty()) break;
		cmbServer->Items->Add(server);
	}

    UpdateUiConfig(false);

	delete ini;
}

//---------------------------------------------------------------------------
void TForm1::SaveConfig()
{
	TIniFile *ini = new TIniFile(config::file);

	ini->WriteInteger(L"Options", L"Server", config::server);
	ini->WriteBool(L"Options", L"Retry", config::retry);
	ini->WriteBool(L"Options", L"AutoExit", config::autoExit);
	ini->WriteBool(L"Options", L"AutoSync", config::autoSync);

	delete ini;
}

//---------------------------------------------------------------------------
void TForm1::UpdateUiConfig(bool uiToConfig)
{
	if (uiToConfig)
	{
		config::server = cmbServer->ItemIndex;
		config::retry = chkRetry->Checked;
		config::autoSync = chkAutosync->Checked;
		config::autoExit = chkExit->Checked;
	}
	else
	{
		cmbServer->Tag = 666; // disable recursive event
		cmbServer->ItemIndex = config::server;
		cmbServer->Tag = 0;
		chkRetry->Tag = 666;
		chkRetry->Checked = config::retry;
		chkRetry->Tag = 0;
		chkAutosync->Tag = 666;
		chkAutosync->Checked = config::autoSync;
		chkAutosync->Tag = 0;
		chkExit->Tag = 666;
		chkExit->Checked = config::autoExit;
		chkExit->Tag = 0;
	}
}

//---------------------------------------------------------------------------
void __fastcall TForm1::chkRetryClick(TObject *Sender)
{
	if (((TControl*)Sender)->Tag != 0) return; // disable recursive OnClick
	UpdateUiConfig(true);
}

//---------------------------------------------------------------------------

