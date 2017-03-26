#include <iostream>
#include "anotherdebugger.h"

using namespace std;
using namespace anotherdebugger;

int main()
{
	auto lyydebugger = AnotherDebugger(false);
	g_ad = &lyydebugger;
	lyydebugger.startDebuggerLoop();

	return 0;
}