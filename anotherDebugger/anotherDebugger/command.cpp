#include "main.h"

void parseCommand(wstring& cmdline, Command& cmd)
{
	wistringstream cmdStrStream(cmdline);

	wstring arg;
	while (cmdStrStream >> arg)
	{
		cmd.push_back(arg);
	}
}