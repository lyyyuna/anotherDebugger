#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>
#include "anotherdebugger.h"
#include <Windows.h>
#include <DbgHelp.h>

using namespace std;

namespace anotherdebugger
{
	void AnotherDebugger::onShowSourceLines(const Command & cmds)
	{
		if (debuggeeStatus == DebuggeeStatus::NONE)
		{
			cout << "Debuggee not started." << endl;
			return;
		}

		CONTEXT context;
		getDebuggeeContext(&context);

		IMAGEHLP_LINE64 lineInfo = { 0 };

		lineInfo.SizeOfStruct = sizeof(lineInfo);
		DWORD displacement = 0;

		if (SymGetLineFromAddr64(
			debuggeehProcess,
			context.Eip,
			&displacement,
			&lineInfo
			) == FALSE)
		{
			DWORD err = GetLastError();
			cout << err << endl;
			switch (err)
			{
			case 126:
				cout << "Debug info in current module has not loaded" << endl;
				return;
			case 487:
				cout << "No debug info in current module." << endl;
				return;
			default:
				cout << "SymGetLineFromAddr64 failed: " << err << endl;
				return;
			}
		}

		// parse command
		int start;
		int len;
		if (cmds.size() == 3)
		{
			stringstream ss(cmds[1]);

			ss >> start;
			if (start < 0)
			{
				cout << "Invalid params" << endl;
				return;
			}

			stringstream ss1(cmds[2]);

			ss1 >> len;
			if (len < 0)
			{
				cout << "Invalid params" << endl;
				return;
			}

			displaySourceLines(
				lineInfo.FileName,
				lineInfo.LineNumber,
				lineInfo.Address,
				start,
				len
				);
		}
		else if (cmds.size() == 2)
		{
			stringstream ss1(cmds[1]);

			ss1 >> len;
			if (len < 0)
			{
				cout << "Invalid params" << endl;
				return;
			}
			displayFromCurLine(
				lineInfo.FileName,
				lineInfo.LineNumber,
				lineInfo.Address,
				len
				);
		}
		else
		{
			cout << "Invalid params" << endl;
			return;
		}
	}


	void AnotherDebugger::displayFromCurLine(LPCTSTR srcfile, int linenum, DWORD64 addr, int len)
	{
		cout << endl;

		ifstream srcStream(srcfile);

		if (srcStream.fail() == true)
		{
			cout << "Open file failed." << endl;
			cout << "The path is: " << srcfile << endl;
			return;
		}

		string line;
		int curlinenum = 1;

		while (curlinenum < linenum)
		{
			getline(srcStream, line);
			curlinenum++;
		}

		while (curlinenum < linenum + len)
		{
			if (curlinenum == linenum)
			{
				if (!getline(srcStream, line))
				{
					break;
				}
				displayOneLine(srcfile, line, curlinenum, true);
			}
			else {
				if (!getline(srcStream, line))
				{
					break;
				}
				displayOneLine(srcfile, line, curlinenum, false);
			}
			curlinenum++;
		}

	}

	void AnotherDebugger::displaySourceLines(LPCTSTR srcfile, int linenum, DWORD64 addr, int start, int len)
	{
		cout << endl;

		ifstream srcStream(srcfile);

		if (srcStream.fail() == true)
		{
			cout << "Open file failed." << endl;
			cout << "The path is: " << srcfile << endl;
			return;
		}

		string line;
		int curlinenum = 1;
		while (curlinenum < start)
		{
			getline(srcStream, line);
			curlinenum++;
		}

		while (curlinenum < (start + len))
		{
			if (curlinenum == linenum)
			{
				if (!getline(srcStream, line))
				{
					break;
				}
				displayOneLine(srcfile, line, curlinenum, true);
			}
			else {
				if (!getline(srcStream, line))
				{
					break;
				}
				displayOneLine(srcfile, line, curlinenum, false);
			}
			curlinenum++;
		}

		srcStream.close();
	}

	void AnotherDebugger::displayOneLine(LPCTSTR srcfile, string & line, int linenum, bool isCurline)
	{
		if (isCurline == true)
		{
			cout << "=>";
		}
		else
		{
			cout << "  ";
		}

		LONG displacement;
		IMAGEHLP_LINE64 lineinfo = { 0 };
		lineinfo.SizeOfStruct = sizeof(lineinfo);

		if (SymGetLineFromName64(
			debuggeehProcess,
			NULL,
			srcfile,
			linenum,
			&displacement,
			&lineinfo
			) == FALSE)
		{
			cout << "SymGetLineFromName64 failed: " << GetLastError() << endl;
			return;
		}

		cout << setw(4) << setfill(' ') << linenum << " ";

		cout << line << endl;
	}

}