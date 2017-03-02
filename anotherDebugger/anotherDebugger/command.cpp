#include "main.h"

void parseCommand(string& cmdline, Command& cmd)
{
	istringstream cmdStrStream(cmdline);

	string arg;
	while (cmdStrStream >> arg)
	{
		cmd.push_back(arg);
	}
}

map<string, cmdHandler> cmdMap = {
	{ "s", OnStartDebug },
	{ "g", OnGo },
	{ "d", OnDump },
	{ "r", OnShowRegisters },
	{ "t", OnStopDebug },
	{ "l", OnShowSourceLines }
};

BOOL dispatchCommand(const Command& cmd)
{
	if (cmd.size() == 0)
	{
		cout << "Invalid command." << endl;
		return TRUE;
	}

	if ("q" == cmd[0])
	{
		return FALSE;
	}

	auto cmdIter = cmdMap.find(cmd[0]);
	if (cmdIter == cmdMap.end())
	{
		cout << "Invalid command." << endl;
		return TRUE;
	}
	else {
		cmdIter->second(cmd);
		return TRUE;
	}
}

