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
	extern AnotherDebugger *g_ad;
	DWORD getSymbolAddr(PSYMBOL_INFO pSymbolInfo)
	{
		if ((pSymbolInfo->Flags & SYMFLAG_REGREL) == 0)
		{
			return DWORD(pSymbolInfo->Address);
		}

		CONTEXT c;
		g_ad->getDebuggeeContext(&c);

		DWORD64 displacement;
		SYMBOL_INFO symbolInfo = { 0 };
		symbolInfo.SizeOfStruct = sizeof(SYMBOL_INFO);

		SymFromAddr(
			g_ad->debuggeehProcess,
			c.Eip,
			&displacement,
			&symbolInfo);

		// if it is the first command of a function
		// then displacement == 0
		// 
		if (displacement == 0)
		{
			return (DWORD)(c.Esp - 4 + pSymbolInfo->Address);
		}
		else {
			return (DWORD)(c.Ebp + pSymbolInfo->Address);
		}

	}

	BOOL CALLBACK EnumVariablesCallBack(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
	{
		auto pVarInfos = static_cast<list<VariableInfo>*>(UserContext);

		VariableInfo varInfo;
		if (pSymInfo->Tag = SymTagData)
		{
			varInfo.address = getSymbolAddr(pSymInfo);
			varInfo.modBase = (DWORD)pSymInfo->ModBase;
			varInfo.size = SymbolSize;
			varInfo.typeID = pSymInfo->TypeIndex;
			varInfo.name = pSymInfo->Name;

			pVarInfos->push_back(varInfo);
		}

		return TRUE;
	}

	void AnotherDebugger::onShowLocalVariables(const Command & cmds)
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
		IMAGEHLP_STACK_FRAME stackFrame = { 0 };
		stackFrame.InstructionOffset = c.Eip;

		if (FALSE == SymSetContext(
			debuggeehProcess,
			&stackFrame,
			NULL))
		{
			if (GetLastError() != ERROR_SUCCESS)
			{
				cout << "No debug info in the current address." << endl;
				return;
			}
		}

		if (cmds.size() == 1)
		{
			list<VariableInfo> varInfos;

			if (SymEnumSymbols(
				debuggeehProcess,
				0,
				NULL,
				EnumVariablesCallBack,
				&varInfos
				) == FALSE)
			{
				cout << "SymEnumSymbols failed: " << GetLastError() << endl;
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
				0,
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

	void AnotherDebugger::showVariable(list<VariableInfo> & varInfos)
	{
		if (varInfos.begin() == varInfos.end())
		{
			cout << "This variable is not existed." << endl;
			return;
		}
		showVariableSummary(*varInfos.begin());
		cout << "\t";
		showVariableValue(*varInfos.begin());
		cout << endl;
	}

	void AnotherDebugger::showVariables(list<VariableInfo> & varInfos)
	{
		for (auto iter = varInfos.begin();
			iter != varInfos.end();
			iter++)
		{
			showVariableSummary(*iter);
			if (true == isPODType(iter->typeID, iter->modBase))
			{
				cout << "\t";
				showVariableValue(*iter);
			}

			cout << endl;
		}
	}

	void AnotherDebugger::showVariableSummary(const VariableInfo & varInfo)
	{
		cout << varInfo.name << "\t" <<
			getTypeName(varInfo.typeID, varInfo.modBase);
	}

	void AnotherDebugger::showVariableValue(const VariableInfo & varInfo)
	{
		BYTE * pData = (BYTE *)malloc(sizeof(BYTE) * varInfo.size);
		readDebuggeeMemory(varInfo.address, varInfo.size, pData);

		cout << getTypeValue(varInfo.typeID, varInfo.modBase, varInfo.address, pData);

		free(pData);
	}



}