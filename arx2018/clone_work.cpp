#include "pch.h"
#include "clone_work.h"

#include <dbidmap.h>
#include <adui.h>
#include <acui.h>
#include <acuiDialog.h>

#include "command.h"
#include "MyInventoryData.h"

namespace wzj {

    namespace detail {
        const TCHAR g_key[] = _T("#note");

        CString tempDwgUrl() {
            return desktop_url() + _T("\\before_clone3.dwg");
        }

        /**
        * 用户选取选择集
        * @param ss [out], 获取的选择集
        * @param *length [out], 选择集长度
        * @return true表示非空选择集
        */
        bool select_sset(ads_name ss, Adesk::Int32* length) {
            if (RTNORM != acedSSGet(_T("_x"), NULL, NULL, NULL, ss))
            {
                acutPrintf(_T("acedSSGet cancelled\n"));
                return false;
            }
            
            *length = 0;
            if (RTNORM != acedSSLength(ss, length) || 0 == *length)
            {
                acutPrintf(_T("empty selection set\n"));
                acedSSFree(ss);
                return false;
            }

            return true;
        }

        AcDbObjectId get_block(const TCHAR* bname) {
            AcDbBlockTable* pBlockTable; 
            auto es = acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);
            if (es != Acad::eOk)
                return AcDbObjectId();

            AcDbObjectId ret;
            pBlockTable->getAt(bname, ret);
            pBlockTable->close();
            return ret;
        }

        // Copies the entire current space (either model- or paper space)
        // using AcDbObject::clone(). The cloned entities are inserted
        // into model space layer 'clone'. You might notice that calling clone() 
        // doesn't work for "complex entities" like Polylines or Blocks.
        void clone1() {
            ads_name ss = {};
            Adesk::Int32 length = 0;
            if (!select_sset(ss, &length))
                return;

            // 创建图层'clone', 否则不能设置对象所属图层
            create_layer(_T("clone"));

            ads_name cur = {};
            for (int i = 0; i < length; ++i) {
                AcDbObjectId id;
                if (RTNORM != ads_ssname(ss, i, cur) || Acad::eOk != acdbGetObjectId(id, cur))
                    continue;

                AcDbEntity* pEnt;
                if (acdbOpenObject(pEnt, id, AcDb::kForRead) != Acad::eOk)
                    continue;

                AcDbEntity* pClone = AcDbEntity::cast(pEnt->clone());
                if (pClone != nullptr) {
                    pClone->setLayer(_T("clone"));
                    AcDbObjectId cid;
                    add_to_model_space(cid, pClone);
                    pClone->close();
                }
                pEnt->close();
            }
            acedSSFree(ss);
        }
        // Copies the entire current space (either model- or paper space)
        // using AcDbDatabase::deepCloneObjects(). The cloned entities are
        // inserted into paper space. This version works well with
        // Polylines and Blocks.
        void clone2() {
            ads_name ss = {};
            Adesk::Int32 length = 0;
            if (!select_sset(ss, &length))
                return;

            AcDbObjectIdArray list;
            for (int i = 0; i < length; ++i) {
                ads_name cur = {};
                AcDbObjectId id;
                if (RTNORM != ads_ssname(ss, i, cur) || acdbGetObjectId(id, cur) != Acad::eOk)
                    continue;
                list.append(id);
            }
            ads_ssfree(ss);

            AcDbObjectId blockId = get_block(ACDB_PAPER_SPACE);
            if (!blockId.isNull()) {
                AcDbIdMapping id_map;
                auto es = acdbHostApplicationServices()->workingDatabase()->deepCloneObjects(list, blockId, id_map);
                if (es != Acad::eOk)
                    ads_printf(_T("Error on deepCloneObjects\n"));
            }
            else
                ads_printf(_T("Error in getting ModelSpace ID\n"));
        }
        // Copies an external drawing into the current model space.
        // This is the supported way: a combination of
        // AcDbDatabase::insert and AcDbDatabase::wblock.
        // First, we wblock the external database into a temporary
        // in-memory database. We can't wblock directly into the
        // current drawing, because wblock is destructive on the
        // target database.
        // Then we use insert to insert the temporary database into
        // the current drawing. We can't use insert to insert the
        // external drawing directly, because insert uses
        // cheapClone, which moves the objects from the source to
        // the target database.
        //
        // Using this approach, all the symbol table gets merged
        // correctly into the current drawing database.
        void clone3() {
            CAcUiFileDialog dlg(TRUE, _T("dwg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("AutoCAD Files (*.dwg)|*.dwg|All Files (*.*)|*.*||"));
            if (dlg.DoModal() != IDOK)
                return;

            CString fname = dlg.GetFileName();
            ads_printf(_T("Dealing with file: %s"), fname);

            AcDbDatabase extDatabase(Adesk::kFalse);
            if (Acad::eOk != extDatabase.readDwgFile(fname))
            {
                acedAlert(_T("Error reading DWG!"));
                return;
            }

            AcDbBlockTable* pBT= nullptr;
            if (Acad::eOk != extDatabase.getSymbolTable(pBT, AcDb::kForRead))
            {
                acedAlert(_T("Error getting BlockTable of DWG"));
                return;
            }

            AcDbBlockTableRecord* pBTR = nullptr;
            Acad::ErrorStatus es = pBT->getAt(ACDB_MODEL_SPACE, pBTR, AcDb::kForRead);
            pBT->close();
            if (Acad::eOk != es) {
                acedAlert(_T("Error getting model space of DWG"));
                return;
            }

            AcDbObjectIdArray list;
            AcDbBlockTableRecordIterator* pIT = nullptr;
            pBTR->newIterator(pIT);
            for (; !pIT->done(); pIT->step()) {
                AcDbObjectId id;
                pIT->getEntityId(id);
                list.append(id);

                // 以下BUG在2018中不存在. 所以可以不用这样处理. 2018中字典默认被其元素硬引用
                /*
                // There is a bug in ARX that causes extension dictionaries
                // to appear to be soft owners of their contents.  This causes
                // the contents to be skipped during wblock.  To fix this we
                // must explicitly tell the extension dictionary to be a hard
                // owner of it's entries.
                AcDbEntity* pEnt;
                if (pIT->getEntity(pEnt, AcDb::kForRead) == Acad::eOk) {
                    AcDbObjectId dictId;
                    if ((dictId = pEnt->extensionDictionary()) != AcDbObjectId::kNull) {
                        AcDbDictionary* pDict = nullptr;
                        acdbOpenObject(pDict, dictId, AcDb::kForWrite);
                        if (pDict) {
                            pDict->setTreatElementsAsHard(Adesk::kTrue);
                            pDict->close();
                        }
                    }
                    pEnt->close();
                }
                */
            }
            delete pIT;
            pBTR->close();

            if (list.isEmpty()) {
                acedAlert(_T("No entities found in model space of DWG"));
                return;
            }

            AcDbDatabase* pTempDb;
            if (Acad::eOk != extDatabase.wblock(pTempDb, list, AcGePoint3d::kOrigin))
            {
                acedAlert(_T("wblock failed!"));
                return;
            }
            if (Acad::eOk != acdbHostApplicationServices()->workingDatabase()
                ->insert(AcGeMatrix3d::kIdentity, pTempDb))
                acedAlert(_T("insert failed!"));
            delete pTempDb;
        }

        // 为clone3准备一个dwg文件, 保存在桌面\\before_clone3.dwg
        void before_clone3() {
            AcDbDatabase db(Adesk::kTrue);

            AcDbCircle* circle = new AcDbCircle(
                AcGePoint3d(100, 100, 100),
                AcGeVector3d::kZAxis,
                100
            );
            // 添加到db中才能使用扩展字典
            AcDbObjectId id;
            add_to_model_space(id, circle, &db);
            // 添加扩展字典
            auto es = circle->createExtensionDictionary();
            auto dictId = circle->extensionDictionary();
            circle->close();
            AcDbDictionary* dict = nullptr;
            if (acdbOpenObject(dict, dictId, AcDb::kForWrite) != Acad::eOk) {
                ads_printf(_T("Can't open extension dictionary"));
                return;
            }
            // 为扩展字典添加数据
            MyInventoryData* value = new MyInventoryData;
            CString str;
            CTime curTime = CTime::GetCurrentTime();
            str.Format(_T("wzj@inventory_id@%04d-%02d-%02d"), curTime.GetYear(), curTime.GetMonth(), curTime.GetDay());
            value->setInventoryId(str);
            dict->setAt(g_key, value, id);
            dict->close();
            value->close();
            
            //
            CString url = tempDwgUrl();
            db.saveAs(url);
            ads_printf(_T("File: %s. is saved\n"), url);
        }
    }


    void clone_work::init_impl() {
        acedRegCmds->addCommand(_T("MY_COMMAND_CLONE_WORK"), _T("GLOBAL_CLONE1"), _T("LOCAL_CLONE1"), ACRX_CMD_MODAL, &detail::clone1);

        acedRegCmds->addCommand(_T("MY_COMMAND_CLONE_WORK"), _T("GLOBAL_CLONE2"), _T("LOCAL_CLONE2"), ACRX_CMD_MODAL, &detail::clone2);

        acedRegCmds->addCommand(_T("MY_COMMAND_CLONE_WORK"), _T("GLOBAL_BEFORE_CLONE3"), _T("LOCAL_BEFORE_CLONE3"),
            ACRX_CMD_MODAL, &detail::before_clone3);

        acedRegCmds->addCommand(_T("MY_COMMAND_CLONE_WORK"), _T("GLOBAL_CLONE3"), _T("LOCAL_CLONE3"), ACRX_CMD_MODAL, &detail::clone3);
    }

    void clone_work::stop_impl() {
        acedRegCmds->removeGroup(_T("MY_COMMAND_CLONE_WORK"));
        // 移除临时文件
        if (_taccess(detail::tempDwgUrl(), 0) != -1)
            CFile::Remove(detail::tempDwgUrl());
    }
}