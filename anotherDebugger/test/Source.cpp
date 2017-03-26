#include <iostream>
#include <vector>

using namespace::std;

int g_a;

int func()
{
	int a;
	a = 1 + 2;
	a = a * a;
	cout << "Another hello." << endl;

	return a;
}

int main()
{
	for (int i = 0; i < 2; i++)
	{
		int a;
		g_a++;
		a = 1 + 2;
		func();
		cout << "hello world." << endl;
	}
	return 0;
}

