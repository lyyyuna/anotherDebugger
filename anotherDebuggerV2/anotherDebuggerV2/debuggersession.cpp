#include <iostream>
#include <iomanip>
#include <string>
#include "anotherdebugger.h"
#include <Windows.h>
#include <DbgHelp.h>

namespace anotherdebugger
{
	void AnotherDebugger::startDebuggerSession(LPCTSTR path)
	{
		if (debuggeeStatus != DebuggeeStatus::NONE)
		{
			cout << "Debuggee is already running." << endl;
			return;
		}

		STARTUPINFO startupinfo = { 0 };
		startupinfo.cb = sizeof(startupinfo);
		PROCESS_INFORMATION processinfo = { 0 };
		unsigned int creationflags = DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE | CREATE_SUSPENDED;

		if (CreateProcess(
			"L:\\git_up\\anotherDebugger\\anotherDebugger\\Debug\\test.exe",
			//path,
			NULL,
			NULL,
			NULL,
			FALSE,
			creationflags,
			NULL,
			NULL,
			&startupinfo,
			&processinfo) == FALSE)
		{
			std::cout << "CreateProcess failed: " << GetLastError() << std::endl;
			return;
		}

		debuggeehProcess = processinfo.hProcess;
		debuggeehThread = processinfo.hThread;
		debuggeeprocessID = processinfo.dwProcessId;
		debuggeethreadID = processinfo.dwThreadId;

		debuggeeStatus = DebuggeeStatus::SUSPENDED;

		cout << "Debuggee has started and is suspended." << endl;
	}

	void AnotherDebugger::continueDebuggerSession()
	{
		if (debuggeeStatus == DebuggeeStatus::NONE)
		{
			cout << "Debuggee is not started yet." << endl;
			return;
		}
		if (debuggeeStatus == DebuggeeStatus::SUSPENDED)
		{
			cout << "Continue to run." << endl;
			ResumeThread(debuggeehThread);
		}
		else
		{
			ContinueDebugEvent(debuggeeprocessID, debuggeethreadID, Flag::continueStatus);
		}

		DEBUG_EVENT debugEvent;
		while (WaitForDebugEvent(&debugEvent, INFINITE) == TRUE)
		{
			debuggeeprocessID = debugEvent.dwProcessId;
			debuggeethreadID = debugEvent.dwThreadId;
			if (dispatchDebugEvent(debugEvent) == TRUE)
			{
				ContinueDebugEvent(debuggeeprocessID, debuggeethreadID, Flag::continueStatus);
			}
			else {
				break;
			}
		}

	}

	bool AnotherDebugger::dispatchDebugEvent(const DEBUG_EVENT & debugEvent)
	{
		switch (debugEvent.dwDebugEventCode)
		{
		case CREATE_PROCESS_DEBUG_EVENT:
			return onProcessCreated(&debugEvent.u.CreateProcessInfo);
			break;

		case CREATE_THREAD_DEBUG_EVENT:
			return onThreadCreated(&debugEvent.u.CreateThread);
			break;

		case EXCEPTION_DEBUG_EVENT:
			return onException(&debugEvent.u.Exception);
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			return onProcessExited(&debugEvent.u.ExitProcess);
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			return onThreadExited(&debugEvent.u.ExitThread);
			break;

		case LOAD_DLL_DEBUG_EVENT:
			return onDllLoaded(&debugEvent.u.LoadDll);
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			return onDllUnloaded(&debugEvent.u.UnloadDll);
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			return onOutputDebugString(&debugEvent.u.DebugString);
			break;

		case RIP_EVENT:
			return onRipEvent(&debugEvent.u.RipInfo);
			break;

		default:
			cout << "Unknown debug event." << endl;
			return false;
			break;
		}
	}

	bool AnotherDebugger::onProcessCreated(const CREATE_PROCESS_DEBUG_INFO * pInfo)
	{
		cout << "Debuggee created." << endl;

		this->resetBreakPoint();

		if (SymInitialize(debuggeehProcess, NULL, FALSE) == TRUE)
		{
			DWORD64 moduleAddress = SymLoadModule64(
				debuggeehProcess,
				pInfo->hFile,
				NULL,
				NULL,
				(DWORD64)pInfo->lpBaseOfImage,
				0
				);
			if (moduleAddress == 0)
			{
				cout << "SymLoadModule64 failed: " << GetLastError() << endl;
			}
		}
		else
		{
			cout << "SymInitialize failed: " << GetLastError() << endl;
		}
		return true;
	}

	bool AnotherDebugger::onThreadCreated(const CREATE_THREAD_DEBUG_INFO * pInfo)
	{
		cout << "A new thread was created." << endl;
		return true;
	}

	bool AnotherDebugger::onException(const EXCEPTION_DEBUG_INFO * pInfo)
	{
		if (DEBUG == true)
		{
			std::cout << "An exception was occured." << std::endl;
			std::cout << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
				<< pInfo->ExceptionRecord.ExceptionAddress << "." << std::endl
				<< "Exception code: " << pInfo->ExceptionRecord.ExceptionCode << std::dec << std::endl;
		}

		switch (pInfo->ExceptionRecord.ExceptionCode)
		{
		case EXCEPTION_BREAKPOINT:
			return onBreakPoint(pInfo);
		case EXCEPTION_SINGLE_STEP:
			return onSingleStepTrap(pInfo);
		}

		if (pInfo->dwFirstChance == TRUE)
		{
			if (DEBUG == true)
			{
				std::cout << "First chance." << std::endl;
			}
		}
		else
		{
			if (DEBUG == true)
			{
				std::cout << "Second chance." << std::endl;
			}
		}

		debuggeeStatus = DebuggeeStatus::INTERRUPTED;
		return false;
	}

	bool AnotherDebugger::onProcessExited(const EXIT_PROCESS_DEBUG_INFO * pInfo)
	{
		std::cout << "Debuggee was terminated." << std::endl;
		cout << "The exit code: " << pInfo->dwExitCode << endl;

		// clean up symbol tree
		SymCleanup(debuggeehProcess);

		ContinueDebugEvent(debuggeeprocessID, debuggeethreadID, DBG_CONTINUE);

		CloseHandle(debuggeehThread);
		CloseHandle(debuggeehProcess);

		debuggeehProcess = NULL;
		debuggeehThread = NULL;
		debuggeeprocessID = 0;
		debuggeethreadID = 0;
		debuggeeStatus = DebuggeeStatus::NONE;
		continueStatus = DBG_EXCEPTION_NOT_HANDLED;

		return false;
	}

	bool AnotherDebugger::onThreadExited(const EXIT_THREAD_DEBUG_INFO * pInfo)
	{
		cout << "A thread was terminated." << endl;
		return true;
	}

	bool AnotherDebugger::onOutputDebugString(const OUTPUT_DEBUG_STRING_INFO * pInfo)
	{
		BYTE* pBuffer = (BYTE*)malloc(pInfo->nDebugStringLength);
		SIZE_T bytesread;

		ReadProcessMemory(
			debuggeehProcess,
			pInfo->lpDebugStringData,
			pBuffer,
			pInfo->nDebugStringLength,
			&bytesread);

		int requireLen = MultiByteToWideChar(
			CP_ACP,
			MB_PRECOMPOSED,
			(LPCSTR)pBuffer,
			pInfo->nDebugStringLength,
			NULL,
			0);
		WCHAR* pWideStr = (WCHAR*)malloc(requireLen * sizeof(WCHAR));
		MultiByteToWideChar(
			CP_ACP,
			MB_PRECOMPOSED,
			(LPCSTR)pBuffer,
			pInfo->nDebugStringLength,
			pWideStr,
			requireLen);

		std::cout << "Debuggee debug string is: " << pWideStr << std::endl;
		free(pWideStr);
		free(pBuffer);

		debuggeeStatus = DebuggeeStatus::INTERRUPTED;
		return false;
	}

	bool AnotherDebugger::onRipEvent(const RIP_INFO * pInfo)
	{
		std::cout << "A RIP_EVENT occured." << std::endl;
		debuggeeStatus = DebuggeeStatus::INTERRUPTED;
		return false;
	}

	bool AnotherDebugger::onDllLoaded(const LOAD_DLL_DEBUG_INFO* pInfo)
	{
		std::cout << "A dll was loaded." << std::endl;

		// load symbol for this dll
		DWORD64 moduleAddress = SymLoadModule64(
			debuggeehProcess,
			pInfo->hFile,
			NULL,
			NULL,
			(DWORD64)pInfo->lpBaseOfDll,
			0);

		if (moduleAddress == 0)
		{
			cout << "SymLoadModule64 failed: " << GetLastError() << endl;
		}
		return TRUE;
	}

	bool AnotherDebugger::onDllUnloaded(const UNLOAD_DLL_DEBUG_INFO* pInfo)
	{
		std::cout << "A dll was unloaded." << std::endl;

		SymUnloadModule64(debuggeehProcess, (DWORD64)pInfo->lpBaseOfDll);
		return TRUE;
	}

	void AnotherDebugger::stopDebugSession()
	{
		if (TerminateProcess(debuggeehProcess, -1) == TRUE)
		{
			continueDebuggerSession();
		}
		else {

			cout << "TerminateProcess failed: " << GetLastError() << endl;
		}
	}

	bool AnotherDebugger::getDebuggeeContext(CONTEXT * pContext)
	{
		pContext->ContextFlags = CONTEXT_FULL;

		if (GetThreadContext(this->debuggeehThread, pContext) == FALSE)
		{
			cout << "Get thread context failed: " << endl;
			return false;
		}
		return true;
	}
}