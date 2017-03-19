#include <iostream>
#include <vector>
#include <windows.h>
#include <winbase.h>
#include <dbghelp.h>

using namespace::std;

int main()
{
	for (int i = 0; i < 2; i++)
	{
		cout << "hello, world" << endl;
	}
	return 0;
}