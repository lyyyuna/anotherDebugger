#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <list>
#include "anotherdebugger.h"
#include <Windows.h>
#include <DbgHelp.h>

using namespace std;

namespace anotherdebugger
{
	void AnotherDebugger::setStepOverBreakPointAt(DWORD addr)
	{
		bpStepOver.address = addr;
		bpStepOver.content = setBreakPointAt(addr);
	}

	void AnotherDebugger::deleteStepOverBreakPoint()
	{

	}
}