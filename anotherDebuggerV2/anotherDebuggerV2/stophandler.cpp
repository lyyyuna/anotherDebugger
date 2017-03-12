#include <iostream>
#include <string>
#include "anotherdebugger.h"

using namespace std;

namespace anotherdebugger
{
	void AnotherDebugger::onStopDebug(const Command & cmds)
	{
		if (debuggeeStatus == AnotherDebugger::DebuggeeStatus::NONE)
		{
			cout << "Debuggee not started." << endl;
			return;
		}
		else
		{
			stopDebugSession();
		}
	}
}