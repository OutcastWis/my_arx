#pragma once

#include <dbjig.h>
#include <drawable.h>

class Intermediary : public AcGiDrawable
{
public:

    virtual Adesk::UInt32 subSetAttributes(AcGiDrawableTraits* traits) override
    {
        return kDrawableIsCompoundObject;
    }

    virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw* wd) override
    {
        wd->geometry().pushModelTransform(m_mat);
        AcDbEntity* pE;
        if (acdbOpenObject(pE, m_idEntity, AcDb::kForRead) == Acad::eOk)
        {
            wd->geometry().draw(pE);
            pE->close();
        }
        wd->geometry().popModelTransform();
        return Adesk::kTrue;
    }

    virtual void subViewportDraw(AcGiViewportDraw* vd) override
    {
        return;
    }

    virtual Adesk::Boolean  isPersistent(void) const override
    {
        return Adesk::kFalse;
    };

    virtual AcDbObjectId id(void) const override
    {
        return AcDbObjectId::kNull;
    }

public:
    AcDbObjectId m_idEntity;
    AcGeMatrix3d m_mat;
};


/**
* �ǳ��򵥵���Ƥ��Jig. 
* @detail �������ƶ�ʽ, ʹ����Ƥ������. ����˳�ʱ, �Ž��ж���Ļ���.
*      
*/
class MyJig3d : public AcEdJig
{
public:
    MyJig3d() :m_pModel(0), m_LastPoint(0.0, 0.0, 0.0) {}
    ~MyJig3d();

    // ����������engage()��ʼ, ���ϵ���drag()ȥ����sampler().ͬʱ��
    // ԭ�����²�����update�ŵ���engage��, ����update���ø���
    virtual Adesk::Boolean  update() override
    {
        return Adesk::kFalse;
    }
    // ��¼��m_int��, �������
    virtual AcDbEntity* entity() const override
    {
        return const_cast<AcDbPolyline*>(&m_dummy);
    }

    virtual DragStatus	sampler() override;

    void engage(void);

    void init(const AcDbObjectId& idEntity, const AcGePoint3d& refPoint, int viewportNumber);
private:
    AcDbPolyline m_dummy;
    AcGsModel* m_pModel;    // ����ͼ�εĻ���. ��jig����ʱ�Ż���ͼ��
    AcGePoint3d m_refPoint; // WCS
    double m_elev;			// UCS
    Intermediary m_int;
    AcGeMatrix3d m_xform;
    AcGeMatrix3d m_xformTemp;
    AcGePoint3d  m_LastPoint;

    enum Mode { kMove = 0, kRotateX, kRotateY, kRotateZ } m_mode;

    void apply();
};

