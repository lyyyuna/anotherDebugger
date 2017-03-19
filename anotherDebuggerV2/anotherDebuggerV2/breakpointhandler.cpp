#include <iostream>
#include <iomanip>
#include <string>
#include <list>
#include "anotherdebugger.h"

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

		if (cmds.size() == 1)
		{
			displayBreakPoints();
			return;
		}


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
}