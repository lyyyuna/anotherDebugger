#include "main.h"

void OnShowSourceLines(const Command & cmd)
{

	if (getDebuggeeStatus() == DebuggeeStatus::NONE)
	{
		cout << "Debuggee not started." << endl;
		return;
	}

	if (cmd.size() == 3)
	{
		stringstream ss(cmd[1]);
		int start;
		ss >> start;
		if (start < 0)
		{
			cout << "Invalid params" << endl;
			return;
		}

		stringstream ss1(cmd[2]);
		int len;
		ss1 >> len;
		if (len < 0)
		{
			cout << "Invalid params" << endl;
			return;
		}
	}
	else
	{
		cout << "Invalid params" << endl;
		return;
	}
	
	CONTEXT context;
	GetDebuggeeContext(&context);

	IMAGEHLP_LINE64 lineInfo = { 0 };

	lineInfo.SizeOfStruct = sizeof(lineInfo);
	DWORD displacement = 0;

	if (SymGetLineFromAddr64(
		GetDebuggeeHandle(),
		context.Eip,
		&displacement,
		&lineInfo
		) == FALSE)
	{
		DWORD err = GetLastError();
		cout << err << endl;
		switch (err)
		{
		case 126:
			cout << "Debug info in current module has not loaded" << endl;
			return;
		case 487:
			cout << "No debug info in current module." << endl;
			return;
		default:
			cout << "SymGetLineFromAddr64 failed: " << err << endl;
			return;
		}
	}
}