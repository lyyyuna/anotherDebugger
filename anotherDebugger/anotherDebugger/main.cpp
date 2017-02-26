#include "main.h"

int main()
{
	wcout << "anotherDebugger by lyyyuna" << endl;

	string cmdline;
	Command cmd;
	while (true)
	{
		cout << endl << "> ";
		getline(cin, cmdline);
		
		parseCommand(cmdline, cmd);
		if (FALSE == dispatchCommand(cmd))
		{
			break;
		}

		cmd.clear();
	}
	return 0;
}