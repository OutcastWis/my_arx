#pragma once

#include "MyGlyph.h"

class MyOsnapMode : public AcDbCustomOsnapMode
{
public:
    MyOsnapMode();
    virtual ~MyOsnapMode();
    // 命名规则等价于注册命令时
    virtual const TCHAR* localModeString() const override;
    virtual const TCHAR* globalModeString() const override;

    virtual const AcRxClass* entityOsnapClass() const override;
    virtual AcGiGlyph* glyph() const override;
    virtual const TCHAR* tooltipString() const override;
private:
    MyGlyph* m_pGlyph;
};

