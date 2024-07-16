#pragma once

#include <dbindex.h>

#include "config.h"
#include "MyBlockOrderIterator.h"


/**
* 应用于块定义(AcDbBlockTableRecord)
*/
class DLLIMPEXP  MyBlockOrderIndex : public AcDbIndex
{
public:
    ACRX_DECLARE_MEMBERS(MyBlockOrderIndex);
    virtual ~MyBlockOrderIndex() = default;
    // override
    virtual AcDbFilteredBlockIterator* newIterator(const AcDbFilter* pFilter) const override;
    virtual Acad::ErrorStatus rebuildFull(AcDbIndexUpdateData* pIdxData) override;
    virtual Acad::ErrorStatus dwgInFields(AcDbDwgFiler* pFiler) override;
    virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler* pFiler) const override;
protected:
    virtual Acad::ErrorStatus rebuildModified(AcDbBlockChangeIterator* iter) override;

private:
    static Adesk::Int16 classVersion;
};

