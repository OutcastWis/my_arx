#include "pch.h"
#include "block_order.h"

#include <rxregsvc.h>
#include <dbtrans.h>
#include <actrans.h>
#include <dbindex.h>
#include <acestext.h>


#include "../block_order/MyBlockOrderIndex.h"
#include "../block_order/MyBlockOrderFilter.h"

namespace wzj {

    namespace detail {
        struct transaction {
            transaction()
            {
                AcTransaction* pt = actrTransactionManager->startTransaction();
                assert(NULL != pt);
            }

            ~transaction() {
                commit_ ? actrTransactionManager->endTransaction() : actrTransactionManager->abortTransaction();
            }

            void commit() { commit_ = true; }

            bool commit_ = false;
        };
        // 原始顺序, 即使用AcDbBlockTableRecordIterator
        void normal_order(const AcDbObjectPointer<AcDbBlockReference>& pBlkRef, const AcDbBlockTableRecordPointer& pBTR) {
            AcDbBlockTableRecordIterator* pIter = nullptr;
            CString info = _T("normal order = [");
            if (pBTR->newIterator(pIter) == Acad::eOk) {
                for (pIter->start(); !pIter->done(); pIter->step())
                {
                    AcDbObjectId cur_id;
                    if (pIter->getEntityId(cur_id) == Acad::eOk) {
                        TCHAR hdl[AcDbHandle::kStrSiz] = {};
                        cur_id.handle().getIntoAsciiBuffer(hdl);
                        info += hdl;
                        info += _T(", ");
                    }
                }
                delete pIter;
            }
            info += _T("]\n");
            ads_printf((const TCHAR*)info);
        }
        // 依赖MyBlockOrderFilter的顺序. 返回false表示没有MyBlockOrderFilter
        bool my_order(const AcDbObjectPointer<AcDbBlockReference>& pBlkRef, const AcDbBlockTableRecordPointer& pBTR) {
            MyBlockOrderIndex* pMyIdx = nullptr;
            AcDbObjectPointer<MyBlockOrderIndex> pIndex;
            AcDbIndexFilterManager::getIndex(pBTR, MyBlockOrderIndex::desc(), AcDb::kForRead, (AcDbIndex*&)pMyIdx);
            if (pIndex.acquire(pMyIdx) != Acad::eOk)
                return false;

            MyBlockOrderFilter* pMyFilter = nullptr;
            AcDbObjectPointer<MyBlockOrderFilter> pFilter;
            AcDbIndexFilterManager::getFilter(pBlkRef, MyBlockOrderFilter::desc(), AcDb::kForRead, (AcDbFilter*&)pMyFilter);
            if (pFilter.acquire(pMyFilter) != Acad::eOk)
                return false;

            AcDbFilteredBlockIterator* iter = pIndex->newIterator(pFilter);
            CString info = _T("my order = [");
            for (iter->start(); iter->id() != AcDbObjectId::kNull; iter->next()) {
                AcDbObjectId cur_id = iter->id();
                TCHAR hdl[AcDbHandle::kStrSiz] = {};
                cur_id.handle().getIntoAsciiBuffer(hdl);
                info += hdl;
                info += _T(", ");
            }
            info += _T("]\n");
            ads_printf((const TCHAR*)info);
            delete iter;

            return true;
        }
        /**
        * 对第一层内嵌的对象(可以是普通对象, 也可以是内嵌的block reference)进行向前或向后排序
        */
        void block_order() {
            resbuf* res = nullptr;
            try {
                transaction trans;
                ads_name ename = {};
                ads_point pt = {};
                int rt = ads_entsel(_T("Select a reference:"), ename, pt);
                if (rt == RTCAN)
                    return;
                if (rt != RTNORM)
                    throw Acad::eInvalidInput;

                AcDbObjectId id;
                acdbGetObjectId(id, ename);
                AcDbObjectPointer<AcDbBlockReference> pBlkRef(id, AcDb::kForWrite);
                AcDbBlockTableRecordPointer pBTR(pBlkRef->blockTableRecord(), AcDb::kForWrite);
                //get or add index
                MyBlockOrderIndex* pMyIdx;
                Acad::ErrorStatus es = AcDbIndexFilterManager::getIndex(pBTR, MyBlockOrderIndex::desc(), AcDb::kForWrite, (AcDbIndex*&)pMyIdx);
                if (es == Acad::eKeyNotFound) {
                    pMyIdx = MyBlockOrderIndex::cast(MyBlockOrderIndex::desc()->create());
                    AcDbIndexFilterManager::addIndex(pBTR, pMyIdx);
                }
                else if (es != Acad::eOk)
                    throw es;
                AcDbObjectPointer<MyBlockOrderIndex> pIndex;
                if (pIndex.acquire(pMyIdx) == Acad::eNullObjectPointer)
                    throw Acad::eOutOfMemory;
                //get or add filter
                MyBlockOrderFilter* pMyFilter;
                es = AcDbIndexFilterManager::getFilter(pBlkRef, MyBlockOrderFilter::desc(), AcDb::kForWrite, (AcDbFilter*&)pMyFilter);
                if (es == Acad::eKeyNotFound)
                {
                    pMyFilter = MyBlockOrderFilter::cast(MyBlockOrderFilter::desc()->create());
                    AcDbIndexFilterManager::addFilter(pBlkRef, pMyFilter);
                }
                else if (es != Acad::eOk)
                    throw es;
                AcDbObjectPointer<MyBlockOrderFilter> pFilter;
                if (pFilter.acquire(pMyFilter) == Acad::eNullObjectPointer)
                    throw Acad::eOutOfMemory;
                //prompt for object to be moved
                ads_point xform[4] = {};
                resbuf* temp = nullptr, * last = nullptr, * lastButOne = nullptr;
                ads_name eobj = {};
                do
                {
                    if (res != NULL)
                        acutRelRb(res);
                    res = NULL; temp = NULL; last = NULL; lastButOne = NULL;
                    if ((rt = acedNEntSel(_T("\nSelect entity to order:"), eobj, pt, xform, &res)) == RTCAN)
                        return;
                    if (rt != RTNORM)
                        throw Acad::eInvalidInput;
                    temp = res;
                    last = temp;
                    while (temp != NULL && temp->rbnext != NULL) {
                        lastButOne = temp;
                        temp = temp->rbnext;
                        last = temp;
                    }
                } //prompt until we find an object within the original block reference
                while (res == NULL || last == NULL || (last != NULL && last->resval.rlname[0] != ename[0]));

                AcDbObjectId idObj;
                //if the object selected is within a block reference (nested block)
                //then just add the block reference (NOTE that eobj is the inner most object
                //and we only sort objects within the this reference.)
                if (lastButOne != NULL)
                    acdbGetObjectId(idObj, lastButOne->resval.rlname);
                else
                    acdbGetObjectId(idObj, eobj);
                acutRelRb(res);

                if (acedInitGet(0, _T("TOP,T BOTTOM,B")) != RTNORM)
                    throw Acad::eInvalidInput;
                TCHAR result[512] = {};
                if ((rt = acedGetKword(_T("\nMove to [Top/Bottom]:"), result)) == RTCAN)
                    return;
                if (rt != RTNORM)
                    throw Acad::eInvalidInput;

                TCHAR hdl[AcDbHandle::kStrSiz] = {};
                idObj.handle().getIntoAsciiBuffer(hdl);
                if (_tcsicmp(result, _T("TOP")) == 0) {
                    pFilter->moveToTop(idObj);
                    ads_printf(_T("move to TOP: %s\n"), hdl);
                }
                else if (_tcsicmp(result, _T("BOTTOM")) == 0) {
                    pFilter->moveToBottom(idObj);
                    ads_printf(_T("move to BOTTOM: %s\n"), hdl);
                }
                pIndex->setLastUpdatedAtU(pBTR->database()->tduupdate());
                //make sure that the reference is redrawn
                pBlkRef->assertWriteEnabled();
                trans.commit();
            }
            catch (Acad::ErrorStatus es)
            {
                if (res != NULL)
                    acutRelRb(res);
                acutPrintf(_T("\nError: es==%s"), acadErrorStatusText(es));
            }
        }
        /**
        * 查看当前对象的循序
        */
        void see_order() {
            ads_point xform[4] = {}, pt = {};
            ads_name eobj = {};
            resbuf* res = nullptr;
            do {
                auto rt = acedNEntSel(_T("\nSelect a block reference to see its order:"), eobj, pt, xform, &res);
                if (rt != RTNORM)
                    return;

            } while (res == nullptr);
            // 获取最外层block reference
            resbuf* last = res;
            while (last != NULL && last->rbnext != NULL) {
                last = last->rbnext;
            }

            AcDbObjectId id;
            acdbGetObjectId(id, last->resval.rlname);
            acutRelRb(res);

            AcDbObjectPointer<AcDbBlockReference> pBlkRef(id, AcDb::kForRead);
            if (pBlkRef.openStatus() != Acad::eOk) {
                acutPrintf(_T("\nOpen block reference failed. Error: es==%s"), acadErrorStatusText(pBlkRef.openStatus()));
                return;
            }
            AcDbBlockTableRecordPointer pBTR(pBlkRef->blockTableRecord(), AcDb::kForRead);

            if (!my_order(pBlkRef, pBTR))
                normal_order(pBlkRef, pBTR);
        }
    }

    void block_order::init_impl() {
        acrxLoadModule(_T("block_order.dbx"), true);

        acedRegCmds->addCommand(_T("MY_COMMAND_BORDER"), _T("GLOBAL_BORDER"), _T("LOCAL_BORDER"), ACRX_CMD_MODAL, &detail::block_order);

        acedRegCmds->addCommand(_T("MY_COMMAND_BORDER"), _T("GLOBAL_SORDER"), _T("LOCAL_SORDER"), ACRX_CMD_MODAL, &detail::see_order);
    }


    void block_order::stop_impl() {
        acrxUnloadModule(_T("block_order.dbx"));

        acedRegCmds->removeGroup(_T("MY_COMMAND_BORDER"));
    }
}