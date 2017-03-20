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
	void AnotherDebugger::onStepOut(const Command & cmds)
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

		FLAG.isBeingStepOut = true;
		FLAG.isBeingSingleInstruction = false;

		DWORD retAddr = getRetInstructionAddr();

		if (retAddr != 0)
		{
			setStepOutBreakPointAt(retAddr);
		}

		continueDebuggerSession();
	}

	bool AnotherDebugger::onStepOutBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo)
	{
		deleteStepOutBreakPoint();

		backwardDebuggeeEIP();

		CONTEXT c;
		getDebuggeeContext(&c);
		
		if (retInstructionLen(c.Eip) != 0)
		{
			DWORD retAddr;
			readDebuggeeMemory(c.Esp, sizeof(DWORD), &retAddr);

			setStepOutBreakPointAt(retAddr);

			FLAG.continueStatus = DBG_CONTINUE;
			return true;
		}

		FLAG.isBeingStepOut = false;

		debuggeeStatus = DebuggeeStatus::INTERRUPTED;
		return false;
	}

	void AnotherDebugger::setStepOutBreakPointAt(DWORD addr)
	{
		bpStepOut.address = addr;
		bpStepOut.content = setBreakPointAt(addr);
	}

	void AnotherDebugger::deleteStepOutBreakPoint()
	{
		if (bpStepOut.address != 0)
		{
			recoverBreakPoint(bpStepOut);

			bpStepOut.address = 0;
			bpStepOut.content = 0;
		}
	}

	DWORD AnotherDebugger::getRetInstructionAddr()
	{
		CONTEXT c;
		getDebuggeeContext(&c);

		DWORD64 displacement;
		SYMBOL_INFO symBol = { 0 };
		symBol.SizeOfStruct = sizeof(SYMBOL_INFO);

		if (SymFromAddr(
			debuggeehProcess,
			c.Eip,
			&displacement,
			&symBol) == FALSE) 
		{
			return 0;
		}

		DWORD funcEndAddr = (DWORD)(symBol.Address + symBol.Size);
		auto retLen = retInstructionLen(funcEndAddr - 3);
		if (retLen == 3)
		{
			return funcEndAddr - 3;
		}

		retLen = retInstructionLen(funcEndAddr - 1);
		if (retLen == 1)
		{
			return funcEndAddr - 1;
		}

		return 0;
	}

	DWORD AnotherDebugger::retInstructionLen(DWORD addr)
	{
		BYTE byte;
		this->readDebuggeeMemory(addr, 1, &byte);

		if (byte == 0xC3 || byte == 0xCB)
		{
			return 1;
		}

		if (byte == 0xC2 || byte == 0xCA)
		{
			return 3;
		}
		return 0;
	}
}