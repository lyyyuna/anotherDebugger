#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <list>
#include "anotherdebugger.h"
#include <Windows.h>
#include <DbgHelp.h>

using namespace std;

namespace anotherdebugger
{
	void AnotherDebugger::resetBreakPointHandler()
	{
		bpUserList.clear();
		isInitBpSet = false;

		bpStepOut.address = 0;
		bpStepOver.content = 0;

		bpStepOver.address = 0;
		bpStepOut.content = 0;

		FLAG.continueStatus = DBG_CONTINUE;
		FLAG.isBeingSingleInstruction = false;
		FLAG.isBeingStepOut = false;
		FLAG.isBeingStepOver = false;
		FLAG.resetUserBreakPointAddress = 0;
		FLAG.glf.lineNumber = 0;
		FLAG.glf.filePath = string();
	}

	void AnotherDebugger::onSetBreakPoint(const Command & cmds)
	{
		if (debuggeeStatus == DebuggeeStatus::NONE)
		{
			cout << "Debuggee not started." << endl;
			return;
		}

		auto printHex = [](unsigned int value) -> void
		{
			cout << hex << uppercase;
			cout << " 0x";
			cout << std::setw(2) << std::setfill('0') << value << std::dec << std::nouppercase << std::flush;
		};

		auto displayBreakPoints = [this, printHex]() -> void
		{
			if (this->bpUserList.size() == 0)
			{
				cout << "No break point." << endl;
				return;
			}

			for (auto it = this->bpUserList.begin(); it !=
				this->bpUserList.end();
				++it)
			{
				printHex(it->address);
				cout << endl;
			}
		};

		auto getBreakPointAddress = [this](int line) -> DWORD
		{
			CONTEXT context;
			getDebuggeeContext(&context);

			IMAGEHLP_LINE64 lineInfo = { 0 };
			lineInfo.SizeOfStruct = sizeof(lineInfo);
			DWORD displacement = 0;

			if (SymGetLineFromAddr64(
				debuggeehProcess,
				context.Eip,
				&displacement,
				&lineInfo) == FALSE) {

				DWORD errorCode = GetLastError();

				switch (errorCode) {

				case 126:
					cout << "Debug info in current module has not loaded." << endl;
					return 0;

				case 487:
					cout << "No debug info in current module." << endl;
					return 0;

				default:
					cout << "SymGetLineFromAddr64 failed: " << errorCode << endl;
					return 0;
				}
			}

			LONG displacement2 = 0;
			if (SymGetLineFromName64(
				debuggeehProcess,
				NULL,
				lineInfo.FileName,
				line,
				&displacement2,
				&lineInfo) == FALSE) {

				std::wcout << TEXT("SymGetLineFromName64 failed: ") << GetLastError() << std::endl;
				return 0;
			}

			if (displacement == 0)
			{
				// find the right address of the line.
				return (DWORD)lineInfo.Address;
			}
			else
			{
				// not find. maybe it is a blank line.
				return 0;
			}
		};

		if (cmds.size() == 1)
		{
			displayBreakPoints();
			return;
		}

		if (cmds.size() != 2)
		{
			cout << "Invalid params." << endl;
			return;
		}

		unsigned int line;
		stringstream ss(cmds[1]);
		ss >> line;
		auto bpAddress = getBreakPointAddress(line);
		if (bpAddress == 0)
		{
			cout << "Cannot deduce the right breakpoint address, please set another one." << endl;
			return;
		}

		if (cmds[0] == "b")
		{
			if (true == setUserBreakPointAt(bpAddress))
			{
				cout << "Set break point success." << endl;
			}
			else {
				cout << "Set break point failed." << endl;
			}
			return;
		}
		else if (cmds[0] == "bp")
		{
			if (true == deleteUserBreakPointAt(bpAddress))
			{
				cout << "delete break point success." << endl;
			}
			else {
				cout << "delete break point failed." << endl;
			}
			return;
		}

		cout << "Invalid params" << endl;
		return;
	}

	bool AnotherDebugger::setUserBreakPointAt(DWORD addr)
	{
		for (auto it = bpUserList.begin();
			it != bpUserList.end();
			++it)
		{
			if (it->address == addr)
			{
				cout << "The break point already exist." << endl;
				return false;
			}
		}

		BreakPoint newBp;
		newBp.address = addr;
		newBp.content = setBreakPointAt(addr);

		bpUserList.push_back(newBp);

		return true;
	}

	bool AnotherDebugger::deleteUserBreakPointAt(DWORD addr)
	{
		for (auto it = bpUserList.begin();
			it != bpUserList.end();
			++it)
		{
			if (it->address == addr)
			{
				recoverBreakPoint(*it);
				bpUserList.erase(it);

				return true;
			}
		}

		cout << "The break point does not exist. " << endl;

		return false;
	}

	AnotherDebugger::BpType AnotherDebugger::getBreakPointType(DWORD addr)
	{
		if (isInitBpSet == false)
		{
			isInitBpSet = true;
			return BpType::INIT;
		}

		// stepover check should ahead breakpoint
		if (bpStepOver.address == addr)
		{
			return BpType::STEP_OVER;
		}

		if (bpStepOut.address == addr)
		{
			return BpType::STEP_OUT;
		}

		for (auto iter = bpUserList.begin(); iter != bpUserList.end(); iter++)
		{
			if (iter->address == addr)
			{
				return BpType::USER;
			}
		}

		// other type breakpoint (maybe in the debuggee program itself)
		return BpType::CODE;
	}

	bool AnotherDebugger::onBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo)
	{
		auto bpType = getBreakPointType((DWORD)(pInfo->ExceptionRecord.ExceptionAddress));

		switch (bpType)
		{
		case BpType::INIT:
			FLAG.continueStatus = DBG_CONTINUE;
			return true;

		case BpType::CODE:
			return onNormalBreakPoint(pInfo);

		case BpType::STEP_OVER:
			deleteStepOverBreakPoint();
			backwardDebuggeeEIP();
			return onSingleStepCommonProcedures();

		case BpType::USER:
			return onUserBreakPoint(pInfo);

		case BpType::STEP_OUT:
			return onStepOutBreakPoint(pInfo);
		}

		return true;
	}

	bool AnotherDebugger::onNormalBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo)
	{
		if (true == FLAG.isBeingSingleInstruction)
		{
			FLAG.continueStatus = DBG_CONTINUE;
			return true; //
		}

		if (true == FLAG.isBeingStepOver)
		{
			deleteStepOverBreakPoint();
			FLAG.isBeingStepOver = false;
		}

		if (true == FLAG.isBeingStepOut)
		{
			deleteStepOutBreakPoint();
			FLAG.isBeingStepOut = false;
		}

		cout << "A break point happened at: ";
		auto printHex = [](unsigned int value) -> void
		{
			cout << hex << uppercase;
			cout << " 0x";
			cout << std::setw(2) << std::setfill('0') << value << std::dec << std::nouppercase << std::flush;
		};

		printHex((DWORD)pInfo->ExceptionRecord.ExceptionAddress);
		cout << "." << endl;

		// always continue?

		debuggeeStatus = DebuggeeStatus::INTERRUPTED;
		return false; //
	}

	bool AnotherDebugger::onUserBreakPoint(const EXCEPTION_DEBUG_INFO * pInfo)
	{
		auto recoverUserBreakPoint = [this](DWORD address) -> bool
		{
			for (auto it = this->bpUserList.begin();
				it != this->bpUserList.end();
				++it)
			{
				if (it->address == address)
				{
					this->recoverBreakPoint(*it);
					return true;
				}
			}
			cout << "Error, the breakpoint seems not exist." << endl; 

			return false;
		};

		auto saveResetUserBreakPoint = [this](DWORD address) -> void
		{
			this->FLAG.resetUserBreakPointAddress = address;
		};

		recoverUserBreakPoint((DWORD)pInfo->ExceptionRecord.ExceptionAddress);
		backwardDebuggeeEIP();
		setCPUTrapFlag();
		saveResetUserBreakPoint((DWORD)pInfo->ExceptionRecord.ExceptionAddress);

		return onNormalBreakPoint(pInfo);
	}

	bool AnotherDebugger::onSingleStepTrap(const EXCEPTION_DEBUG_INFO * pInfo)
	{
		auto resetUserBreakPoint = [this]() -> void
		{
			for (auto it = this->bpUserList.begin();
				it != this->bpUserList.end();
				++it)
			{
				if (it->address == this->FLAG.resetUserBreakPointAddress)
				{
					setBreakPointAt(it->address);
					this->FLAG.resetUserBreakPointAddress = 0;
				}
			}
		};

		if (0 != FLAG.resetUserBreakPointAddress)
		{
			resetUserBreakPoint();
		}

		if (true == FLAG.isBeingSingleInstruction)
		{
			return onSingleStepCommonProcedures();
		}

		FLAG.continueStatus = DBG_CONTINUE;
		return true;
	}

	bool AnotherDebugger::onSingleStepCommonProcedures()
	{
		if (isLineChanged() == false)
		{
			if (true == FLAG.isBeingStepOver)
			{
				CONTEXT c;
				getDebuggeeContext(&c);

			}
		}

		return false;
	}

	void AnotherDebugger::saveCurrentLineInfo()
	{
		getCurrentLineInfo(FLAG.glf);
	}

	bool AnotherDebugger::isLineChanged()
	{
		LineInfo lf;
		if (false == getCurrentLineInfo(lf))
		{
			return false;
		}

		if (lf.lineNumber == FLAG.glf.lineNumber &&
			lf.filePath == FLAG.glf.filePath)
		{
			return false;
		}

		return true;
	}

	bool AnotherDebugger::getCurrentLineInfo(LineInfo & lf)
	{
		CONTEXT context;
		getDebuggeeContext(&context);

		DWORD displacement;
		IMAGEHLP_LINE64 lineInfo = { 0 };
		lineInfo.SizeOfStruct = sizeof(lineInfo);

		if (SymGetLineFromAddr64(
			debuggeehProcess,
			context.Eip,
			&displacement,
			&lineInfo) == TRUE) {

			lf.filePath = string(lineInfo.FileName);
			lf.lineNumber = lineInfo.LineNumber;

			return true;
		}
		else {
			lf.filePath = string();
			lf.lineNumber = 0;

			return false;
		}
	}
}