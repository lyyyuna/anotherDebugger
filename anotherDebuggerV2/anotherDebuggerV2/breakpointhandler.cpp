#include <iostream>
#include <string>
#include "anotherdebugger.h"

namespace anotherdebugger
{
	void AnotherDebugger::resetBreakPoint()
	{
		bpUserList.clear();
		isInitBpSet = false;

		bpStepOut.address = 0;
		bpStepOver.content = 0;

		bpStepOver.address = 0;
		bpStepOut.content = 0;
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
			Flag::continueStatus = DBG_CONTINUE;
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
}