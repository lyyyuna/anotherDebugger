#include <iostream>
#include <string>
#include "anotherdebugger.h"

using namespace std;

namespace anotherdebugger
{
	void AnotherDebugger::onStartDebug(const Command & cmds)
	{
		if (cmds.size() < 2)
		{
			cout << "Lack path" << endl;
			return;
		}
		else if (cmds.size() > 2)
		{
			cout << "Invalid command" << endl;
			return;
		}

		startDebuggerSession(cmds[1].c_str());
	}
}