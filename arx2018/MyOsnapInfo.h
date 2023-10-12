#pragma once
#include <dbosnap.h>

/**
* @file
* @brief 这里的类, 用来给捕获提供捕获点坐标
*/



class MyOsnapInfo : public AcDbCustomOsnapInfo
{
public:
	ACRX_DECLARE_MEMBERS(MyOsnapInfo);

	virtual Acad::ErrorStatus getOsnapInfo(
		AcDbEntity* pickedObject,
		Adesk::GsMarker		gsSelectionMark,
		const AcGePoint3d& pickPoint,
		const AcGePoint3d& lastPoint,
		const AcGeMatrix3d& viewXform,
		AcArray<AcGePoint3d>& snapPoints,
		AcArray<int>& geomIdsForPts,
		AcArray<AcGeCurve3d*>& snapCurves,
		AcArray<int>& geomIdsForLines) override = 0;

	void setSnapToSegments(bool v) {
		m_bSnapToSegments = v;
	}
protected:
	bool m_bSnapToSegments = false; // 对polyline起作用. 是否捕获polyline的分段
};

// AcDbEntity level protocol extension

class MyOsnapEntityInfo : public MyOsnapInfo
{
public:
	virtual Acad::ErrorStatus   getOsnapInfo(
		AcDbEntity* pickedObject,
		Adesk::GsMarker		gsSelectionMark,
		const AcGePoint3d& pickPoint,
		const AcGePoint3d& lastPoint,
		const AcGeMatrix3d& viewXform,
		AcArray<AcGePoint3d>& snapPoints,
		AcArray<int>& geomIdsForPts,
		AcArray<AcGeCurve3d*>& snapCurves,
		AcArray<int>& geomIdsForLines) override {
		// 不做特殊处理
		return Acad::eOk;
	}
};

/**
* AcDbCurve level protocol extension
* 弧线的三分点
*/
class MyOsnapCurveInfo : public MyOsnapInfo
{
public:
	virtual Acad::ErrorStatus   getOsnapInfo(
		AcDbEntity* pickedObject,
		Adesk::GsMarker		gsSelectionMark,
		const AcGePoint3d& pickPoint,
		const AcGePoint3d& lastPoint,
		const AcGeMatrix3d& viewXform,
		AcArray<AcGePoint3d>& snapPoints,
		AcArray<int>& geomIdsForPts,
		AcArray<AcGeCurve3d*>& snapCurves,
		AcArray<int>& geomIdsForLines) override;
};

/**
* AcDbPolyline level protocol extension
* 如果可以捕获分段, 则各分段的3分点. 否则总体的3分点
*/
class MyOsnapPolylineInfo : public MyOsnapInfo
{
public:
	virtual Acad::ErrorStatus   getOsnapInfo(
		AcDbEntity* pickedObject,
		Adesk::GsMarker		gsSelectionMark,
		const AcGePoint3d& pickPoint,
		const AcGePoint3d& lastPoint,
		const AcGeMatrix3d& viewXform,
		AcArray<AcGePoint3d>& snapPoints,
		AcArray<int>& geomIdsForPts,
		AcArray<AcGeCurve3d*>& snapCurves,
		AcArray<int>& geomIdsForLines) override;
};
