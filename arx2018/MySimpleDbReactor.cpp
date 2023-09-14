#include "pch.h"
#include "MySimpleDbReactor.h"

#include <tuple>

#include <axlock.h>

void MySimpleDbReactor::objectModified(const AcDbDatabase* pDb, const AcDbObject* pDbObj)
{
    AcAxDocLock docLock(pDbObj->ownerId(), AcAxDocLock::kNormal);
    if (docLock.lockStatus() != Acad::eOk)
        return;

    if (ops_.find(_T("objectModified")) != ops_.end()) {
        std::pair< const AcDbDatabase*, const AcDbObject* > data = std::make_pair(pDb, pDbObj);
        ops_[_T("objectModified")](_T(""), &data);
    }
}

void MySimpleDbReactor::objectErased(const AcDbDatabase* pDb, const AcDbObject* pDbObj, Adesk::Boolean bErased)
{
    AcAxDocLock docLock(pDbObj->ownerId(), AcAxDocLock::kNormal);
    if (docLock.lockStatus() != Acad::eOk)
        return;

    if (ops_.find(_T("objectErased")) != ops_.end())
    {
        typedef std::tuple<const AcDbDatabase* , const AcDbObject* , Adesk::Boolean>  td_type;
        td_type data{pDb, pDbObj, bErased};
        ops_[_T("objectErased")](_T(""), &data);
    }
}
