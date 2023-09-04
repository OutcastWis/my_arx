#include "pch.h"
#include "command.h"


#include <tchar.h>


#include <rxobject.h>
#include <rxregsvc.h>
#include <rxmfcapi.h>
#include <aced.h>
#include <dbelipse.h>
#include <dbsymtb.h>
#include <geassign.h>
#include <dbapserv.h>
#include <dbgroup.h>
#include <acestext.h>
#include <AcExtensionModule.h>

#include "MyAcUiDialog.h"
#include "MyTxtStyle.h"
#include "MyTessellate.h"
#include "cmd_count.h"

extern CAcExtensionModule theArxDLL;


void register_class() {
    MyTxtStyle::rxInit();
    MyTessellate::rxInit();


    acrxBuildClassHierarchy();
}

void unregister_class() {
    deleteAcRxClass(MyTxtStyle::desc());
    deleteAcRxClass(MyTessellate::desc());

    wzj::cmd_count::instance()->stop();
}

void init_app() {

    register_class();

    acedRegCmds->addCommand(_T("WZJ_COMMAND_GROUP"), _T("GLOBAL_TEST"), _T("LOCAL_TEST"), 
        ACRX_CMD_MODAL | ACRX_CMD_REDRAW, test);

    acedRegCmds->addCommand(_T("WZJ_COMMAND_GROUP"), _T("GLOBAL_CURVES"), _T("LOCAL_CURVES"), ACRX_CMD_MODAL, curves);

    acedRegCmds->addCommand(_T("WZJ_COMMAND_GROUP"), _T("GLOBAL_MKENTS"), _T("LOCAL_MKENTS"), ACRX_CMD_MODAL, mk_ents);

    acedRegCmds->addCommand(_T("WZJ_COMMAND_GROUP"), _T("GLOBAL_MODEL_DIALOG"), _T("LOCAL_MODEL_DIALOG"), ACRX_CMD_MODAL, model_dialog,
        NULL, -1, theArxDLL.ModuleResourceInstance());

    acedRegCmds->addCommand(_T("WZJ_COMMAND_GROUP"), _T("GLOBAL_PROG_METER"), _T("LOCAL_PROG_METER"), ACRX_CMD_MODAL, progress_meter);

    acedRegCmds->addCommand(_T("WZJ_COMMAND_GROUP"), _T("GLOBAL_CUSTOM_TEXT"), _T("LOCAL_CUSTOM_TEXT"), ACRX_CMD_MODAL, custom_text);

    acedRegCmds->addCommand(_T("WZJ_COMMAND_GROUP"), _T("GLOBAL_COMPLEX_LAYER"), _T("LOCAL_COMPLEX_LAYER"), ACRX_CMD_MODAL, complex_layer);

    acedRegCmds->addCommand(_T("WZJ_COMMAND_GROUP"), _T("GLOBAL_CMD_COUNT"), _T("LOCAL_CMD_COUNT"), ACRX_CMD_MODAL, cmd_count);

    acedRegCmds->addCommand(_T("WZJ_COMMAND_GROUP"), _T("GLOBAL_HLSUB"), _T("LOCAL_HLSUB"), ACRX_CMD_MODAL, highlight_subentity);
}


void unload_app() {
    acedRegCmds->removeGroup(_T("WZJ_COMMAND_GROUP"));

    unregister_class();
}


Acad::ErrorStatus add_to_model_space(AcDbObjectId& objId, AcDbEntity* pEntity) {
    AcDbBlockTable* pBlockTable;
    AcDbBlockTableRecord* pSpaceRecord;

    acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);

    pBlockTable->getAt(ACDB_MODEL_SPACE, pSpaceRecord, AcDb::kForWrite);
    pBlockTable->close();

    pSpaceRecord->appendAcDbEntity(objId, pEntity);
    pSpaceRecord->close();

    return Acad::eOk;
}


void create_layer(const TCHAR* layer_name) {
    AcDbLayerTable* pLayerTable;
    auto es = acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pLayerTable, AcDb::kForWrite);
    if (es != Acad::eOk) {
        ads_printf(_T("\n[ERROR] Failed to open symbol table. Error: %s"), acadErrorStatusText(es));
    }

    AcDbLayerTableRecord* pLayerTableRecord =new AcDbLayerTableRecord;
    pLayerTableRecord->setName(layer_name);

    // Defaults are used for other properties of 
    // the layer if they are not otherwise specified.

    pLayerTable->add(pLayerTableRecord);
    pLayerTable->close();
    pLayerTableRecord->close();
}


AcDbObjectId create_line() {
    AcGePoint3d startPt(4.0, 2.0, 0.0);
    AcGePoint3d endPt(10.0, 7.0, 0.0);
    AcDbLine* pLine = new AcDbLine(startPt, endPt);

    AcDbBlockTable* pBlockTable;
    acdbHostApplicationServices()->workingDatabase()
        ->getSymbolTable(pBlockTable, AcDb::kForRead);

    AcDbBlockTableRecord* pBlockTableRecord;
    pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord,
        AcDb::kForWrite);
    pBlockTable->close();

    AcDbObjectId lineId;
    pBlockTableRecord->appendAcDbEntity(lineId, pLine);

    pBlockTableRecord->close();
    pLine->close();

    return lineId;
}


AcDbObjectId create_circle() {
    AcGePoint3d center(9.0, 3.0, 0.0);
    AcGeVector3d normal(0.0, 0.0, 1.0);
    AcDbCircle* pCirc = new AcDbCircle(center, normal, 2.0);

    AcDbBlockTable* pBlockTable;
    acdbHostApplicationServices()->workingDatabase()
        ->getSymbolTable(pBlockTable, AcDb::kForRead);

    AcDbBlockTableRecord* pBlockTableRecord;
    pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord,
        AcDb::kForWrite);
    pBlockTable->close();

    AcDbObjectId circleId;
    pBlockTableRecord->appendAcDbEntity(circleId, pCirc);

    pBlockTableRecord->close();
    pCirc->close();

    return circleId;
}


void create_group(AcDbObjectIdArray& objIds, const TCHAR* pGroupName) {
    AcDbGroup* pGroup = new AcDbGroup(pGroupName);

    // Put the group in the group dictionary which resides
    // in the named object dictionary.
    //
    AcDbDictionary* pGroupDict;
    acdbHostApplicationServices()->workingDatabase()->getGroupDictionary(pGroupDict, AcDb::kForWrite);

    AcDbObjectId pGroupId;
    pGroupDict->setAt(pGroupName, pGroup, pGroupId);
    pGroupDict->close();

    // Now that the group has been added, it has an ObjectID.
    // This is important since the group will become a persistent
    // reactor for the added entities...
    for (int i = 0; i < objIds.length(); i++) {
        pGroup->append(objIds[i]);
    }

    pGroup->close();
}


Acad::ErrorStatus change_color(AcDbObjectId entId, Adesk::UInt16 newColor)
{
    AcDbEntity* pEntity;
    acdbOpenObject(pEntity, entId, AcDb::kForWrite);

    pEntity->setColorIndex(newColor);
    pEntity->close();

    return Acad::eOk;
}


void curves() {
    ads_name en = {};
    AcGePoint3d pt;
    if (ads_entsel(_T("\nSelect an Ellipse: "), en, asDblArray(pt)) != RTNORM)
    {
        acutPrintf(_T("\nNothing selected"));
        return;
    }

    AcDbObjectId eId;
    acdbGetObjectId(eId, en);

    AcDbObject* pObj;
    acdbOpenObject(pObj, eId, AcDb::kForRead);
    if (pObj->isKindOf(AcDbEllipse::desc())) {
        pObj->close();
        acutPrintf(_T("\n[OK] Selected an ellipse"));
    }
    else {
        pObj->close();
        acutPrintf(_T("\n[WARNING] Selected entity is not an ellipse"));
    }
}


void mk_ents() {
    ads_printf(_T("\n[INFO] Create a layer called WZJ_MYLAYER"));
    create_layer(_T("WZJ_MYLAYER"));

    AcDbObjectIdArray idArr;
    idArr.append(create_line());
    idArr.append(create_circle());

    ads_printf(_T("\n[INFO] Change the color of circle to Index 1"));
    change_color(idArr.last(), 1);

    ads_printf(_T("\n[INFO] Make a group called WZJ_MYGROUP"));
    create_group(idArr, _T("WZJ_MYGROUP"));
}


void model_dialog() {
    MyAcUiDialog dlg(CWnd::FromHandle(adsw_acadMainWnd()));
    INT_PTR nReturnValue = dlg.DoModal();
}


void progress_meter() {
    acutPrintf(_T("Running Progress Meter...\n"));

    acedSetStatusBarProgressMeter(_T("Test Progress Bar"), 0, 100);
    for (int i = 0; i <= 100; i++) {
        Sleep(100);
        acedSetStatusBarProgressMeterPos(i);
    }
    acedRestoreStatusBar();
}


void custom_text() {
    // 获取text style
    AcDbTextStyleTable* pStyleTable;
    acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pStyleTable, AcDb::kForRead);

    AcDbTextStyleTableIterator* pIterator;
    pStyleTable->newIterator(pIterator);
    // get ObjectId of first record in table
    AcDbTextStyleTableRecord* style = nullptr;
    pIterator->getRecord(style);
    delete pIterator;
    pStyleTable->close();

    // AcDbTextStyleTableRecord 转 AcGiTextStyle
    AcGiTextStyle ts;
    {
        const TCHAR* pTmpStr;
        style->fileName(pTmpStr);
        ts.setFileName(pTmpStr);

        style->bigFontFileName(pTmpStr);
        ts.setBigFontFileName(pTmpStr);

        ts.setTextSize(style->textSize());
        ts.setXScale(style->xScale());
        ts.setObliquingAngle(style->obliquingAngle());

        style->close();
        ts.loadStyleRec(); // 该函数最后调用
    }


    MyTxtStyle::SetStyle(ts);
    MyTxtStyle* mts = new MyTxtStyle;

    AcDbObjectId id;
    add_to_model_space(id, mts);
    mts->close();
}

void complex_layer() {
    AcDbLayerTable* pLayerTbl;
    acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pLayerTbl, AcDb::kForWrite);

    if (!pLayerTbl->has(_T("WZJ_COMPLEX_LAYER"))) {
        AcDbLayerTableRecord* pLayerTblRcd = new AcDbLayerTableRecord;
        pLayerTblRcd->setName(_T("WZJ_COMPLEX_LAYER"));
        pLayerTblRcd->setIsFrozen(0);// layer to THAWED
        pLayerTblRcd->setIsOff(0);   // layer to ON
        pLayerTblRcd->setVPDFLT(0);  // viewport default
        pLayerTblRcd->setIsLocked(0);// un-locked

        AcCmColor color;
        color.setColorIndex(1); // set color to red
        pLayerTblRcd->setColor(color);

        // For linetype, we need to provide the object ID of
        // the linetype record for the linetype we want to
        // use.  First, we need to get the object ID.
        AcDbLinetypeTable* pLinetypeTbl;
        AcDbObjectId ltId;
        acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pLinetypeTbl, AcDb::kForRead);
        if ((pLinetypeTbl->getAt(_T("DASHED"), ltId)) != Acad::eOk)
        {
            acutPrintf(_T("\nUnable to find DASHED linetype. Using CONTINUOUS"));

            // CONTINUOUS is in every drawing, so use it.
            pLinetypeTbl->getAt(_T("CONTINUOUS"), ltId);
        }
        pLinetypeTbl->close();

        pLayerTblRcd->setLinetypeObjectId(ltId);
        pLayerTbl->add(pLayerTblRcd);
        pLayerTblRcd->close();
        pLayerTbl->close();
    }
    else {
        pLayerTbl->close();
        acutPrintf(_T("\nlayer WZJ_COMPLEX_LAYER already exists"));
    }
}


void tessellate() {
    MyTessellate* t = new MyTessellate;

    AcDbObjectId id;
    add_to_model_space(id, t);
    t->close();
}

void cmd_count() {
    wzj::cmd_count::instance()->init();
}

