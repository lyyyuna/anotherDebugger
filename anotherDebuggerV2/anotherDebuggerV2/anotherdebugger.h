#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <list>
#include <Windows.h>

using namespace std;

namespace anotherdebugger
{
	typedef vector<string> Command;
	
	class AnotherDebugger
	{
	public:
		bool DEBUG;

		// debugger status
		enum class DebuggeeStatus
		{
			NONE,
			SUSPENDED,
			INTERRUPTED
		};

		// breakpoint type
		enum class BpType
		{
			INIT,
			STEP_OVER,
			STEP_OUT,
			USER,
			CODE
		};

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

		// breakpoint
		void resetBreakPoint();
		BpType getBreakPointType(DWORD);
		bool onBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo);
		bool onSingleStepTrap(const EXCEPTION_DEBUG_INFO * pInfo);
		bool onSingleStepCommonProcedures();  // common steps for singlesteptrap and stepOver breakpoint
		bool onNormalBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo);
		bool onUserBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo);
		bool onSingleStepCommonProcedures();
		bool onStepOutBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo);
		void backwardDebuggeeEIP();
		void deleteStepOverBreakPoint();


		struct BreakPoint
		{
			DWORD address;
			BYTE content;
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

		// breakpoint
		bool isInitBpSet;
		BreakPoint bpStepOver;
		BreakPoint bpStepOut;
		list<BreakPoint> bpUserList;

		// flag
		struct Flag
		{
			static DWORD continueStatus;
		};
	};
}