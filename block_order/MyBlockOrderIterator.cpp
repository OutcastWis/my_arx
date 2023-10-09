#include "pch.h"
#include "MyBlockOrderIterator.h"

#include <dbsymtb.h>
#include <dbobjptr.h> 

MyBlockOrderIterator::MyBlockOrderIterator(const MyBlockOrderFilter* pFilter, const AcDbObjectId& idBTR)
{
    mpFilter = pFilter;
    midBTR = idBTR;
    mbMain = true;
}

Acad::ErrorStatus MyBlockOrderIterator::start()
{
    mPos = 0;
    AcDbObjectId id;
    if (mbMain) {
        mbMain = false;

        AcDbBlockTableRecordPointer pBTR;
        if (pBTR.open(midBTR, AcDb::kForRead) == Acad::eOk) {
            AcDbBlockTableRecordIterator* pIter;
            if (pBTR->newIterator(pIter) == Acad::eOk) {
                for (pIter->start(); !pIter->done(); pIter->step())
                {
                    if (pIter->getEntityId(id) == Acad::eOk)
                        mBuffer.append(id);
                }
                delete pIter;
            }
        }
    }

    if (mBuffer.length() == 0)
        return Acad::eOk;

    // fill in the objects that were 'sent to top'
    auto pOrder = &(mpFilter->sentToTop());
    for (int i = 0; i < pOrder->length(); i++)
    {
        int pos;
        if (mBuffer.find(pOrder->at(i), pos))
        {
            mIds.append(pOrder->at(i));
            mBuffer.removeAt(pos);
        }
    }
    // note where we stop with 'sent back' items
    int bottomHalf = mIds.length();
    // ·´Ïò
    pOrder = &(mpFilter->sentToBottom());
    for (int i = pOrder->length() - 1; i >= 0; --i) {
        int pos;
        if (mBuffer.find(pOrder->at(i), pos))
        {
            mIds.append(pOrder->at(i));
            mBuffer.removeAt(pos);
        }
    }

    //insert the rest in between
    //this is not reverse order just that's how insertAt works
    for (int i = mBuffer.length() - 1; i >= 0; i--)
    {
        mIds.insertAt(bottomHalf, mBuffer[i]);
    }

    return Acad::eOk;
}

AcDbObjectId MyBlockOrderIterator::next()
{
    AcDbObjectId temp = id();
    if (mIds.length() > mPos)
        mPos++;
    return temp;
}

AcDbObjectId MyBlockOrderIterator::id() const
{
    if (mIds.length() > mPos)
        return mIds[mPos];
    return AcDbObjectId::kNull;
}

Acad::ErrorStatus MyBlockOrderIterator::seek(AcDbObjectId id)
{
    int temp;
    if (mIds.find(id, temp))
    {
        mPos = temp;
        return Acad::eOk;
    }
    return Acad::eKeyNotFound;
}

double MyBlockOrderIterator::estimatedHitFraction() const
{
    // so that other iterators are candidates for the main one before
    // this
    return 1.0;
}

Acad::ErrorStatus MyBlockOrderIterator::accepts(AcDbObjectId id, Adesk::Boolean& idPassesFilter) const
{
    //this is a buffering iterator so this is not applicable
    return Acad::eNotApplicable;
}

Adesk::Boolean MyBlockOrderIterator::buffersForComposition() const
{
    //set this flag to false because we are not the main iterator
    const_cast<MyBlockOrderIterator*>(this)->mbMain = false;
    return Adesk::kTrue;
}

Acad::ErrorStatus MyBlockOrderIterator::addToBuffer(AcDbObjectId id)
{
    //if we are not the main iterator then this function will be called
    //repeatedly with the results of the previous iterator
    mBuffer.append(id);
    return Acad::eOk;
}

