#include <iostream>
#include "anotherdebugger.h"

using namespace std;

namespace anotherdebugger
{
	void AnotherDebugger::onGo(const Command & cmds)
	{
		if (debuggeeStatus == AnotherDebugger::DebuggeeStatus::NONE)
		{
			cout << "Debuggee not started." << endl;
			return;
		}

		FLAG.isBeingSingleInstruction = false;

		if (cmds.size() < 2)
		{
			//HandledException(FALSE);
			continueDebuggerSession();
			return;
		}

		if (cmds[1] == "c")
		{
			//HandledException(TRUE);
			continueDebuggerSession();
			return;
		}
	}
}
