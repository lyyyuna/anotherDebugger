#include <Windows.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <malloc.h>
#include <vector>
#include <sstream>
#include <map>

using namespace std;

void startDebuggerSession(LPCTSTR path);
void HandledException(BOOL handled);

BOOL OnProcessCreated(const CREATE_PROCESS_DEBUG_INFO*);
BOOL OnThreadCreated(const CREATE_THREAD_DEBUG_INFO*);
BOOL OnException(const EXCEPTION_DEBUG_INFO*);
BOOL OnProcessExited(const EXIT_PROCESS_DEBUG_INFO*);
BOOL OnThreadExited(const EXIT_THREAD_DEBUG_INFO*);
BOOL OnOutputDebugString(const OUTPUT_DEBUG_STRING_INFO*);
BOOL OnRipEvent(const RIP_INFO*);
BOOL OnDllLoaded(const LOAD_DLL_DEBUG_INFO*);
BOOL OnDllUnloaded(const UNLOAD_DLL_DEBUG_INFO*);

typedef vector<string> Command;

typedef void(*cmdHandler)(const Command&);

void parseCommand(string&, Command&);
BOOL dispatchCommand(const Command&);
void OnStartDebug(const Command& cmd);
void OnShowRegisters(const Command& cmd);
void OnStopDebug(const Command& cmd);
void OnGo(const Command& cmd);
void OnDump(const Command& cmd);


enum class DebuggeeStatus
{
	NONE,
	SUSPENDED,
	INTERRUPTED
};
BOOL dispatchDebugEvent(const DEBUG_EVENT& debugEvent);
void ContinueDebugerSession();
DebuggeeStatus getDebuggeeStatus();