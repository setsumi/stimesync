#ifndef ToolsH
#define ToolsH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <IdComponent.hpp>
#include <list>

// ---------------------------------------------------------------------------
String tl_GetModuleName(); // .exe full path
String tl_GetProgramPath(); // including trailing '\'

typedef void(__closure * FTCdef)(const String);
void tl_RunInMainThread(FTCdef FuncToCall, const String fP1);

String BytesToHexStr(unsigned char *bytes, int count);
String BytesToHexStr(const TIdBytes &bytes);

String GetWindowClassPlus(HWND hwnd);
String GetWindowTitlePlus(HWND hwnd);
DWORD PSAPI_EnumProcesses(std::list<DWORD>& listProcessIDs, DWORD dwMaxProcessCount);

int t_DateTimeMs(const TDateTime &dtm);
String t_DoubleToStr(double dbl);
String t_MsDigit(const TDateTime &dt);
String t_MsFullDigits(const TDateTime &dt);
String t_FormatDateTimeMs(const TDateTime &dt);
String t_FormatTimeMs(const TDateTime &dt);

// ---------------------------------------------------------------------------
#endif
