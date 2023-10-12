#pragma once
#include <dbosnap.h>

/**
* @file
* @brief �������, �����������ṩ���������
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
	bool m_bSnapToSegments = false; // ��polyline������. �Ƿ񲶻�polyline�ķֶ�
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
		// �������⴦��
		return Acad::eOk;
	}
};

/**
* AcDbCurve level protocol extension
* ���ߵ����ֵ�
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
* ������Բ���ֶ�, ����ֶε�3�ֵ�. ���������3�ֵ�
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
