#include "main.h"

void OnStartDebug(const Command& cmd)
{
	if (cmd.size() != 2)
	{
		cout << "Lack path" << endl;
		return;
	}

	startDebuggerSession(cmd[1].c_str());
}