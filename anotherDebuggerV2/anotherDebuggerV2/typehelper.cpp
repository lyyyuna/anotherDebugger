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

	map<CBaseTypeEnum, string> baseTypeNameMap =
	{
		{ CBaseTypeEnum::cbtNone, "<no-type>" },
		{ CBaseTypeEnum::cbtVoid, "void" },
		{ CBaseTypeEnum::cbtBool, "bool" },
		{ CBaseTypeEnum::cbtChar, "char" },
		{ CBaseTypeEnum::cbtUChar, "unsigned char" },
		{ CBaseTypeEnum::cbtWChar, "wchar_t" },
		{ CBaseTypeEnum::cbtShort, "short" },
		{ CBaseTypeEnum::cbtUShort, "unsigned short" },
		{ CBaseTypeEnum::cbtInt, "int" },
		{ CBaseTypeEnum::cbtUInt, "unsigned int" },
		{ CBaseTypeEnum::cbtLong, "long" },
		{ CBaseTypeEnum::cbtULong, "unsigned long" },
		{ CBaseTypeEnum::cbtLongLong, "long long" },
		{ CBaseTypeEnum::cbtULongLong, "unsigned long long" },
		{ CBaseTypeEnum::cbtFloat, "float" },
		{ CBaseTypeEnum::cbtDouble, "double" },

	};

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
	string AnotherDebugger::getTypeValue(int typeID, DWORD modBase, DWORD address, const BYTE* pData)
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
			return getBaseTypeValue(typeID, modBase, pData);

		case SymTagPointerType:
			return getPointerTypeValue(typeID, modBase, pData);

		case SymTagEnum:
			return getEnumTypeValue(typeID, modBase, pData);

		case SymTagArrayType:
			return getArrayTypeValue(typeID, modBase, address, pData);

		case SymTagUDT:
			return getUDTTypeValue(typeID, modBase, address, pData);

		case SymTagTypedef:

			//获取真正类型的ID
			DWORD actTypeID;
			SymGetTypeInfo(
				debuggeehProcess,
				modBase,
				typeID,
				TI_GET_TYPEID,
				&actTypeID);

			return getTypeValue(actTypeID, modBase, address, pData);

		default:
			return "??";
		}
	}

	string AnotherDebugger::getCBaseTypeValue(CBaseTypeEnum cBaseType, const BYTE* pData)
	{
		ostringstream valueStr;

		switch (cBaseType) {

		case CBaseTypeEnum::cbtNone:
			valueStr << "??";
			break;

		case CBaseTypeEnum::cbtVoid:
			valueStr << "??";
			break;

		case CBaseTypeEnum::cbtBool:
			valueStr << (*pData == 0 ? "false" : "true");
			break;

		case CBaseTypeEnum::cbtChar:
			valueStr << (*((char*)pData));
			break;

		case CBaseTypeEnum::cbtUChar:
			valueStr << std::hex
				<< std::uppercase
				<< std::setw(2)
				<< std::setfill(TEXT('0'))
				<< *((unsigned char*)pData);
			break;

		case CBaseTypeEnum::cbtWChar:
			valueStr << (*((wchar_t*)pData));
			break;

		case CBaseTypeEnum::cbtShort:
			valueStr << *((short*)pData);
			break;

		case CBaseTypeEnum::cbtUShort:
			valueStr << *((unsigned short*)pData);
			break;

		case CBaseTypeEnum::cbtInt:
			valueStr << *((int*)pData);
			break;

		case CBaseTypeEnum::cbtUInt:
			valueStr << *((unsigned int*)pData);
			break;

		case CBaseTypeEnum::cbtLong:
			valueStr << *((long*)pData);
			break;

		case CBaseTypeEnum::cbtULong:
			valueStr << *((unsigned long*)pData);
			break;

		case CBaseTypeEnum::cbtLongLong:
			valueStr << *((long long*)pData);
			break;

		case CBaseTypeEnum::cbtULongLong:
			valueStr << *((unsigned long long*)pData);
			break;

		case CBaseTypeEnum::cbtFloat:
			valueStr << *((float*)pData);
			break;

		case CBaseTypeEnum::cbtDouble:
			valueStr << *((double*)pData);
			break;
		}

		return valueStr.str();
	}

	string AnotherDebugger::getBaseTypeValue(int typeID, DWORD modBase, const BYTE* pData)
	{
		auto type = getCBaseType(typeID, modBase);

		return getCBaseTypeValue(type, pData);
	}

	string AnotherDebugger::getPointerTypeValue(int typeID, DWORD modBase, const BYTE* pData)
	{
		ostringstream valueStr;

		valueStr << hex << uppercase << 
			setfill('0') << setw(8) <<
			*((DWORD*)pData);

		return valueStr.str();
	}

	string AnotherDebugger::getEnumTypeValue(int typeID, DWORD modBase, const BYTE* pData)
	{
		string valueName;

		auto type = getCBaseType(typeID, modBase);

		DWORD childrenCount;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_CHILDRENCOUNT,
			&childrenCount);

		TI_FINDCHILDREN_PARAMS* pFindParams =
			(TI_FINDCHILDREN_PARAMS*)malloc(sizeof(TI_FINDCHILDREN_PARAMS) + childrenCount * sizeof(ULONG));
		pFindParams->Start = 0;
		pFindParams->Count = childrenCount;

		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_FINDCHILDREN,
			pFindParams);

		for (int index = 0; index != childrenCount; ++index) 
		{
			VARIANT enumValue;
			SymGetTypeInfo(
				debuggeehProcess,
				modBase,
				pFindParams->ChildId[index],
				TI_GET_VALUE,
				&enumValue);

			if (variantEqual(enumValue, type, pData) == true) 
			{
				WCHAR* pBuffer;
				SymGetTypeInfo(
					debuggeehProcess,
					modBase,
					pFindParams->ChildId[index],
					TI_GET_SYMNAME,
					&pBuffer);

				LocalFree(pBuffer);

				auto wvalueName = wstring(pBuffer);
				string valuename(wvalueName.cbegin(), wvalueName.cend());
				valueName = valuename;

				break;
			}
		}

		free(pFindParams);

		if (valueName.length() == 0) 
		{
			valueName = getBaseTypeValue(typeID, modBase, pData);
		}

		return valueName;
		
	}

	bool AnotherDebugger::variantEqual(VARIANT var, CBaseTypeEnum cBaseType, const BYTE* pData) 
	{
		switch (cBaseType) 
		{

		case CBaseTypeEnum::cbtChar:
			return var.cVal == *((char*)pData);

		case CBaseTypeEnum::cbtUChar:
			return var.bVal == *((unsigned char*)pData);

		case CBaseTypeEnum::cbtShort:
			return var.iVal == *((short*)pData);

		case CBaseTypeEnum::cbtWChar:
		case CBaseTypeEnum::cbtUShort:
			return var.uiVal == *((unsigned short*)pData);

		case CBaseTypeEnum::cbtUInt:
			return var.uintVal == *((int*)pData);

		case CBaseTypeEnum::cbtLong:
			return var.lVal == *((long*)pData);

		case CBaseTypeEnum::cbtULong:
			return var.ulVal == *((unsigned long*)pData);

		case CBaseTypeEnum::cbtLongLong:
			return var.llVal == *((long long*)pData);

		case CBaseTypeEnum::cbtULongLong:
			return var.ullVal == *((unsigned long long*)pData);

		case CBaseTypeEnum::cbtInt:
		default:
			return var.intVal == *((int*)pData);
		}
	}

	string AnotherDebugger::getArrayTypeValue(int typeID, DWORD modBase, DWORD address, const BYTE * pData)
	{
		DWORD elemCount;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_COUNT,
			&elemCount);

		elemCount = elemCount > 32 ? 32 : elemCount;

		DWORD innerTypeID;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_TYPEID,
			&innerTypeID);

		ULONG64 elemLen;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			innerTypeID,
			TI_GET_LENGTH,
			&elemLen);

		ostringstream valueStr;

		for (int index = 0; index != elemCount; ++index) 
		{
			DWORD elemOffset = DWORD(index * elemLen);

			valueStr << TEXT("  [") << index << TEXT("]  ")
				<< getTypeValue(innerTypeID, modBase, address + elemOffset, pData + index * elemLen);

			if (index != elemCount - 1) 
			{
				valueStr << std::endl;
			}
		}

		return valueStr.str();
	}

	string AnotherDebugger::getUDTTypeValue(int typeID, DWORD modBase, DWORD address, const BYTE * pData)
	{
		DWORD memberCount;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_GET_CHILDRENCOUNT,
			&memberCount);

		TI_FINDCHILDREN_PARAMS* pFindParams =
			(TI_FINDCHILDREN_PARAMS*)malloc(sizeof(TI_FINDCHILDREN_PARAMS) + memberCount * sizeof(ULONG));
		pFindParams->Start = 0;
		pFindParams->Count = memberCount;

		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			typeID,
			TI_FINDCHILDREN,
			pFindParams);

		ostringstream valueStr;

		for (int index = 0; index != memberCount; ++index) 
		{
			auto isDataMember = getDataMemberInfo(
				pFindParams->ChildId[index],
				modBase,
				address,
				pData,
				valueStr);

			if (isDataMember == true) 
			{
				valueStr << std::endl;
			}
		}

		return valueStr.str();
	}

	bool AnotherDebugger::getDataMemberInfo(DWORD memberID, DWORD modBase, DWORD address, const BYTE* pData, ostringstream& valueStr)
	{
		DWORD memberTag;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			memberID,
			TI_GET_SYMTAG,
			&memberTag);

		if (memberTag != SymTagData && memberTag != SymTagBaseClass) 
		{
			return false;
		}

		valueStr << " ";

		DWORD memberTypeID;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			memberID,
			TI_GET_TYPEID,
			&memberTypeID);

		valueStr << getTypeName(memberTypeID, modBase);

		if (memberTag == SymTagData) 
		{
			WCHAR* name;
			SymGetTypeInfo(
				debuggeehProcess,
				modBase,
				memberID,
				TI_GET_SYMNAME,
				&name);

			valueStr << "  " << name;

			LocalFree(name);
		}
		else {
			valueStr << "  <base-class>";
		}

		DWORD offset;
		SymGetTypeInfo(
			debuggeehProcess,
			modBase,
			memberID,
			TI_GET_OFFSET,
			&offset);

		DWORD childAddress = address + offset;

		if (isPODType(memberTypeID, modBase) == true) 
		{

			valueStr << TEXT("  ")
				<< getTypeValue(
				memberTypeID,
				modBase,
				childAddress,
				pData + offset);
		}

		return true;
	}
}