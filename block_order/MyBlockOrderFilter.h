#pragma once

#include <dbfilter.h>

#include "config.h"

/**
* Ӧ���ڿ�����(AcDbBlockReference)
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
    // ��id��ӵ�m_sentToTop��m_sentToBottom��. ͬʱȷ��idֻ����������һ��
    virtual Acad::ErrorStatus moveToTop(const AcDbObjectId& id);
    virtual Acad::ErrorStatus moveToBottom(const AcDbObjectId& id);

private:
    static Adesk::Int16 classVersion;

    AcDbObjectIdArray m_sentToTop;
    AcDbObjectIdArray m_sentToBottom;

};

