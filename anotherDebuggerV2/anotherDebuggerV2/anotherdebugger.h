#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <Windows.h>

using namespace std;

namespace anotherdebugger
{
	typedef vector<string> Command;
	
	class AnotherDebugger
	{
	public:
		bool DEBUG;

	public:
		// debugger loop
		AnotherDebugger(bool flag);
		void startDebuggerLoop();
		void parseCommand(string &);
		bool dispatchCommand();
		void cleanCommand() { cmds.clear(); }

		// debugger session
		void startDebuggerSession(LPCTSTR path);
		void continueDebuggerSession();
		bool dispatchDebugEvent(const DEBUG_EVENT &);
		void stopDebugSession();
		bool getDebuggeeContext(CONTEXT * pContext);

		// debugger event handler
		bool onProcessCreated(const CREATE_PROCESS_DEBUG_INFO*);
		bool onThreadCreated(const CREATE_THREAD_DEBUG_INFO*);
		bool onException(const EXCEPTION_DEBUG_INFO*);
		bool onProcessExited(const EXIT_PROCESS_DEBUG_INFO*);
		bool onThreadExited(const EXIT_THREAD_DEBUG_INFO*);
		bool onOutputDebugString(const OUTPUT_DEBUG_STRING_INFO*);
		bool onRipEvent(const RIP_INFO*);
		bool onDllLoaded(const LOAD_DLL_DEBUG_INFO*);
		bool onDllUnloaded(const UNLOAD_DLL_DEBUG_INFO*);

		// debugger command handler
		void ondummyhandler(const Command & cmds){}
		void onStartDebug(const Command & cmds);
		void onGo(const Command & cmds);
		void onDump(const Command & cmds);
		void onStopDebug(const Command & cmds);
		void onShowRegisters(const Command & cmds);
		void onShowSourceLines(const Command & cmds);

		// helper function
		void displayOneLine(LPCTSTR srcfile, string & line, int linenum, bool isCurline);
		void displaySourceLines(LPCTSTR srcfile, int linenum, DWORD64 addr, int start, int len);
		void displayFromCurLine(LPCTSTR srcfile, int linenum, DWORD64 addr, int len);

		// debugger status
		enum class DebuggeeStatus
		{
			NONE,
			SUSPENDED,
			INTERRUPTED
		};
		
		typedef void(AnotherDebugger::*cmdhandler)(const Command &);
		map<string, cmdhandler> cmdmap;
		Command cmds;
		DebuggeeStatus debuggeeStatus;
		DWORD continueStatus;
		HANDLE debuggeehProcess;
		HANDLE debuggeehThread;
		DWORD debuggeeprocessID;
		DWORD debuggeethreadID;
	};
}