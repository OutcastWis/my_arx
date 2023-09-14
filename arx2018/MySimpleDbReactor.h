#pragma once

#include <map>
#include <functional>

#include <dbmain.h>


class MySimpleDbReactor : public AcDbDatabaseReactor
{
public:
	// override
	virtual void objectModified(const AcDbDatabase* pDb, const AcDbObject* pDbObj);
	virtual void objectErased(const AcDbDatabase* pDb, const AcDbObject* pDbObj,
		Adesk::Boolean bErased = Adesk::kTrue);

public:

	// ��ͬoverride��Ӧ��ͬ�Ĳ�������. first������override�ĺ�����. second�ǲ�������, 
	// const TCHAR*��Ϊ��Ϣ, void* ��ʾ�Զ�������
	std::map < CString, std::function<void(const TCHAR*, void *)>> ops_;
};

