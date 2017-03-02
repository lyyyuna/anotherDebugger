#include "main.h"

static HANDLE g_hProcess = NULL;
static HANDLE g_hThread = NULL;
static DWORD g_processID = 0;
static DWORD g_threadID = 0;

static DWORD g_continueStatus = DBG_EXCEPTION_NOT_HANDLED;
static auto g_debuggeeStatus = DebuggeeStatus::NONE;


DebuggeeStatus getDebuggeeStatus()
{
	return g_debuggeeStatus;
}

HANDLE GetDebuggeeHandle() 
{
	return g_hProcess;
}


void startDebuggerSession(LPCTSTR path)
{
	if (g_debuggeeStatus != DebuggeeStatus::NONE)
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

	g_hProcess = processinfo.hProcess;
	g_hThread = processinfo.hThread;
	g_processID = processinfo.dwProcessId;
	g_threadID = processinfo.dwThreadId;

	g_debuggeeStatus = DebuggeeStatus::SUSPENDED;

	cout << "Debuggee has started and was suspended." << endl;

}

void ContinueDebugerSession()
{
	if (g_debuggeeStatus == DebuggeeStatus::NONE)
	{
		cout << "Debuggee is not started yet." << endl;
		return;
	}
	if (g_debuggeeStatus == DebuggeeStatus::SUSPENDED)
	{
		cout << "Continue to run." << endl;
		ResumeThread(g_hThread);
	}
	else {
		ContinueDebugEvent(g_processID, g_threadID, g_continueStatus);
	}

	DEBUG_EVENT debugEvent;
	while (WaitForDebugEvent(&debugEvent, INFINITE) == TRUE)
	{
		g_processID = debugEvent.dwProcessId;
		g_threadID = debugEvent.dwThreadId;
		if (dispatchDebugEvent(debugEvent) == TRUE)
		{
			ContinueDebugEvent(g_processID, g_threadID, DBG_EXCEPTION_NOT_HANDLED);
		}
		else {
			break;
		}
	}
}

BOOL dispatchDebugEvent(const DEBUG_EVENT & debugEvent)
{
	switch (debugEvent.dwDebugEventCode)
	{
	case CREATE_PROCESS_DEBUG_EVENT:
		return OnProcessCreated(&debugEvent.u.CreateProcessInfo);
		break;

	case CREATE_THREAD_DEBUG_EVENT:
		return OnThreadCreated(&debugEvent.u.CreateThread);
		break;

	case EXCEPTION_DEBUG_EVENT:
		return OnException(&debugEvent.u.Exception);
		break;

	case EXIT_PROCESS_DEBUG_EVENT:
		return OnProcessExited(&debugEvent.u.ExitProcess);
		break;

	case EXIT_THREAD_DEBUG_EVENT:
		return OnThreadExited(&debugEvent.u.ExitThread);
		break;

	case LOAD_DLL_DEBUG_EVENT:
		return OnDllLoaded(&debugEvent.u.LoadDll);
		break;

	case UNLOAD_DLL_DEBUG_EVENT:
		return OnDllUnloaded(&debugEvent.u.UnloadDll);
		break;

	case OUTPUT_DEBUG_STRING_EVENT:
		return OnOutputDebugString(&debugEvent.u.DebugString);
		break;

	case RIP_EVENT:
		return OnRipEvent(&debugEvent.u.RipInfo);
		break;

	default:
		std::cout << "Unknown debug event." << std::endl;
		return FALSE;
		break;
	}
}



BOOL OnProcessCreated(const CREATE_PROCESS_DEBUG_INFO* pInfo)
{
	cout << "Debuggee was created." << endl;

	// init symbol system
	if (SymInitialize(g_hProcess, NULL, FALSE) == TRUE)
	{
		DWORD64 moduleAddress = SymLoadModule64(
			g_hProcess,
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
	return TRUE;
}



BOOL OnThreadCreated(const CREATE_THREAD_DEBUG_INFO* pInfo)
{
	//CloseHandle(pInfo->hThread);
	std::cout << "A new thread was created." << std::endl;
	return TRUE;
}


void HandledException(BOOL handled) 
{
	g_continueStatus = (handled == TRUE) ? DBG_CONTINUE : DBG_EXCEPTION_NOT_HANDLED;
}

BOOL OnException(const EXCEPTION_DEBUG_INFO* pInfo)
{
	std::cout << "An exception was occured." << std::endl;
	std::cout << std::hex << std::uppercase << std::setw(8) << std::setfill('0')
		<< pInfo->ExceptionRecord.ExceptionAddress << "." << std::endl
		<< "Exception code: " << pInfo->ExceptionRecord.ExceptionCode << std::dec << std::endl;

	if (pInfo->dwFirstChance == TRUE)
	{
		std::cout << "First chance." << std::endl;
		//g_continueStatus = DBG_EXCEPTION_NOT_HANDLED;
	}
	else
	{
		std::cout << "Second chance." << std::endl;
		//g_continueStatus = DBG_CONTINUE;
	}

	g_debuggeeStatus = DebuggeeStatus::INTERRUPTED;
	return FALSE;
}



BOOL OnProcessExited(const EXIT_PROCESS_DEBUG_INFO* pInfo)
{
	std::cout << "Debuggee was terminated." << std::endl;
	cout << "The exit code: " << pInfo->dwExitCode << endl;

	// clean up symbol tree
	SymCleanup(g_hProcess);

	ContinueDebugEvent(g_processID, g_threadID, DBG_CONTINUE);

	CloseHandle(g_hThread);
	CloseHandle(g_hProcess);

	g_hProcess = NULL;
	g_hThread = NULL;
	g_processID = 0;
	g_threadID = 0;
	g_debuggeeStatus = DebuggeeStatus::NONE;
	g_continueStatus = DBG_EXCEPTION_NOT_HANDLED;

	return FALSE;
}



BOOL OnThreadExited(const EXIT_THREAD_DEBUG_INFO* pInfo)
{
	std::cout << "A thread was terminated." << std::endl;
	return TRUE;
}



BOOL OnOutputDebugString(const OUTPUT_DEBUG_STRING_INFO* pInfo)
{
	// std::cout << TEXT("Debuggee outputed debug string.") << std::endl;

	BYTE* pBuffer = (BYTE*)malloc(pInfo->nDebugStringLength);
	SIZE_T bytesread;

	ReadProcessMemory(
		g_hProcess,
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

	g_debuggeeStatus = DebuggeeStatus::INTERRUPTED;
	return FALSE;
}



BOOL OnRipEvent(const RIP_INFO* pInfo)
{
	std::cout << "A RIP_EVENT occured." << std::endl;
	g_debuggeeStatus = DebuggeeStatus::INTERRUPTED;
	return FALSE;
}



BOOL OnDllLoaded(const LOAD_DLL_DEBUG_INFO* pInfo)
{
	std::cout << "A dll was loaded." << std::endl;

	// load symbol for this dll
	DWORD64 moduleAddress = SymLoadModule64(
		g_hProcess,
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



BOOL OnDllUnloaded(const UNLOAD_DLL_DEBUG_INFO* pInfo)
{
	std::cout << "A dll was unloaded." << std::endl;

	SymUnloadModule64(g_hProcess, (DWORD64)pInfo->lpBaseOfDll);
	return TRUE;
}


BOOL GetDebuggeeContext(CONTEXT * pContext)
{
	pContext->ContextFlags = CONTEXT_FULL;

	if (GetThreadContext(g_hThread, pContext) == FALSE)
	{
		cout << "Get thread context failed: " << endl;
		return FALSE;
	}

	return TRUE;
}

BOOL ReadDebuggeeMemory(unsigned int address, unsigned int length, void* pData) 
{
	SIZE_T bytesRead;

	return ReadProcessMemory(g_hProcess, (LPCVOID)address, pData, length, &bytesRead);
}

void StopDebugSeesion() 
{
	if (TerminateProcess(g_hProcess, -1) == TRUE) 
	{
		ContinueDebugerSession();
	}
	else {

		cout << "TerminateProcess failed: " << GetLastError() << endl;
	}
}
