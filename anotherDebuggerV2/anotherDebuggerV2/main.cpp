#include <iostream>
#include "anotherdebugger.h"

using namespace std;
using namespace anotherdebugger;

int main()
{
	auto lyydebugger = AnotherDebugger(true);
	setDebuggerForCallback(lyydebugger);
	lyydebugger.startDebuggerLoop();

	return 0;
}