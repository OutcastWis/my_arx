#include "pch.h"
#include "MyBlockOrderFilter.h"

#include "MyBlockOrderIndex.h"
#include "MyBlockOrderIterator.h"

ACRX_DXF_DEFINE_MEMBERS(MyBlockOrderFilter, AcDbFilter,
    AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 0, 
    MYBLOCKORDERFILTER, MyBlockOrder);

//classes may change their version without changing the dwg version
//so we need a seperate version number
Adesk::Int16 MyBlockOrderFilter::classVersion = 1;

AcRxClass* MyBlockOrderFilter::indexClass() const
{
    assertReadEnabled();
    return MyBlockOrderIndex::desc();
}

Acad::ErrorStatus MyBlockOrderFilter::dwgInFields(AcDbDwgFiler* pFiler)
{
    assertWriteEnabled();
    Acad::ErrorStatus es;
    if ((es = AcDbFilter::dwgInFields(pFiler)) != Acad::eOk)
        return es;

    //read the version of this piece of persistent data
    Adesk::Int16 version;
    pFiler->readInt16(&version);
    if (version > classVersion)
        return Acad::eMakeMeProxy;//it was saved by a newer version of this class

    //remove old items
    m_sentToTop.removeAll();
    m_sentToBottom.removeAll();
    //read the new ones
    Adesk::UInt32 len;
    pFiler->readUInt32(&len);
    for (Adesk::UInt32 i = 0; i < len; ++i) {
        AcDbSoftPointerId id;
        pFiler->readSoftPointerId(&id);
        //this is where we get rid of non translated (erased, not copied etc.) objects
        if (!id.isNull())
            m_sentToTop.append(id);
    }
    pFiler->readUInt32(&len);
    for (Adesk::UInt32 i = 0; i < len; i++)
    {
        AcDbSoftPointerId id;
        pFiler->readSoftPointerId(&id);
        //this is where we get rid of non translated (erased, not copied etc.) objects
        if (!id.isNull())
            m_sentToBottom.append(id);
    }
    return pFiler->filerStatus();
    return Acad::ErrorStatus();
}

Acad::ErrorStatus MyBlockOrderFilter::dwgOutFields(AcDbDwgFiler* pFiler) const
{
    assertReadEnabled();
    Acad::ErrorStatus es;
    if ((es = AcDbFilter::dwgOutFields(pFiler)) != Acad::eOk)
        return es;

    //write class version
    pFiler->writeInt16(classVersion);
    pFiler->writeUInt32(m_sentToTop.length());
    int i;
    for (i = 0; i < m_sentToTop.length(); i++)
        pFiler->writeSoftPointerId(m_sentToTop[i]);
    pFiler->writeUInt32(m_sentToBottom.length());
    for (i = 0; i < m_sentToBottom.length(); i++)
        pFiler->writeSoftPointerId(m_sentToBottom[i]);
    return pFiler->filerStatus();
}

Acad::ErrorStatus MyBlockOrderFilter::moveToTop(const AcDbObjectId& id)
{
    assertWriteEnabled();
    int pos;
    //make sure it does not live in any of the arrays yet
    if (m_sentToTop.find(id, pos))
        m_sentToTop.removeAt(pos);
    if (m_sentToBottom.find(id, pos))
        m_sentToBottom.removeAt(pos);
    //add it
    m_sentToTop.append(id);

    return Acad::eOk;
}

Acad::ErrorStatus MyBlockOrderFilter::moveToBottom(const AcDbObjectId& id)
{
    assertWriteEnabled();
    int pos;
    //make sure it does not live in any of the arrays yet
    if (m_sentToTop.find(id, pos))
        m_sentToTop.removeAt(pos);
    if (m_sentToBottom.find(id, pos))
        m_sentToBottom.removeAt(pos);
    //add it
    m_sentToBottom.append(id);
    return Acad::eOk;
}
