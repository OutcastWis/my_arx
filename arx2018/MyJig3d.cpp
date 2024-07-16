#include "pch.h"
#include "MyJig3d.h"

#include <acgs.h>
#include <AcGsManager.h>
#include <dbxutil.h>
#include <geassign.h>


namespace detail {
    static AcDbObjectId GetModelSpaceBlockID(void)
    {
        AcDbObjectId            ObjectId;
        AcDbBlockTable* pBlockTable;
        Acad::ErrorStatus       status;

        status = acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBlockTable, AcDb::kForRead);
        if (status != Acad::eOk)
            return ObjectId;

        status = pBlockTable->getAt(ACDB_MODEL_SPACE, ObjectId);
        pBlockTable->close();
        return ObjectId;
    }
}

MyJig3d::~MyJig3d()
{
    if (m_pModel)
    {
        m_pModel->onErased(&m_int, nullptr);
    }
}

AcEdJig::DragStatus MyJig3d::sampler()
{
    setSpecialCursorType(AcEdJig::kRubberBand);
    AcGePoint3d pt, temp;
    AcEdJig::DragStatus status;
    double angle;
    if (m_mode == kMove) {
        setKeywordList(_T("Base X Y Z Exit")); // 设置5个关键字, 对应drag()的AcEdJig::kKW1-5
        status = acquirePoint(pt, m_refPoint); // 该函数以m_refPoint为基础, 返回用户移动后的坐标pt. 俩者之间生成橡皮线
        acdbWcs2Ucs(asDblArray(pt), asDblArray(pt), Adesk::kFalse);
        pt.z = m_elev;
        acdbUcs2Wcs(asDblArray(pt), asDblArray(pt), Adesk::kFalse);
    }
    else {
        setKeywordList(_T("Exit"));
        status = acquirePoint(pt, m_refPoint);
        acdbWcs2Ucs(asDblArray(pt), asDblArray(pt), Adesk::kFalse);
        pt.z = m_elev;
        acdbUcs2Wcs(asDblArray(pt), asDblArray(pt), Adesk::kFalse);
        angle = acutAngle(asDblArray(pt), asDblArray(m_refPoint));
    }

    if (pt == m_LastPoint)      // if there is no work to be done,
        return status;          // exit early!
    m_LastPoint = pt;

    if (status == AcEdJig::kNormal) { // 正常拖拽
        switch (m_mode)
        {
        case kMove:
            m_xformTemp = AcGeMatrix3d::translation(pt - m_refPoint);
            m_int.m_mat = (m_xformTemp * m_xform);
            break;
        case kRotateX:
            m_xformTemp = AcGeMatrix3d::rotation(angle, AcGeVector3d::kXAxis, m_refPoint);
            m_int.m_mat = (m_xformTemp * m_xform);
            break;
        case kRotateY:
            m_xformTemp = AcGeMatrix3d::rotation(angle, AcGeVector3d::kYAxis, m_refPoint);
            m_int.m_mat = (m_xformTemp * m_xform);
            break;
        case kRotateZ:
            m_xformTemp = AcGeMatrix3d::rotation(angle, AcGeVector3d::kZAxis, m_refPoint);
            m_int.m_mat = (m_xformTemp * m_xform);
            break;
        }
        // 在onAdd之后, 这里可以传递nullptr
        m_pModel->onModified(&m_int, nullptr);
        // m_pModel->onModified(&m_int, detail::GetModelSpaceBlockID().asOldId());
    }
    return status;
}

void MyJig3d::engage(void)
{
    const TCHAR* prompt = _T("\nSpecify displacement or [Base point/X/Y/Z/Exit]:");
    AcEdJig::DragStatus status;
    do {
        setDispPrompt(prompt);
        status = drag();
        switch (status)
        {
        case AcEdJig::kKW1:
            if (m_mode == kMove)
            {
                acedGetPoint(asDblArray(m_refPoint), _T("Specify base point:"), asDblArray(m_refPoint));
                acdbUcs2Wcs(asDblArray(m_refPoint), asDblArray(m_refPoint), Adesk::kFalse);
            }
            else {
                apply();
                status = AcEdJig::kCancel;
            }
            break;
        case AcEdJig::kKW2:
            assert(m_mode == kMove);
            m_mode = kRotateX;
            prompt = _T("Specify rotation around X axis or [Exit]:");
            break;
        case AcEdJig::kKW3:
            assert(m_mode == kMove);
            m_mode = kRotateY;
            prompt = _T("Specify rotation around Y axis [Exit]:");
            break;
        case AcEdJig::kKW4:
            assert(m_mode == kMove);
            m_mode = kRotateZ;
            prompt = _T("Specify rotation around Z axis [Exit]:");
            break;
        case AcEdJig::kKW5:
            apply();
            status = AcEdJig::kCancel;
            break;
        default:
            m_xform = m_xformTemp * m_xform;
            if (m_mode == kMove)
                m_refPoint += m_xformTemp.translation();
            m_mode = kMove;
            prompt = _T("\nSpecify displacement or [Base point/X/Y/Z/Exit]:");
            break;
        };
    } while (status != AcEdJig::kCancel);
}

void MyJig3d::init(const AcDbObjectId& idEntity, const AcGePoint3d& refPoint, int viewportNumber) {
    m_int.m_idEntity = idEntity;
    m_refPoint = refPoint;

	AcGsView* pView = acgsGetCurrent3dAcGsView(viewportNumber);
	if (pView == 0)
		throw CString(_T("Perform this command in a 3d View.  Use the shademode command to activate one."));

    assert(pView->getModel(&m_int) == nullptr);
	auto& kernel = pView->getDevice()->graphicsKernel();
    auto& desc = kernel.getDescriptor();
    CString info;
    for (auto it = desc.begin(); it != desc.end(); ++it)
        info.Format(_T("%s & %s"), (const TCHAR*)info, (const wchar_t*)(*it));
    ads_printf(_T("GsKernel desc: %s\n"), (const TCHAR*)info);

#ifdef CAD2022
    m_pModel = acgsGetGsModel(&kernel, acdbHostApplicationServices()->workingDatabase());
#else 
    m_pModel = acgsGetGsManager()->gsModel(acdbHostApplicationServices()->workingDatabase());
#endif 
    if (m_pModel == NULL)
        throw CString(_T("Unable to retrieve AcDb AcGsModel"));

#ifdef CAD2022
	auto check_model = acgsGetCurrentGsManager2()->gsModel(&kernel, acdbHostApplicationServices()->workingDatabase());
#else
    auto check_model = acgsGetGsModel(acdbHostApplicationServices()->workingDatabase());
#endif
    assert(check_model == m_pModel);

	AcGePoint3d pt(refPoint);
	acdbWcs2Ucs(asDblArray(pt), asDblArray(pt), Adesk::kFalse);
    m_elev = pt.z;

    // 这里一定要传递ModelSpaceBlockID, 否则图形不会在移动时候显示
    m_pModel->onAdded(&m_int, detail::GetModelSpaceBlockID().asOldId());
   // m_pModel->onAdded(&m_int, nullptr);

    m_mode = kMove;
}

void MyJig3d::apply()
{
    m_xform = m_xformTemp * m_xform;
    AcDbEntity* pE;
    if (acdbOpenObject(pE, m_int.m_idEntity, AcDb::kForWrite) == Acad::eOk)
    {
        Acad::ErrorStatus es;
        if ((es = pE->transformBy(m_xform)) != Acad::eOk)
            throw CString(_T("Transformation cannot be applied to object"));
        pE->close();
    }
}

