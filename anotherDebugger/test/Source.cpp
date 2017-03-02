#include <iostream>
#include <vector>
#include <windows.h>
#include <winbase.h>
#include <dbghelp.h>

using namespace::std;

int main()
{
	__asm { int 3};
	DWORD  error;
	HANDLE hProcess;
	
	std::vector<void*> stack(60);

	const int size = ::CaptureStackBackTrace(0, stack.size(), &(stack.front()), NULL);

	stack.resize(size);

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
	hProcess = GetCurrentProcess();
	if (SymInitialize(hProcess, NULL, TRUE))
	{
		IMAGEHLP_LINE64 lineInfo = { 0 };

		lineInfo.SizeOfStruct = sizeof(lineInfo);
		DWORD displacement = 0;
		if (SymGetLineFromAddr64(
			hProcess,
			(DWORD64)stack[0],
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
				return 0;
			case 487:
				cout << "No debug info in current module." << endl;
				return 0;
			default:
				cout << "SymGetLineFromAddr64 failed: " << err << endl;
				return 0;
			}
		}
	}
	else {
		cout << "error" << endl;
	}

	return 0;
}