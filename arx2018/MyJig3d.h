#pragma once

#include <dbjig.h>
#include <drawable.h>

class Intermediary : public AcGiDrawable
{
public:

    virtual Adesk::UInt32 subSetAttributes(AcGiDrawableTraits* traits) override
    {
        (void*)traits;
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
        (void*)vd;
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
* 非常简单的橡皮绳Jig. 
* @detail 对象在移动式, 使用橡皮绳表现. 并使用AcGsModel来管理绘制中间过程
*      
*/
class MyJig3d : public AcEdJig
{
public:
    MyJig3d() :m_pModel(0), m_LastPoint(0.0, 0.0, 0.0) {}
    ~MyJig3d();

    // 整体流程由engage()开始, 不断调用drag()去触发sampler().同时将
    // 原本更新操作由update放到了engage中, 所以update不用干事
    virtual Adesk::Boolean  update() override
    {
        return Adesk::kFalse;
    }
    // 记录在m_int中, 这边无用
    virtual AcDbEntity* entity() const override
    {
        return const_cast<AcDbPolyline*>(&m_dummy);
    }

    virtual DragStatus	sampler() override;

    void engage(void);

    void init(const AcDbObjectId& idEntity, const AcGePoint3d& refPoint, int viewportNumber);
private:
    AcDbPolyline m_dummy;
    AcGsModel* m_pModel;    // 控制图形的绘制
    AcGePoint3d m_refPoint; // WCS
    double m_elev;			// UCS
    Intermediary m_int;
    AcGeMatrix3d m_xform;
    AcGeMatrix3d m_xformTemp;
    AcGePoint3d  m_LastPoint;

    enum Mode { kMove = 0, kRotateX, kRotateY, kRotateZ } m_mode;

    void apply();
};

