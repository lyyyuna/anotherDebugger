#include "main.h"

void OnGo(const Command& cmd)
{
	if (getDebuggeeStatus() == DebuggeeStatus::NONE)
	{
		cout << "Debuggee not started." << endl;
		return;
	}

	if (cmd.size() < 2)
	{
		//HandledException(FALSE);
		ContinueDebugerSession();
		return;
	}

	if (cmd[1] == "c")
	{
		//HandledException(TRUE);
		ContinueDebugerSession();
		return;
	}
}