#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <list>
#include "anotherdebugger.h"
#include <Windows.h>
#include <DbgHelp.h>
#include "SymbolType.h"
#include <codecvt>

using namespace std;

namespace anotherdebugger
{
	bool AnotherDebugger::isPODType(int typeID, DWORD modBase)
	{
		DWORD symTag;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_SYMTAG,
			&symTag);

		switch (symTag)
		{
		case SymTagBaseType:
		case SymTagPointerType:
		// case SymTagEnum:
			return true;
		default:
			return false;
		}
	}

	string AnotherDebugger::getTypeName(int typeID, DWORD modBase)
	{
		DWORD typeTag;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_SYMTAG,
			&typeTag);

		switch (typeTag)
		{
		case SymTagBaseType:
			return getBaseTypeName(typeID, modBase);

		case SymTagPointerType:
			return getPointerTypeName(typeID, modBase);

		case SymTagArrayType:
			return getArrayTypeName(typeID, modBase);

		case SymTagUDT:
			return getUDTTypeName(typeID, modBase);

		case SymTagEnum:
			return getEnumTypeName(typeID, modBase);

		case SymTagFunctionType:
			return getFunctionTypeName(typeID, modBase);

		default:
			return "??";
		}
	}

	CBaseTypeEnum AnotherDebugger::getCBaseType(int typeID, DWORD modBase) {

		DWORD baseType;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_BASETYPE,
			&baseType);

		ULONG64 length;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_LENGTH,
			&length);

		switch (baseType) 
		{
			case btVoid:
				return CBaseTypeEnum::cbtVoid;

			case btChar:
				return CBaseTypeEnum::cbtChar;

			case btWChar:
				return CBaseTypeEnum::cbtWChar;

			case btInt:
				switch (length) 
				{
					case 2:  return CBaseTypeEnum::cbtShort;
					case 4:  return CBaseTypeEnum::cbtInt;
					default: return CBaseTypeEnum::cbtLongLong;
				}

			case btUInt:
				switch (length) 
				{
					case 1:  return CBaseTypeEnum::cbtUChar;
					case 2:  return CBaseTypeEnum::cbtUShort;
					case 4:  return CBaseTypeEnum::cbtUInt;
					default: return CBaseTypeEnum::cbtULongLong;
				}

			case btFloat:
				switch (length) 
				{
					case 4:  return CBaseTypeEnum::cbtFloat;
					default: return CBaseTypeEnum::cbtDouble;
				}

			case btBool:
				return CBaseTypeEnum::cbtBool;

			case btLong:
				return CBaseTypeEnum::cbtLong;

			case btULong:
				return CBaseTypeEnum::cbtULong;

			default:
				return CBaseTypeEnum::cbtNone;
		}
	}

	string AnotherDebugger::getBaseTypeName(int typeID, DWORD modBase)
	{
		auto type = getCBaseType(typeID, modBase);

		auto iter = baseTypeNameMap.find(type);
		if (iter == baseTypeNameMap.end())
		{
			return "";
		}
		else
		{
			return iter->second;
		}
	}

	string AnotherDebugger::getPointerTypeName(int typeID, DWORD modBase)
	{
		BOOL isReference;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_IS_REFERENCE,
			&isReference);

		DWORD innerTypeID;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_TYPEID,
			&innerTypeID);

		return getTypeName(innerTypeID, modBase) + (isReference == TRUE ? "&" : "*");
	}

	string AnotherDebugger::getArrayTypeName(int typeID, DWORD modBase) 
	{

		DWORD innerTypeID;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_TYPEID,
			&innerTypeID);

		DWORD elemCount;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_COUNT,
			&elemCount);

		ostringstream strBuilder;

		strBuilder << getTypeName(innerTypeID, modBase) << '[' << elemCount << ']';

		return strBuilder.str();
	}

	string AnotherDebugger::getFunctionTypeName(int typeID, DWORD modBase)
	{
		DWORD returnTypeID;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_TYPEID,
			&returnTypeID);

		ostringstream funcDecl;
		funcDecl << getTypeName(returnTypeID, modBase);

		DWORD paramCount;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_CHILDRENCOUNT,
			&paramCount);

		BYTE* pBuffer = (BYTE*)malloc(sizeof(TI_FINDCHILDREN_PARAMS) + sizeof(ULONG) * paramCount);
		TI_FINDCHILDREN_PARAMS* pParams = (TI_FINDCHILDREN_PARAMS*)pBuffer;
		pParams->Count = paramCount;
		pParams->Start = 0;

		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_FINDCHILDREN,
			pParams);

		funcDecl << "(";

		for (int index = 0; index != paramCount; ++index) 
		{
			DWORD paramTypeID;
			SymGetTypeInfo(
				debuggeehProcess,
				modBase,
				pParams->ChildId[index],
				TI_GET_TYPEID,
				&paramTypeID);

			if (index != 0) 
			{
				funcDecl << ", ";
			}

			funcDecl << getTypeName(paramTypeID, modBase);
		}

		funcDecl << ")";

		free(pBuffer);

		return funcDecl.str();
	}

	string AnotherDebugger::getNameableTypeName(int typeID, DWORD modBase)
	{
		WCHAR* pBuffer;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_SYMNAME,
			&pBuffer);  // TI_GET_SYMNAME must be a WCHAR**

		wstring wtypeName(pBuffer);

		LocalFree(pBuffer);

		string typeName(wtypeName.begin(), wtypeName.end());

		return typeName;
	}

	string AnotherDebugger::getEnumTypeName(int typeID, DWORD modBase)
	{
		return getNameableTypeName(typeID, modBase);
	}

	string AnotherDebugger::getUDTTypeName(int typeID, DWORD modBase)
	{
		return getNameableTypeName(typeID, modBase);
	}


	// Value

}