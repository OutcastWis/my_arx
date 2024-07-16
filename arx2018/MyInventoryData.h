#pragma once

#include <tchar.h>
#include <dbmain.h>

/**
* @file MyInventoryData
* 自定义数据, 配合对象的命名字典使用
*/
class MyInventoryData : public AcDbObject
{
public:
    ACRX_DECLARE_MEMBERS(MyInventoryData);

    MyInventoryData();
    virtual ~MyInventoryData();

    const TCHAR* inventoryId() const;
    void setInventoryId(const TCHAR* inventoryId);

    // Overridden methods from AcDbObject
    Acad::ErrorStatus   dwgInFields(AcDbDwgFiler* filer) override;
    Acad::ErrorStatus   dwgOutFields(AcDbDwgFiler* filer) const override;

    Acad::ErrorStatus   dxfInFields(AcDbDxfFiler* filer) override;
    Acad::ErrorStatus   dxfOutFields(AcDbDxfFiler* filer) const override;

private:
    TCHAR* mId;
};

