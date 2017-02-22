#include "main.h"

static HANDLE g_hProcess = NULL;
static HANDLE g_hTread = NULL;
static DWORD g_processID = 0;
static DWORD g_threadID = 0;

static DWORD g_continueStatus = DBG_EXCEPTION_NOT_HANDLED;
static auto g_debuggeeStatus = DebuggeeStatus::NONE;


int startDebuggerSession(LPCTSTR path) {

	STARTUPINFO startupinfo = { 0 };
	startupinfo.cb = sizeof(startupinfo);
	PROCESS_INFORMATION processinfo = { 0 };
	unsigned int creationflags = DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE;

	if (CreateProcess(
		//TEXT("L:\\git_up\\anotherDebugger\\anotherDebugger\\Debug\\test.exe"),
		path,
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
		return -1;
	}

	g_hProcess = processinfo.hProcess;
	BOOL waitEvent = TRUE;

	DEBUG_EVENT debugEvent;
	while (waitEvent == TRUE && WaitForDebugEvent(&debugEvent, INFINITE))
	{
		switch (debugEvent.dwDebugEventCode)
		{
		case CREATE_PROCESS_DEBUG_EVENT:
			OnProcessCreated(&debugEvent.u.CreateProcessInfo);
			break;

		case CREATE_THREAD_DEBUG_EVENT:
			OnThreadCreated(&debugEvent.u.CreateThread);
			break;

		case EXCEPTION_DEBUG_EVENT:
			OnException(&debugEvent.u.Exception);
			break;

		case EXIT_PROCESS_DEBUG_EVENT:
			OnProcessExited(&debugEvent.u.ExitProcess);
			waitEvent = FALSE;
			break;

		case EXIT_THREAD_DEBUG_EVENT:
			OnThreadExited(&debugEvent.u.ExitThread);
			break;

		case LOAD_DLL_DEBUG_EVENT:
			OnDllLoaded(&debugEvent.u.LoadDll);
			break;

		case UNLOAD_DLL_DEBUG_EVENT:
			OnDllUnloaded(&debugEvent.u.UnloadDll);
			break;

		case OUTPUT_DEBUG_STRING_EVENT:
			OnOutputDebugString(&debugEvent.u.DebugString);
			break;

		case RIP_EVENT:
			OnRipEvent(&debugEvent.u.RipInfo);
			break;

		default:
			std::cout << "Unknown debug event." << std::endl;
			break;
		}

		if (waitEvent == TRUE)
		{
			ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, g_continueStatus);
		}
		else
		{
			break;
		}
	}

	CloseHandle(processinfo.hThread);
	CloseHandle(processinfo.hProcess);

	return 0;
}



void OnProcessCreated(const CREATE_PROCESS_DEBUG_INFO* pInfo)
{
	//CloseHandle(pInfo->hFile);
	//CloseHandle(pInfo->hProcess);
	//CloseHandle(pInfo->hThread);
	std::cout << "Debuggee was created." << std::endl;
}



void OnThreadCreated(const CREATE_THREAD_DEBUG_INFO* pInfo)
{
	//CloseHandle(pInfo->hThread);
	std::cout << "A new thread was created." << std::endl;
}



void OnException(const EXCEPTION_DEBUG_INFO* pInfo)
{
	std::cout << "An exception was occured." << std::endl;
	std::cout << std::hex << std::uppercase << std::setw(8) << std::setfill(L'0')
		<< pInfo->ExceptionRecord.ExceptionAddress << "." << std::endl
		<< "Exception code: " << pInfo->ExceptionRecord.ExceptionCode << std::dec << std::endl;

	if (pInfo->dwFirstChance == TRUE)
	{
		std::cout << "First chance." << std::endl;
		g_continueStatus = DBG_EXCEPTION_NOT_HANDLED;
	}
	else
	{
		std::cout << "Second chance." << std::endl;
		g_continueStatus = DBG_CONTINUE;
	}
}



void OnProcessExited(const EXIT_PROCESS_DEBUG_INFO* pInfo)
{
	std::cout << "Debuggee was terminated." << std::endl;
}



void OnThreadExited(const EXIT_THREAD_DEBUG_INFO* pInfo)
{
	std::cout << "A thread was terminated." << std::endl;
}



void OnOutputDebugString(const OUTPUT_DEBUG_STRING_INFO* pInfo)
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

	g_continueStatus = DBG_CONTINUE;
}



void OnRipEvent(const RIP_INFO* pInfo)
{
	std::cout << "A RIP_EVENT occured." << std::endl;
}



void OnDllLoaded(const LOAD_DLL_DEBUG_INFO* pInfo)
{
	std::cout << "A dll was loaded." << std::endl;
}



void OnDllUnloaded(const UNLOAD_DLL_DEBUG_INFO* pInfo)
{
	std::cout << "A dll was unloaded." << std::endl;
}