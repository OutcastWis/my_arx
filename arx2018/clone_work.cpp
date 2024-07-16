#include "pch.h"
#include "clone_work.h"

#include <dbidmap.h>
#include <adui.h>
#include <acui.h>
#include <acuiDialog.h>
#include <dbxrecrd.h>
#include <acestext.h>

#include "command.h"
#include "MyInventoryData.h"


namespace wzj {
    static clone_work* global_one = clone_work::instance();


    namespace detail {
        const TCHAR g_key[] = _T("#note");
        const TCHAR g_namedDict[] = _T("#dict");

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
        // 添加一个带有扩展字典的circle
        void add_a_circle(AcDbDatabase* db) {
            //
            AcDbCircle* circle = new AcDbCircle(
                AcGePoint3d(100, 100, 100),
                AcGeVector3d::kZAxis,
                100
            );
            // 添加到db中才能使用扩展字典
            AcDbObjectId id;
            add_to_model_space(id, circle, db);
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
        }
        // 添加一个命名字典, 名字是g_namedDict
        void add_a_named_dict(AcDbDatabase* db) {
            AcDbDictionary* pNamedobj;
            db->getNamedObjectsDictionary(pNamedobj, AcDb::kForRead);

            AcDbDictionary* pDict;
            if (!pNamedobj->has(g_namedDict)) {
                if (pNamedobj->upgradeOpen() == Acad::eOk) {
                    pDict = new AcDbDictionary;
                    AcDbObjectId id;
                    pNamedobj->setAt(g_namedDict, pDict, id);

                    // 添加数据
                    MyInventoryData* v1 = new MyInventoryData;
                    MyInventoryData* v2 = new MyInventoryData;
                    v1->setInventoryId(_T("value1"));
                    v2->setInventoryId(_T("value2"));
                    pDict->setAt(_T("key1"), v1, id);
                    pDict->setAt(_T("key2"), v2, id);

                    v1->close();
                    v2->close();
                    pDict->close();
                }
            }
            pNamedobj->close();
        }
        // Copies the entire current space (either model- or paper space)
        // using AcDbObject::clone(). The cloned entities are inserted
        // into model space layer 'clone'. You might notice that calling clone() 
        // doesn't work for "complex entities" like Polylines or Blocks.
        // 该函数成功运行后, 你可以图层clone中发现被拷贝的对象
        void clone1() {
            ads_name ss = {};
            Adesk::Int32 length = 0;
            if (!select_sset(ss, &length))
                return;

            // 创建图层'clone', 否则不能设置对象所属图层
            create_layer(_T("clone"), false, false);

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
        // 该函数成功运行后, 你可以在paper space中发现被拷贝对象
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
        // 为clone3准备一个dwg文件, 保存在桌面\\before_clone3.dwg
        void before_clone3() {
            AcDbDatabase db(Adesk::kTrue);

            add_a_circle(&db);
            add_a_named_dict(&db);

            //
            CString url = tempDwgUrl();
            db.saveAs(url);
            ads_printf(_T("File: %s. is saved\n"), url);
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
        // 该函数成功运行后, 你可以在当前图纸中看见和桌面\\before_clone3.dwg的模型空间一样的内容
        void clone3() {
            CAcUiFileDialog dlg(TRUE, _T("dwg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("AutoCAD Files (*.dwg)|*.dwg|All Files (*.*)|*.*||"));
            if (dlg.DoModal() != IDOK)
                return;

            CString fname = dlg.GetFileName();
            ads_printf(_T("Dealing with file: %s\n"), fname);

            AcDbDatabase extDatabase(Adesk::kFalse);
            if (Acad::eOk != extDatabase.readDwgFile(fname))
            {
                acedAlert(_T("Error reading DWG!"));
                return;
            }

            AcDbBlockTable* pBT = nullptr;
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
        // 和其他不同, 该函数可以只copy块定义, 而不生成块引用. 使用wblockCloneObjects
        void clone4() {
            CAcUiFileDialog dlg(TRUE, _T("dwg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("AutoCAD Files (*.dwg)|*.dwg|All Files (*.*)|*.*||"));
            if (dlg.DoModal() != IDOK)
                return;

            CString fname = dlg.GetFileName();
            ads_printf(_T("Dealing with file: %s\n"), fname);

            AcDbDatabase extDatabase(Adesk::kFalse);
            if (Acad::eOk != extDatabase.readDwgFile(fname))
            {
                acedAlert(_T("Error reading DWG!"));
                return;
            }

            AcDbBlockTable* pBT = nullptr;
            if (Acad::eOk != extDatabase.getSymbolTable(pBT, AcDb::kForRead))
            {
                acedAlert(_T("Error getting BlockTable of DWG"));
                return;
            }

            AcDbObjectIdArray list;
            AcDbBlockTableIterator* pBTIt;
            pBT->newIterator(pBTIt);
            for (; !pBTIt->done(); pBTIt->step()) {
                AcDbBlockTableRecord* r;
                if (pBTIt->getRecord(r) != Acad::eOk)
                    continue;
                TCHAR* name = nullptr;
                if (r->getName(name) != Acad::eOk ||
                    _tcscmp(name, ACDB_MODEL_SPACE) == 0 ||
                    _tcsncmp(name, ACDB_PAPER_SPACE, 12) == 0) {
                    r->close();
                    continue;
                }
                r->close();
                AcDbObjectId id;
                pBTIt->getRecordId(id);
                list.append(id);
            }
            pBT->close();
            delete pBTIt;

            if (list.isEmpty()) {
                acedAlert(_T("No entities found in model space of DWG"));
                return;
            }

            AcDbIdMapping mapping;
            AcDbBlockTable* pSelfBT = nullptr;
            if (Acad::eOk != acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pSelfBT, AcDb::kForWrite))
            {
                acedAlert(_T("Error getting BlockTable of Current DWG"));
                return;
            }
            AcDbObjectId btId = pSelfBT->objectId();
            pSelfBT->close();

            Acad::ErrorStatus es;
            if (Acad::eOk != (es = acdbHostApplicationServices()->workingDatabase()->wblockCloneObjects(list,
                btId, mapping, AcDb::kDrcReplace
            ))) {
                acedAlert(_T("wblockCloneObjects failed! "));
                ads_printf(_T("\n%s"), acadErrorStatusText(es));
                return;
            }
        }

        //
        void reactor() {
            if (getYorN(_T("Start reactor"))) {
                global_one->make_reactor();
                acutPrintf(_T("  Added reactor.\n"));
            }
            else {
                global_one->clear();
                acutPrintf(_T("  Removed reactor.\n"));
            }
        }
        // 响应WBLOCK和INSERT命令. 当不做处理时, WBLOCK和INSERT都不会对命名字典进行处理.
        // 如何操作:    1. 调用before_clone3生成需要的文件A
        //              2. 打开该文件A,  调用clone_reactor Y, 加载reactor
        //              3. 使用WBLOCK命令, 输出到文件B
        //              4. 检查文件B, 可以看到里面有g_namedDict的命名字典. 如无操作2, 则文件B中无g_namedDict命名字典
        // INSERT同理
        void beginDeepCloneXlation(void* data) {
            std::pair< AcDbIdMapping&, Acad::ErrorStatus*> *tmp = (std::pair< AcDbIdMapping&, Acad::ErrorStatus*>*)(data);
            AcDbIdMapping& idMap = tmp->first;
            Acad::ErrorStatus* pRetStat = tmp->second;

            auto ctx = idMap.deepCloneContext();
            if (ctx != AcDb::kDcWblock && ctx != AcDb::kDcInsert)
                return;

            // Get the "from" and "to" databases. 
            AcDbDatabase* pFrom, * pTo;
            idMap.origDb(pFrom);
            idMap.destDb(pTo);

            // See if the "from" database has our dictionary, and open it.  If it doesn't have one, we are done.
            AcDbDictionary* pSrcNamedObjDict;
            pFrom->getNamedObjectsDictionary(pSrcNamedObjDict, AcDb::kForRead);
            AcDbDictionary* pSrcDict;
            auto es = pSrcNamedObjDict->getAt(g_namedDict, pSrcDict, AcDb::kForRead);
            pSrcNamedObjDict->close();
            if (es == Acad::eKeyNotFound)
                return;
            
            AcDbObjectId dictId;
            AcDbObject* pClone;
            switch (ctx)
            {
            case AcDb::kDcWblock:
                // WBLOCK clones all, or part of a drawing into a
                // newly created drawing.  This means that the
                // NamedObject Dictionary is always cloned, and
                // its AcDbObjectIds are in flux.  Therefore, you
                // cannot use getAt() or setAt() on the dictionary
                // in the new database.  This is because the
                // cloned dictionary references all refer to the 
                // original objects.  During Deep Clone translation,
                // all cloned entries will be translated to the
                // new objects, and entries not cloned will be
                // "removed" by getting "translated" to NULL.
                //
                // The cloning of entries in our own dictionary are
                // not handled here.  If all are to be cloned, then
                // call setTreatElementsAsHard(Adesk::kTrue) on the
                // dictionary.  Otherwise, only those entries which
                // are refered to by hard references in other
                // wblocked objects, will have been cloned via
                // those references.

                // In this example, we will always write out all of
                // the records.  Since TreatElementsAsHard is not
                // currently persistent, we reset it here each time.
                pSrcDict->upgradeOpen();
                pSrcDict->setTreatElementsAsHard(Adesk::kTrue);
                pSrcDict->downgradeOpen();

                pClone = nullptr;
                es = pSrcDict->wblockClone(pTo, pClone, idMap, Adesk::kFalse);
                if (pClone != nullptr)
                    pClone->close();
                {
                    TCHAR hdl[17] = {};
                    pClone->ownerId().handle().getIntoAsciiBuffer(hdl);
                    ads_printf(_T("owner hdl = %s\n"), hdl);
                    pClone->id().handle().getIntoAsciiBuffer(hdl);
                    ads_printf(_T("self hdl = %s\n"), hdl);
                }
                break;
            case AcDb::kDcInsert:
            {
                // In INSERT, an entire drawing is cloned, and
                // "merged" into a pre-existing drawing.  This
                // means that the destination drawing may already
                // have our dictionary - in which case we have to
                // merge our entries into the destination
                // dictionary.   So, first we must find out if
                // the destination NamedObjects dictionary has
                // our dictionary.
                AcDbDictionary* pDestNamedDict;
                pTo->getNamedObjectsDictionary(pDestNamedDict, AcDb::kForWrite);
                // Since INSERT does not clone the destination
                // NamedObjects dictionary, we can use getAt()
                // on it.
                es = pDestNamedDict->getAt(g_namedDict, dictId);
                if (es == Acad::eKeyNotFound) {
                    pClone = NULL;
                    pSrcDict->deepClone(pDestNamedDict, pClone, idMap);

                    // Unless we have overridden the deepClone
                    // of our dictionary, we should expect it to
                    // always be cloned here.
                    if (pClone == NULL) {
                        *pRetStat = Acad::eNullObjectId;
                        break;
                    }

                    pDestNamedDict->setAt(g_namedDict, pClone, dictId);
                    pDestNamedDict->close();
                    pClone->close();
                    break;
                }
                pDestNamedDict->close();
                // Our dictionary already exists in the destination
                // database, so now we must "merge" the entries
                // into it.  Since we have not cloned our
                // destination dictionary, its objectIds are not in
                // flux, and we can use getAt() and setAt() on it.
                AcDbDictionary* pDestDict = nullptr;
                acdbOpenObject(pDestDict, dictId, AcDb::kForWrite);

                AcDbObject* pObj, * pObjClone;
                AcDbDictionaryIterator* pIter;
                for (pIter = pSrcDict->newIterator(); !pIter->done(); pIter->next()) {
                    const TCHAR* pName = pIter->name();
                    pIter->getObject(pObj, AcDb::kForRead);
                    // If the dictionary contains any references
                    // and/or other objects have references to it,
                    // you must either use deepClone() or put the
                    // idPairs into the idMap here, so that they
                    // will be in the map for translation.
                    pObjClone = nullptr;
                    pObj->deepClone(pDestDict, pObjClone, idMap);
                    // INSERT usually uses a method of cloning
                    // called CheapClone, where it "moves" objects
                    // into the destination database instead of
                    // actually cloning them.  When this happens,
                    // pObj and pObjClone are pointers to the
                    // same object.  We only want to close pObj
                    // here if it really is a different object.
                    if (pObj != pObjClone)
                        pObj->close();

                    if (pObjClone == NULL)
                        continue;

                    // If the name already exists in our
                    // destination dictionary, it must be changed
                    // to something unique.  In this example, the
                    // name is changed to an annonymous entry. 
                    // The setAt() method will automatically append
                    // a unique identifier to each name beginning
                    // with "*".  It will become something like,
                    // "*S04".
                    if (pDestDict->getAt(pName, dictId)
                        == Acad::eKeyNotFound)
                        pDestDict->setAt(pName, pObjClone, dictId);
                    else
                        pDestDict->setAt(_T("*S"), pObjClone, dictId);

                    pObjClone->close();
                }
                delete pIter;
                pDestDict->close();
                break;
            }
            default:
                break;
            }

            pSrcDict->close();
        }
    }


    void clone_work::init_impl() {
        acedRegCmds->addCommand(_T("MY_COMMAND_CLONE_WORK"), _T("GLOBAL_CLONE1"), _T("LOCAL_CLONE1"), ACRX_CMD_MODAL, &detail::clone1);

        acedRegCmds->addCommand(_T("MY_COMMAND_CLONE_WORK"), _T("GLOBAL_CLONE2"), _T("LOCAL_CLONE2"), ACRX_CMD_MODAL, &detail::clone2);

        acedRegCmds->addCommand(_T("MY_COMMAND_CLONE_WORK"), _T("GLOBAL_BEFORE_CLONE3"), _T("LOCAL_BEFORE_CLONE3"),
            ACRX_CMD_MODAL, &detail::before_clone3);

        acedRegCmds->addCommand(_T("MY_COMMAND_CLONE_WORK"), _T("GLOBAL_CLONE3"), _T("LOCAL_CLONE3"), ACRX_CMD_MODAL, &detail::clone3);

        acedRegCmds->addCommand(_T("MY_COMMAND_CLONE_WORK"), _T("GLOBAL_CLONE4"), _T("LOCAL_CLONE4"), ACRX_CMD_MODAL, &detail::clone4);

        acedRegCmds->addCommand(_T("MY_COMMAND_CLONE_WORK"), _T("GLOBAL_CLONE_REACTOR"), _T("LOCAL_CLONE_REACTOR"), ACRX_CMD_MODAL, &detail::reactor);
 
        
    }

    void clone_work::stop_impl() {
        acedRegCmds->removeGroup(_T("MY_COMMAND_CLONE_WORK"));
        // 移除临时文件
        if (_taccess(detail::tempDwgUrl(), 0) != -1)
            CFile::Remove(detail::tempDwgUrl());
        //
        clear();
    }

    void clone_work::clear() {
        delete edit_reactor_;
        edit_reactor_ = nullptr;
    }

    void clone_work::make_reactor() {
        if (edit_reactor_ == nullptr) {
            edit_reactor_ = new MySimpleEditReactor;
            edit_reactor_->ops_[_T("beginDeepCloneXlation")] = [](const TCHAR*, void* data) {detail::beginDeepCloneXlation(data); };
        }
    }
}