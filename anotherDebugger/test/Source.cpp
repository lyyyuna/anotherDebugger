#include <iostream>
#include <vector>

using namespace::std;

int g_a;

struct S_A
{
	int a;
	double b;
};

enum E_A
{
	yui,
	bui
};
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
	S_A  sa;
	sa.a = 1;
	sa.b = 1.01;
	E_A ea = yui;

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

