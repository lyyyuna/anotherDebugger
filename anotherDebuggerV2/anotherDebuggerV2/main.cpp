#include <iostream>
#include "anotherdebugger.h"

using namespace std;

int main()
{
	auto lyydebugger = anotherdebugger::AnotherDebugger(true);

	lyydebugger.startDebuggerLoop();

	return 0;
}