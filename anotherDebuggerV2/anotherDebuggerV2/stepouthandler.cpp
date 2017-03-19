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
	bool AnotherDebugger::onStepOutBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo)
	{

		return true;
	}

	void AnotherDebugger::deleteStepOutBreakPoint()
	{

	}
}