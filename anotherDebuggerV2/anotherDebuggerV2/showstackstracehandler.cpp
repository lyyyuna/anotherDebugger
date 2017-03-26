#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <list>
#include "anotherdebugger.h"
#include <Windows.h>
#include <DbgHelp.h>

using namespace std;

namespace anotherdebugger
{
	BOOL CALLBACK EnumerateModuleCallBack(PCTSTR ModuleName, DWORD64 ModuleBase, ULONG ModuleSize, PVOID UserContext) {

		auto pModuleMap = (map<DWORD, string>*)UserContext;
		
		//LPCSTR name = csrchr(ModuleName, '\\') + 1;
		auto nameTmp = string(ModuleName);
		auto index = nameTmp.find_last_of('\\')+1;
		(*pModuleMap)[(DWORD)ModuleBase] = nameTmp.substr(index);

		return TRUE;
	}

	void AnotherDebugger::onShowStackTrace(const Command & cmds)
	{
		if (debuggeeStatus == DebuggeeStatus::NONE)
		{
			cout << "Debuggee not started." << endl;
			return;
		}

		if (cmds.size() != 1)
		{
			cout << "Invalid params" << endl;
			return;
		}

		if (EnumerateLoadedModules64(
			debuggeehProcess,
			EnumerateModuleCallBack,
			&moduleMap) == FALSE) 
		{
			cout << TEXT("EnumerateLoadedModules64 failed: ") << GetLastError() << endl;
			return;
		}

		CONTEXT c;
		getDebuggeeContext(&c);

		STACKFRAME64 stackFrame = { 0 };
		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrPC.Offset = c.Eip;
		stackFrame.AddrStack.Mode = AddrModeFlat;
		stackFrame.AddrStack.Offset = c.Esp;
		stackFrame.AddrFrame.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Offset = c.Ebp;


		auto printHex = [](DWORD value) -> void
		{
			cout << hex << uppercase;

			cout << " 0x" << setw(8) <<
				std::setfill('0') << value <<
				dec << nouppercase << flush;
		};

		while (true) 
		{
			if (StackWalk64(
				IMAGE_FILE_MACHINE_I386,
				debuggeehProcess,
				debuggeehThread,
				&stackFrame,
				&c,
				NULL,
				SymFunctionTableAccess64,
				SymGetModuleBase64,
				NULL) == FALSE) 
			{
				break;
			}

			printHex((DWORD)stackFrame.AddrPC.Offset);
			cout << "  ";

			DWORD moduleBase = (DWORD)SymGetModuleBase64(debuggeehProcess, stackFrame.AddrPC.Offset);

			auto moduleName = moduleMap[moduleBase];

			if (moduleName.length() != 0) 
			{
				cout << moduleName;
			}
			else {
				cout << "??";
			}

			cout << '!';

			BYTE buffer[sizeof(SYMBOL_INFO) + 128 * sizeof(TCHAR)] = { 0 };
			PSYMBOL_INFO pSymInfo = (PSYMBOL_INFO)buffer;
			pSymInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
			pSymInfo->MaxNameLen = 128;

			DWORD64 displacement;

			if (SymFromAddr(
				debuggeehProcess,
				stackFrame.AddrPC.Offset,
				&displacement,
				pSymInfo) == TRUE) 
			{
				std::wcout << pSymInfo->Name << std::endl;
			}
			else {
				std::wcout << "??" << std::endl;
			}
		}
	}
}