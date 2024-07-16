#pragma once

#include <dbindex.h>
#include "MyBlockOrderFilter.h"

/**
* @remark 该类没有继承自AcDbObject. 且不需要跨dll对外开放
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
    // id加入mBuffer
    virtual Acad::ErrorStatus addToBuffer(AcDbObjectId id) override;

private:
    const MyBlockOrderFilter* mpFilter;     // 决定了mIds中的排序
    AcDbObjectId midBTR;        // 指向AcDbBlockTableRecord
    AcDbObjectIdArray mIds;     // 排序后的对象. 该集合和mBuffer一致, 只不过顺序不一
    AcDbObjectIdArray mBuffer;  // 存贮midBTR块中的所有对象. 存储顺序使用AcDbBlockTableRecordIterator
    int mPos;                   // 当前遍历到mIds的哪个位置. 即数组索引
    bool mbMain;                // 是否未初始化, false表示已初始化. 初始化的时候, 需要从midBTR中读取mBuffer.
};

