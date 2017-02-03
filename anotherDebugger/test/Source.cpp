#include <stdio.h>
#include <Windows.h>

int wmain() {

	//OutputDebugString(TEXT("Warning! An exception will be thrown!"));

	__try {

		int a = 0;
		int b = 10 / a;

	}
	__except (EXCEPTION_EXECUTE_HANDLER) {

		OutputDebugString(TEXT("Entered exception handler."));
	}

	printf("hello");
}