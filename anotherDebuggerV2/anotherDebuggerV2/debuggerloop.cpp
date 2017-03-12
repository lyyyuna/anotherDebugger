#include <iostream>
#include <string>
#include <sstream>
#include "anotherdebugger.h"

using namespace std;

namespace anotherdebugger
{
	AnotherDebugger::AnotherDebugger(bool flag) : 
		DEBUG(flag),
		debuggeeStatus(DebuggeeStatus::NONE),
		debuggeehProcess(NULL),
		debuggeehThread(NULL),
		debuggeeprocessID(0),
		debuggeethreadID(0)
	{
		cmdmap.insert(make_pair("s", &AnotherDebugger::onStartDebug));
		cmdmap.insert(make_pair("g", &AnotherDebugger::onGo));
		cmdmap.insert(make_pair("d", &AnotherDebugger::onDump));
		cmdmap.insert(make_pair("r", &AnotherDebugger::onShowRegisters));
		cmdmap.insert(make_pair("t", &AnotherDebugger::onStopDebug));
		cmdmap.insert(make_pair("l", &AnotherDebugger::onShowSourceLines));
	}

	void AnotherDebugger::startDebuggerLoop()
	{
		cout << "Another Debugger by lyyyuna" << endl;
		
		string cmdline;

		while (true)
		{
			cout << endl << "> ";
			getline(cin, cmdline);

			parseCommand(cmdline);
			if (false == dispatchCommand())
			{
				break;
			}

			cleanCommand();
		}
	}

	void AnotherDebugger::parseCommand(string & cmdline)
	{
		istringstream cmdstrStream(cmdline);

		string arg;
		while (cmdstrStream >> arg)
		{
			cmds.push_back(arg);
		}
	}

	bool AnotherDebugger::dispatchCommand()
	{
		if (cmds.size() == 0)
		{
			cout << "Invalid command." << endl;
			return true;
		}

		if ("q" == cmds[0])
		{
			return false;
		}


		auto cmditer = cmdmap.find(cmds[0]);
		if (cmditer == cmdmap.end())
		{
			cout << "Invalid command." << endl;
		}
		else
		{
			auto handler = cmditer->second;
			(this->*handler)(cmds);
		}

		return true;
	}
}