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

		// breakpoint
		struct BreakPoint
		{
			DWORD address;
			BYTE content;
		};

		// 
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

		struct LineInfo{
			string filePath;
			DWORD lineNumber;
		};

		// flag
		struct Flag
		{
			DWORD continueStatus;
			DWORD resetUserBreakPointAddress;
			bool isBeingStepOver;
			bool isBeingStepOut;
			bool isBeingSingleInstruction;
			LineInfo glf;
		} FLAG;


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
		bool setDebuggeeContext(CONTEXT * pContext);
		void setDebuggeeEntryPoint();

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
		void onSetBreakPoint(const Command & cmds);
		void onStepIn(const Command & cmds);
		void onStepOver(const Command & cmds);
		void onStepOut(const Command & cmds);

		// helper function
		void displayOneLine(LPCTSTR srcfile, string & line, int linenum, bool isCurline);
		void displaySourceLines(LPCTSTR srcfile, int linenum, DWORD64 addr, int start, int len);
		void displayFromCurLine(LPCTSTR srcfile, int linenum, DWORD64 addr, int len);

		// breakpoint
		void resetBreakPointHandler();
		BpType getBreakPointType(DWORD);
		bool onBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo);
		bool onSingleStepTrap(const EXCEPTION_DEBUG_INFO * pInfo);
		bool onSingleStepCommonProcedures();  // common steps for singlesteptrap and stepOver breakpoint
		bool onNormalBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo);
		bool onUserBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo);
		bool onStepOutBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo);
		bool setUserBreakPointAt(DWORD);
		bool deleteUserBreakPointAt(DWORD);
		void setStepOverBreakPointAt(DWORD);
		void deleteStepOverBreakPoint();
		void setStepOutBreakPointAt(DWORD);
		void deleteStepOutBreakPoint();
		void recoverBreakPoint(const BreakPoint & bp);

		// breakpoint helper
		bool isLineChanged();
		bool getCurrentLineInfo(LineInfo &);
		int isCallInstruction(DWORD);
		void saveCurrentLineInfo();
		DWORD retInstructionLen(DWORD addr);
		DWORD getRetInstructionAddr();

		// inline breakpoint helper
		BOOL writeDebuggeeMemory(unsigned int address, unsigned int length, const void* pData) 
		{

			SIZE_T byteWriten;

			return WriteProcessMemory(debuggeehProcess, (LPVOID)address, pData, length, &byteWriten);
		}

		BOOL readDebuggeeMemory(unsigned int address, unsigned int length, void* pData) 
		{

			SIZE_T bytesRead;

			return ReadProcessMemory(debuggeehProcess, (LPCVOID)address, pData, length, &bytesRead);
		}

		void backwardDebuggeeEIP()
		{
			CONTEXT c;
			getDebuggeeContext(&c);
			c.Eip -= 1;
			setDebuggeeContext(&c);
		}

		void setCPUTrapFlag()
		{
			CONTEXT c;
			getDebuggeeContext(&c);
			c.EFlags |= 0x100;
			setDebuggeeContext(&c);
		}

		BYTE setBreakPointAt(DWORD addr)
		{
			BYTE byte;
			readDebuggeeMemory(addr, 1, &byte);

			BYTE intInst = 0xCC;
			writeDebuggeeMemory(addr, 1, &intInst);
			return byte;
		}

		void recoverBreakPoint(BreakPoint & bp)
		{
			writeDebuggeeMemory(bp.address, 1, &bp.content);
		}

	};
}