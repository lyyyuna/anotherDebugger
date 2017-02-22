#include <Windows.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <malloc.h>
#include <vector>
#include <sstream>
#include <map>

using namespace std;

int startDebuggerSession(LPCTSTR path);

void OnProcessCreated(const CREATE_PROCESS_DEBUG_INFO*);
void OnThreadCreated(const CREATE_THREAD_DEBUG_INFO*);
void OnException(const EXCEPTION_DEBUG_INFO*);
void OnProcessExited(const EXIT_PROCESS_DEBUG_INFO*);
void OnThreadExited(const EXIT_THREAD_DEBUG_INFO*);
void OnOutputDebugString(const OUTPUT_DEBUG_STRING_INFO*);
void OnRipEvent(const RIP_INFO*);
void OnDllLoaded(const LOAD_DLL_DEBUG_INFO*);
void OnDllUnloaded(const UNLOAD_DLL_DEBUG_INFO*);

typedef vector<string> Command;

typedef void(*cmdHandler)(const Command&);

void parseCommand(string&, Command&);
bool dispatchCommand(Command&);
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