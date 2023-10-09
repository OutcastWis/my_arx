#pragma once

#include <dbfilter.h>

#include "config.h"

/**
* 应用于块引用(AcDbBlockReference)
*/
class DLLIMPEXP MyBlockOrderFilter : public AcDbFilter
{
public:
    ACRX_DECLARE_MEMBERS(MyBlockOrderFilter);
    virtual ~MyBlockOrderFilter() = default;
    // override
    virtual AcRxClass* indexClass() const override;
    virtual Acad::ErrorStatus dwgInFields(AcDbDwgFiler* pFiler) override;
    virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler* pFiler) const override;

public:
   
    //BlockOrder protocol declared virtual 
    virtual const AcDbObjectIdArray& sentToTop()const { assertReadEnabled(); return m_sentToTop; }
    virtual const AcDbObjectIdArray& sentToBottom()const { assertReadEnabled(); return m_sentToBottom; }
    // 将id添加到m_sentToTop或m_sentToBottom中. 同时确保id只会属于其中一个
    virtual Acad::ErrorStatus moveToTop(const AcDbObjectId& id);
    virtual Acad::ErrorStatus moveToBottom(const AcDbObjectId& id);

private:
    static Adesk::Int16 classVersion;

    AcDbObjectIdArray m_sentToTop;
    AcDbObjectIdArray m_sentToBottom;

};

