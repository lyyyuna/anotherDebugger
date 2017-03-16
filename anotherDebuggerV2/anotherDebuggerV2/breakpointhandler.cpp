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

	AnotherDebugger::BpType AnotherDebugger::getBreakPoint(DWORD addr)
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

		// other type breakpoint
		return BpType::OTHER;
	}
}