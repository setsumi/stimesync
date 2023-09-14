// ---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdSNTP.hpp>
#include <IdDNSResolver.hpp>
#include <IdTCPConnection.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ActnMan.hpp>
#include <Vcl.PlatformDefaultStyleActnCtrls.hpp>

// ---------------------------------------------------------------------------
typedef struct SYNCREALPARAMS
{
	String server;
	bool onlyGet;

	SYNCREALPARAMS()
	{
		onlyGet = true;
	}
} SYNCREALPARAMS;

// ---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published: // IDE-managed Components
	TButton *btnSync;
	TEdit *edtSynced;
	TEdit *edtOffset;
	TEdit *edtLocalTime;
	TEdit *edtServerTime;
	TComboBox *cmbServer;
	TLabel *lblHostSync;
	TMemo *Memo1;
	TLabel *lblTimezone;
	TTimer *tmrUi;
	TLabel *lblDateTime;
	TTimer *tmrUiMs;
	TImageList *ImageList1;
	TLabel *Label1;
	TLabel *Label2;
	TButton *btnAbort;
	TTimer *tmrSyncNext;
	TButton *btnGet;
	TGroupBox *GroupBox1;
	TCheckBox *chkRetry;
	TCheckBox *chkAutosync;
	TCheckBox *chkExit;
	TTimer *tmrStartup;
	TLabel *Label3;
	TLabel *lblLocalTime;
	TLabel *lblServerTime;
	TTimer *tmrSyncReal;
	TActionManager *ActionManager1;
	TAction *ActionSync;
	TAction *ActionAbort;
	TAction *ActionGet;

	void __fastcall btnSyncClick(TObject *Sender);
	void __fastcall tmrUiTimer(TObject *Sender);
	void __fastcall tmrUiMsTimer(TObject *Sender);
	void __fastcall edtSyncedMouseLeave(TObject *Sender);
	void __fastcall edtSyncedMouseEnter(TObject *Sender);
	void __fastcall btnAbortClick(TObject *Sender);
	void __fastcall tmrSyncNextTimer(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall chkRetryClick(TObject *Sender);
	void __fastcall tmrStartupTimer(TObject *Sender);
	void __fastcall btnGetClick(TObject *Sender);
	void __fastcall tmrSyncRealTimer(TObject *Sender);
	void __fastcall ActionSyncExecute(TObject *Sender);
	void __fastcall ActionAbortExecute(TObject *Sender);
	void __fastcall ActionGetExecute(TObject *Sender);

private: // User declarations
	bool ResolveHost(String host, TStringList *iplist);
	void ScrollToTop(TMemo *memo);
	void UpdateUiClear();
	void UpdateUiTimezone();
	void UpdateUiDateTime();
	void UpdateUiSync(bool synced, __int64 elapsed);
	void UpdateUiSyncError(String title, String hint);
	void Sync(String server, bool onlyGet);
	void SyncReal(String server, bool onlyGet);
	void SyncDone();
	void SyncNext();
	void LoadConfig();
	void SaveConfig();
	void UpdateUiConfig(bool uiToConfig);
	void Exit();
	void ExitStop();
	void UpdateUiGet(bool success, __int64 elapsed);

	TBalloonHint *Hint1;
	bool syncNext;
	TStringList *failedList;
	bool Exiting;
	double timeOffset;
	bool timeValid;
	bool timeOnlyGet;
	SYNCREALPARAMS syncParams;

public: // User declarations
	__fastcall TForm1(TComponent* Owner);
};

// ---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
// ---------------------------------------------------------------------------
#endif
