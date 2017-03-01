#include "main.h"

void printHex(unsigned int value)
{
	cout << hex << uppercase;

	cout << "0x";

	cout << std::setw(8) << std::setfill('0') << value << std::dec << std::nouppercase << std::flush;
}

void OnShowRegisters(const Command& cmd)
{
	if (getDebuggeeStatus() == DebuggeeStatus::NONE)
	{
		cout << "Debuggee not started." << endl;
		return;
	}

	CONTEXT context;

	if (GetDebuggeeContext(&context) == FALSE)
	{
		cout << "Show register info failed." << endl;
		return;
	}

	cout << "EAX = ";
	printHex(context.Eax);
	cout << endl;

	cout << "EBX = ";
	printHex(context.Ebx);
	cout << endl;

	cout << "ECX = ";
	printHex(context.Ecx);
	cout << endl;

	cout << "EDX = ";
	printHex(context.Edx);
	cout << endl;

	cout << "ESI = ";
	printHex(context.Esi);
	cout << endl;

	cout << "EDI = ";
	printHex(context.Edi);
	cout << endl;

	cout << "EBP = ";
	printHex(context.Ebp);
	cout << endl;

	cout << "ESP = ";
	printHex(context.Esp);
	cout << endl;

	cout << "EIP = ";
	printHex(context.Eip);
	cout << endl;

	if ((context.EFlags & 0x1) != 0) 
	{
		std::cout << "CF  1" << endl;
	} 
	else {
		std::cout << "CF  0" << endl;
	}

	if ((context.EFlags & 0x4) != 0) 
	{
		std::cout << "PF  1" << endl;;
	}
	else {
		std::cout << "CF  0" << endl;
	}

	if ((context.EFlags & 0x10) != 0) 
	{
		std::cout << "AF  1" << endl;;
	}
	else {
		std::cout << "CF  0" << endl;
	}

	if ((context.EFlags & 0x40) != 0) 
	{
		std::cout << "ZF  1" << endl;;
	}
	else {
		std::cout << "CF  0" << endl;
	}

	if ((context.EFlags & 0x80) != 0) 
	{
		std::cout << "SF  1" << endl;;
	}
	else {
		std::cout << "CF  0" << endl;
	}

	if ((context.EFlags & 0x400) != 0) 
	{
		std::cout << "OF  1" << endl;;
	}
	else {
		std::cout << "CF  0" << endl;
	}

	if ((context.EFlags & 0x200) != 0) 
	{
		std::cout << "DF  1" << endl;;
	}
	else {
		std::cout << "CF  0" << endl;
	}
	std::cout << "CF  0" << endl;
}