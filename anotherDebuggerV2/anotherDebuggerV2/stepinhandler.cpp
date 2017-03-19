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
	void AnotherDebugger::onStepIn(const Command & cmds)
	{
		if (debuggeeStatus == DebuggeeStatus::NONE)
		{
			cout << "Debuggee not started." << endl;
			return;
		}

		if (cmds.size() != 1)
		{
			cout << "Invalid params." << endl;
		}

		saveCurrentLineInfo();
		setCPUTrapFlag();
		FLAG.isBeingSingleInstruction = true;

		continueDebuggerSession();
	}
}