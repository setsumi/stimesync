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

	void __fastcall btnSyncClick(TObject *Sender);
	void __fastcall tmrUiTimer(TObject *Sender);
	void __fastcall tmrUiMsTimer(TObject *Sender);
	void __fastcall edtSyncedMouseLeave(TObject *Sender);
	void __fastcall edtSyncedMouseEnter(TObject *Sender);
	void __fastcall btnAbortClick(TObject *Sender);
	void __fastcall tmrSyncNextTimer(TObject *Sender);
	void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall chkRetryClick(TObject *Sender);

private: // User declarations
	bool ResolveHost(String host, TStringList *iplist);
	void ScrollToTop(TMemo *memo);
	void UpdateUiClear();
	void UpdateUiTimezone();
	void UpdateUiDateTime();
	void UpdateUiSync(bool synced, __int64 elapsed);
	void UpdateUiSyncError(String title, String hint);
	void Sync(String server);
	void SyncDone();
	void SyncNext();
	void LoadConfig();
	void SaveConfig();
	void UpdateUiConfig(bool uiToConfig);

	TBalloonHint *Hint1;
	bool syncNext;
    TStringList *failedList;
public: // User declarations
	__fastcall TForm1(TComponent* Owner);
};

// ---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
// ---------------------------------------------------------------------------
#endif
