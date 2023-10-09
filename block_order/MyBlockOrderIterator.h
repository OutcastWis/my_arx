#pragma once

#include <dbindex.h>
#include "MyBlockOrderFilter.h"

/**
* @remark ����û�м̳���AcDbObject. �Ҳ���Ҫ��dll���⿪��
*/
class MyBlockOrderIterator : public AcDbFilteredBlockIterator
{
public:
    MyBlockOrderIterator(const MyBlockOrderFilter* pFilter, const AcDbObjectId& idBTR);
    virtual ~MyBlockOrderIterator() = default;

    // override
    virtual Acad::ErrorStatus start() override;
    virtual AcDbObjectId      next() override;
    virtual AcDbObjectId      id() const override;
    virtual Acad::ErrorStatus seek(AcDbObjectId id) override;

    virtual double estimatedHitFraction() const override;
    virtual Acad::ErrorStatus accepts(AcDbObjectId id, Adesk::Boolean& idPassesFilter) const override;

    virtual Adesk::Boolean    buffersForComposition() const override;
    // id����mBuffer
    virtual Acad::ErrorStatus addToBuffer(AcDbObjectId id) override;

private:
    const MyBlockOrderFilter* mpFilter;     // ������mIds�е�����
    AcDbObjectId midBTR;        // ָ��AcDbBlockTableRecord
    AcDbObjectIdArray mIds;     // �����Ķ���. �ü��Ϻ�mBufferһ��, ֻ����˳��һ
    AcDbObjectIdArray mBuffer;  // ����midBTR���е����ж���. �洢˳��ʹ��AcDbBlockTableRecordIterator
    int mPos;                   // ��ǰ������mIds���ĸ�λ��. ����������
    bool mbMain;                // �Ƿ�δ��ʼ��, false��ʾ�ѳ�ʼ��. ��ʼ����ʱ��, ��Ҫ��midBTR�ж�ȡmBuffer.
};

