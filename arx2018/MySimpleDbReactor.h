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

	// 不同override对应不同的操作函数. first是上述override的函数名. second是操作函数, 
	// const TCHAR*作为信息, void* 表示自定义数据
	std::map < CString, std::function<void(const TCHAR*, void *)>> ops_;
};

