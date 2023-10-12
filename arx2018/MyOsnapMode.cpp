#include "pch.h"
#include "MyOsnapMode.h"

#include "MyOsnapInfo.h"

MyOsnapMode::MyOsnapMode()
{
    m_pGlyph = new MyGlyph;
}

MyOsnapMode::~MyOsnapMode()
{
    delete m_pGlyph;
    m_pGlyph = nullptr;
}

const TCHAR* MyOsnapMode::localModeString() const
{
    return _T("WZJOsnapmode");
}

const TCHAR* MyOsnapMode::globalModeString() const
{
    return _T("WZJOsnapmode");
}

const AcRxClass* MyOsnapMode::entityOsnapClass() const
{
    return MyOsnapInfo::desc();
}

AcGiGlyph* MyOsnapMode::glyph() const
{
    return m_pGlyph;
}

const TCHAR* MyOsnapMode::tooltipString() const
{
    return _T("My 1/3 osnap");
}
