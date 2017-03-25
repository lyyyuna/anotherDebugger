#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <list>
#include <Windows.h>
#include <DbgHelp.h>
#include "anotherdebugger.h"

using namespace std;

namespace anotherdebugger
{
	enum BaseTypeEnum {
		btNoType = 0,
		btVoid = 1,
		btChar = 2,
		btWChar = 3,
		btInt = 6,
		btUInt = 7,
		btFloat = 8,
		btBCD = 9,
		btBool = 10,
		btLong = 13,
		btULong = 14,
		btCurrency = 25,
		btDate = 26,
		btVariant = 27,
		btComplex = 28,
		btBit = 29,
		btBSTR = 30,
		btHresult = 31
	};
	
	enum SymTagEnum {
		SymTagNull,
		SymTagExe,
		SymTagCompiland,
		SymTagCompilandDetails,
		SymTagCompilandEnv,
		SymTagFunction,				// 函数
		SymTagBlock,
		SymTagData,					// 变量
		SymTagAnnotation,
		SymTagLabel,
		SymTagPublicSymbol,
		SymTagUDT,					// 用户定义类型，例如struct，class和union
		SymTagEnum,					// 枚举类型
		SymTagFunctionType,			// 函数类型
		SymTagPointerType,			// 指针类型
		SymTagArrayType,			// 数组类型
		SymTagBaseType,				// 基本类型
		SymTagTypedef,				// typedef类型
		SymTagBaseClass,			// 基类
		SymTagFriend,				// 友元类型
		SymTagFunctionArgType,		// 函数参数类型
		SymTagFuncDebugStart,
		SymTagFuncDebugEnd,
		SymTagUsingNamespace,
		SymTagVTableShape,
		SymTagVTable,
		SymTagCustom,
		SymTagThunk,
		SymTagCustomType,
		SymTagManagedType,
		SymTagDimension
	};
	
	enum class CBaseTypeEnum {
		cbtNone,
		cbtVoid,
		cbtBool,
		cbtChar,
		cbtUChar,
		cbtWChar,
		cbtShort,
		cbtUShort,
		cbtInt,
		cbtUInt,
		cbtLong,
		cbtULong,
		cbtLongLong,
		cbtULongLong,
		cbtFloat,
		cbtDouble,
		cbtEnd,
	};

	struct VariableInfo
	{
		DWORD address;
		DWORD modBase;
		DWORD size;
		DWORD typeID;
		string name;
	};

	map<CBaseTypeEnum, string> baseTypeNameMap =
	{
		{ cbtNone, "<no-type>" },
		{ cbtVoid, "void" },
		{ cbtBool, "bool" },
		{ cbtChar, "char" },
		{ cbtUChar, "unsigned char" },
		{ cbtWChar, "wchar_t" },
		{ cbtShort, "short" },
		{ cbtUShort, "unsigned short" },
		{ cbtInt, "int" },
		{ cbtUInt, "unsigned int" },
		{ cbtLong, "long" },
		{ cbtULong, "unsigned long" },
		{ cbtLongLong, "long long" },
		{ cbtULongLong, "unsigned long long" },
		{ cbtFloat, "float" },
		{ cbtDouble, "double" },

	};
}