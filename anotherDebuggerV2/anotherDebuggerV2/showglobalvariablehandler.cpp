#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <list>
#include "anotherdebugger.h"
#include <Windows.h>
#include <DbgHelp.h>
#include "SymbolType.h"

using namespace std;

namespace anotherdebugger
{
	extern BOOL CALLBACK EnumVariablesCallBack(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext);

	void AnotherDebugger::onShowGlobalVariables(const Command & cmds)
	{
		if (debuggeeStatus == DebuggeeStatus::NONE)
		{
			cout << "Debuggee not started." << endl;
			return;
		}

		if (cmds.size() > 2)
		{
			cout << "Invalid params" << endl;
			return;
		}

		CONTEXT c;
		getDebuggeeContext(&c);
		DWORD modBase = (DWORD)SymGetModuleBase64(debuggeehProcess, c.Eip);

		if (modBase == 0) 
		{
			cout << "SymGetModuleBase64 failed: " 
				<< GetLastError() << std::endl;
			return;
		}

		list<VariableInfo> varInfos;

		if (cmds.size() == 1)
		{
			if (SymEnumSymbols(
				debuggeehProcess,
				modBase,
				NULL,
				EnumVariablesCallBack,
				&varInfos) == FALSE)
			{

				std::cout << "SymEnumSymbols failed: " << GetLastError() << std::endl;
			}
			showVariables(varInfos);
		}
		else
		{
			list<VariableInfo> varInfos;
			LPCSTR expression = NULL;
			expression = cmds[1].c_str();

			if (SymEnumSymbols(
				debuggeehProcess,
				modBase,
				expression,
				EnumVariablesCallBack,
				&varInfos
				) == FALSE)
			{
				cout << "SymEnumSymbols failed: " << GetLastError() << endl;
			}

			showVariable(varInfos);
		}

		
	}
}