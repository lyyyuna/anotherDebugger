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
	void AnotherDebugger::onStepOver(const Command & cmds)
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

		saveCurrentLineInfo();

		FLAG.isBeingStepOver = true;

		CONTEXT c;
		getDebuggeeContext(&c);
		int pass = isCallInstruction(c.Eip);

		if (pass != 0)
		{
			setStepOverBreakPointAt(c.Eip + pass);
			FLAG.isBeingSingleInstruction = false;
		}
		else {
			setCPUTrapFlag();
			FLAG.isBeingSingleInstruction = true;
		}

		continueDebuggerSession();
	}

	void AnotherDebugger::setStepOverBreakPointAt(DWORD addr)
	{
		bpStepOver.address = addr;
		bpStepOver.content = setBreakPointAt(addr);
	}

	void AnotherDebugger::deleteStepOverBreakPoint()
	{
		if (bpStepOver.address != 0)
		{
			recoverBreakPoint(bpStepOver);
			bpStepOver.address = 0;
			bpStepOver.content = 0;
		}
	}
}