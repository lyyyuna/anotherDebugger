#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include "anotherdebugger.h"

using namespace std;

namespace anotherdebugger
{
	void AnotherDebugger::onDump(const Command & cmds)
	{
		if (debuggeeStatus == DebuggeeStatus::NONE)
		{
			cout << "Debuggee not started." << endl;
			return;
		}

		if (cmds.size() != 3)
		{
			cout << "Invalid command" << endl;
			return;
		}

		unsigned int address;
		stringstream ss(cmds[1]);
		ss >> hex >> address;

		unsigned int len;
		stringstream ss1(cmds[2]);
		ss1 >> len;

		auto printHex = [](unsigned int value) -> void
		{
			cout << hex << uppercase;

			cout << " 0x";

			cout << std::setw(2) << std::setfill('0') << value << std::dec << std::nouppercase << std::flush;
		};

		auto readDebuggeeMemory = [this](unsigned int address, unsigned int length, void* pData) -> BOOL
		{
			SIZE_T bytesRead;

			return ReadProcessMemory(this->debuggeehProcess, (LPCVOID)address, pData, length, &bytesRead);
		};

		for (unsigned int i = 0; i < len; i++)
		{
			if ((i) % 8 == 0)
			{
				cout << endl;
				cout << hex << uppercase << " 0x" << setw(8) << setfill('0')
					<< address + i << dec << nouppercase << flush;
				cout << "  ";
			}

			BYTE byte;
			if (readDebuggeeMemory(address + i, 1, &byte) == TRUE)
			{
				printHex(byte);
				cout << " ";
			}
			else
			{
				cout << "0x?? ";
			}
		}
		cout << endl;
	}
}