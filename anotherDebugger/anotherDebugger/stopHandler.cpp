#include "main.h"

void OnStopDebug(const Command& cmd)
{
	if (getDebuggeeStatus() == DebuggeeStatus::NONE)
	{
		cout << "Debuggee not started." << endl;
		return;
	}
	else
	{
		StopDebugSeesion();
	}
}