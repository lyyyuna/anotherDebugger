#include <iostream>
#include <iomanip>
#include <string>
#include "anotherdebugger.h"
#include <Windows.h>

using namespace std;

namespace anotherdebugger
{
	void AnotherDebugger::onShowRegisters(const Command & cmds)
	{
		if (cmds.size() != 1)
		{
			cout << "Invalid parameter." << endl;
			return;
		}

		if (debuggeeStatus == DebuggeeStatus::NONE)
		{
			cout << "Debuggee not started." << endl;
			return;
		}

		CONTEXT context;

		auto printHex = [](unsigned int value) -> void
		{
			cout << hex << uppercase;

			cout << "0x";

			cout << std::setw(8) << std::setfill('0') << value << std::dec << std::nouppercase << std::flush;
		};

		if (getDebuggeeContext(&context) == false)
		{
			cout << "Show register infomation failed." << endl;
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
			cout << "CF  1" << endl;
		}
		else {
			cout << "CF  0" << endl;
		}

		if ((context.EFlags & 0x4) != 0)
		{
			cout << "PF  1" << endl;;
		}
		else {
			cout << "PF  0" << endl;
		}

		if ((context.EFlags & 0x10) != 0)
		{
			cout << "AF  1" << endl;;
		}
		else {
			cout << "AF  0" << endl;
		}

		if ((context.EFlags & 0x40) != 0)
		{
			cout << "ZF  1" << endl;;
		}
		else {
			cout << "ZF  0" << endl;
		}

		if ((context.EFlags & 0x80) != 0)
		{
			cout << "SF  1" << endl;;
		}
		else {
			cout << "SF  0" << endl;
		}

		if ((context.EFlags & 0x400) != 0)
		{
			cout << "OF  1" << endl;;
		}
		else {
			std::cout << "OF  0" << endl;
		}

		if ((context.EFlags & 0x200) != 0)
		{
			std::cout << "DF  1" << endl;;
		}
		else {
			std::cout << "DF  0" << endl;
		}
		std::cout << endl;
	}
}