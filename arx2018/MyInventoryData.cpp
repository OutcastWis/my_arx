#include "pch.h"
#include "MyInventoryData.h"

#include <dbproxy.h>

ACRX_DXF_DEFINE_MEMBERS(MyInventoryData, AcDbObject,
    AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
    AcDbProxyObject::kNoOperation, MYINVENTORYDATA,
    Extension Dictionary Inventory Id);



MyInventoryData::MyInventoryData() : mId(nullptr)
{
}

MyInventoryData::~MyInventoryData()
{
    delete mId;
    mId = nullptr;
}

const TCHAR* MyInventoryData::inventoryId() const
{
    assertReadEnabled();
    return mId;
}

void MyInventoryData::setInventoryId(const TCHAR* inventoryId)
{
    assertWriteEnabled();
    delete mId;

    mId = inventoryId ? _tcsdup(inventoryId) : nullptr;
}

Acad::ErrorStatus MyInventoryData::dwgInFields(AcDbDwgFiler* filer)
{
    assertWriteEnabled();

    Acad::ErrorStatus es = AcDbObject::dwgInFields(filer);
    assert(es == Acad::eOk);

    setInventoryId(nullptr);
    es = filer->readString(&mId); // readString 自动开辟空间
    assert(es == Acad::eOk);

    return es;
}

Acad::ErrorStatus MyInventoryData::dwgOutFields(AcDbDwgFiler* filer) const
{
    assertReadEnabled();
    Acad::ErrorStatus es = AcDbObject::dwgOutFields(filer);
    assert(es == Acad::eOk);

    es = filer->writeItem(mId);
    assert(es == Acad::eOk);

    return es;
}

Acad::ErrorStatus MyInventoryData::dxfInFields(AcDbDxfFiler* filer)
{
    assertWriteEnabled();
    Acad::ErrorStatus es = AcDbObject::dxfInFields(filer);
    assert(es == Acad::eOk);

    if (!filer->atSubclassData(_T("MyInventoryData")))
        return filer->filerStatus();

    resbuf rb;
    es = filer->readResBuf(&rb);

    if (es == Acad::eOk && rb.restype == AcDb::kDxfXTextString)
        setInventoryId(rb.resval.rstring);
    else {
        // unrecognized group. Push it back so that the subclass
        // can read it again.
        filer->pushBackItem();
        es = Acad::eBadDxfSequence;
    }

    return es;
}

Acad::ErrorStatus MyInventoryData::dxfOutFields(AcDbDxfFiler* filer) const
{
    assertReadEnabled();
    if (Acad::eOk != AcDbObject::dxfOutFields(filer))
        return filer->filerStatus();
   
    filer->writeItem(AcDb::kDxfSubclass, _T("MyInventoryData"));
    filer->writeItem(AcDb::kDxfXTextString, mId);

    return filer->filerStatus();
}
