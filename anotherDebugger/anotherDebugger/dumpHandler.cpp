#include "main.h"

void printHex1(unsigned int value)
{
	cout << hex << uppercase;

	cout << " 0x";

	cout << std::setw(2) << std::setfill('0') << value << std::dec << std::nouppercase << std::flush;
}

void OnDump(const Command& cmd)
{
	if (getDebuggeeStatus() == DebuggeeStatus::NONE)
	{
		cout << "Debuggee not started." << endl;
		return;
	}

	if (cmd.size() != 3)
	{
		cout << "Invalid command" << endl;
		return;
	}

	unsigned int address;
	stringstream ss(cmd[1]);
	ss >> hex >> address;

	unsigned int len;
	stringstream ss1(cmd[2]);
	ss1 >> len;

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
		if (ReadDebuggeeMemory(address + i, 1, &byte) == TRUE)
		{
			printHex1(byte);
			cout << " ";
		}
		else
		{
			cout << "0x?? ";
		}
	}
	cout << endl;
}