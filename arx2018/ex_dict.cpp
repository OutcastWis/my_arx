#include "pch.h"
#include "ex_dict.h"

#include "MyInventoryData.h"

namespace wzj {

    namespace detail {

        static const TCHAR* key = _T("inventory_id");

        AcDbObject* selectObject(AcDbObjectId& eId, AcDb::OpenMode openMode) {
            ads_name en = {};
            ads_point pt = {};
            ads_initget(RSG_OTHER, _T("Handle _Handle")); // RSG_OTHER允许用户输入任意值
            int ss = ads_entsel(_T("Select an Entity or enter 'H' to any enter object handle: "), en, pt);

            TCHAR hdl[16] = {};
            switch (ss)
            {
            case RTNORM:
                break;
            case RTKWORD:
                if ((ads_getstring(FALSE, _T("Enter Valid Object Handle: "), hdl) == RTNORM) && (ads_handent(hdl, en) == RTNORM))
                    break; // FALSE表示输入字符串可以包含空格. 否则空格会打断输入
            default:
                ads_printf(_T("Nothing Selected, Return Code=%d\n"), ss);
                return nullptr;
            }

            auto es = acdbGetObjectId(eId, en);
            if (es != Acad::eOk) {
                acutPrintf(_T("\nacdbGetObjectId failed"));
                acutPrintf(_T("\nen==(%lx,%lx), retStat==%d\n"), en[0], en[1], eId);
                return NULL;
            }

            AcDbObject* pObj;
            if ((es = acdbOpenObject(pObj, eId, openMode)) != Acad::eOk) {
                acutPrintf(_T("\nacdbOpenEntity failed: ename=(%lx,%lx), mode==%d")
                    _T(" retStat==%d\n"), en[0], en[1], openMode, es);
                return NULL;
            }
            return pObj;
        }

        /**
        * 选择对象, 然后打开它
        * @param [out] pObjId, 所选对象id
        * @param [in] mode, 打开方式. pObj, pXDict, pIObj都是用该方式打开
        * @param [out] pObj, 所选对象
        * @param [out] pXDict, 所选对象的扩展字典
        * @param [out] pIObj, pXDict中的"inventory_id"对应的存储对象
        */
        void commonOpen(AcDbObjectId& pObjId, AcDb::OpenMode  mode, AcDbObject*& pObj, AcDbDictionary*& pXDict,
                MyInventoryData*& pIObj)
        {
            // Get the main object.
            pObj = NULL;
            pObjId = AcDbObjectId::kNull;
            if ((pObj = selectObject(pObjId, mode)) == NULL)
                return;

            // Get extension dictionary object Id
            pXDict = NULL;
            AcDbObjectId xDictId = pObj->extensionDictionary();

            // If there is no dictionary to be opened,
            // then there are no relevant contents.
            if (xDictId == AcDbObjectId::kNull) {
                if (mode != AcDb::kForWrite)
                    return;

                // This next step should never fail.
                if (pObj->createExtensionDictionary() != Acad::eOk)
                    return;

                // This next step should also never fail.
                if ((xDictId = pObj->extensionDictionary()) == AcDbObjectId::kNull)
                    return;
            }

            // Open the dictionary, if we can.
            //
            if (acdbOpenObject(pXDict, xDictId, mode) != Acad::eOk)
                return;

            // Now, get and open the inventory object, if it is there.
            // If we can't open it, that's OK for now.
            //
            pIObj = NULL;
            pXDict->getAt(key, ((AcDbObject*&)pIObj), mode);
        }

        // 选择对象,显示该对象扩展字典中, "inventory_id"对应的id
        void listInventoryId() {
            AcDbObjectId pObjId;
            AcDbObject* pObj = NULL;
            AcDbDictionary* xDict = NULL;
            MyInventoryData* iObj = NULL;

            commonOpen(pObjId, AcDb::kForRead, pObj, xDict, iObj);

            // No Object or no extension dictionary means
            // there is nothing to do.
            if (pObj == NULL)
                return;

            const TCHAR* idStr = NULL;
            if (iObj != NULL)
                idStr = iObj->inventoryId();

            if (idStr == NULL)
                acutPrintf(_T("\nNo Inventory Id for selected object.\n"));
            else {
                acutPrintf(_T("\nInventory Id for selected object is: %s\n"), idStr);
            }

            if (iObj != NULL)
                iObj->close();
            if (xDict != NULL)
                xDict->close();
            if (pObj != NULL)
                pObj->close();
        }
        // 选择对象, 为该对象的扩展字典添加{"inventory_id", value}
        void setInventoryId() {
            AcDbObjectId pObjId;
            AcDbObject* pObj = NULL;
            AcDbDictionary* pXDict = NULL;
            MyInventoryData* pIObj = NULL;

            commonOpen(pObjId, AcDb::kForWrite, pObj, pXDict, pIObj);

            if (pObj == nullptr)
                return;
            if (pXDict == nullptr) {
                pXDict->close();
                return;
            }

            TCHAR inventoryId[255] = {};
            if (ads_getstring(FALSE, _T("Enter Inventory Id: "), inventoryId) != RTNORM) {
                pXDict->close();
                pObj->close();
                if (pIObj)
                    pIObj->close();
                return;
            }

            if (pIObj == NULL) {
                AcDbObjectId iObjId;
                pIObj = new MyInventoryData;
                if (pXDict->setAt(key, pIObj, iObjId) != Acad::eOk) {
                    delete pIObj;
                    pXDict->close();
                    pObj->close();
                    return;
                }
            }
            pIObj->setInventoryId(inventoryId);

            pIObj->close();
            pXDict->close();
            pObj->close();
        }
    }

    void ex_dict::init_impl() {
        acedRegCmds->addCommand(_T("WZJ_COMMAND_EX_DICT"), _T("GLOBAL_GET_ID"), _T("LOCAL_GET_ID"), ACRX_CMD_TRANSPARENT,
            detail::listInventoryId);

        acedRegCmds->addCommand(_T("WZJ_COMMAND_EX_DICT"),_T("LOCAL_GET_ID"),_T("LOCAL_SET_ID"),ACRX_CMD_TRANSPARENT,
            detail::setInventoryId);
    }

    void ex_dict::stop_impl() {
        acedRegCmds->removeGroup(ACRX_T("WZJ_COMMAND_EX_DICT"));
    }
}