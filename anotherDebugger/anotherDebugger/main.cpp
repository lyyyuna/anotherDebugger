#include "main.h"

int main()
{
	wcout << TEXT("anotherDebugger by lyyyuna") << endl;

	wstring cmdline;
	Command cmd;
	while (true)
	{
		wcout << endl << TEXT(">");
		getline(wcin, cmdline);
		
		parseCommand(cmdline, cmd);
		//if (false == dispatchCommand(cmd))
		//{
		//	break;
		//}
	}
	return 0;
}