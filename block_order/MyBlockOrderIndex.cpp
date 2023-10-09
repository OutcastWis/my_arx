#include "pch.h"
#include "MyBlockOrderIndex.h"


ACRX_DXF_DEFINE_MEMBERS(MyBlockOrderIndex, AcDbIndex, 
    AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 0, 
    MYBLOCKORDERINDEX, MyBlockOrder);

//classes may change their version without changing the dwg version
//so we need a seperate version number
Adesk::Int16 MyBlockOrderIndex::classVersion = 1;

AcDbFilteredBlockIterator* MyBlockOrderIndex::newIterator(const AcDbFilter* pFilter) const
{
    assertReadEnabled();
    if (!pFilter->isKindOf(MyBlockOrderFilter::desc()))
        return nullptr;

    return new MyBlockOrderIterator(MyBlockOrderFilter::cast(pFilter), objectBeingIndexedId());
}

Acad::ErrorStatus MyBlockOrderIndex::rebuildFull(AcDbIndexUpdateData* pIdxData)
{
    //there's nothing to rebuild
    assertWriteEnabled();
    return Acad::eOk;
}

Acad::ErrorStatus MyBlockOrderIndex::dwgInFields(AcDbDwgFiler* pFiler)
{
    assertWriteEnabled();
    Acad::ErrorStatus es;
    if ((es = AcDbIndex::dwgInFields(pFiler)) != Acad::eOk)
        return es;

    //read the version of this piece of persistent data
    Adesk::Int16 version;
    pFiler->readInt16(&version);
    if (version > classVersion)
        return Acad::eMakeMeProxy;//it was saved by a newer version of this class

    return pFiler->filerStatus();
}

Acad::ErrorStatus MyBlockOrderIndex::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();
    Acad::ErrorStatus es;
    if ((es = AcDbIndex::dwgOutFields(pFiler)) != Acad::eOk)
        return es;
    //write class version
    pFiler->writeInt16(classVersion);
    return pFiler->filerStatus();
}

Acad::ErrorStatus MyBlockOrderIndex::rebuildModified(AcDbBlockChangeIterator* iter)
{
    //there's nothing to rebuild
    assertWriteEnabled();
    return Acad::eOk;
}
