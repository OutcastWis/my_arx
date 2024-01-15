#pragma once

#include <acgi.h>

class MyGiContext : public AcGiContext
{
public:
    ACRX_DECLARE_MEMBERS(MyGiContext);

    MyGiContext();
    virtual ~MyGiContext();

    Adesk::Boolean isPsOut() const override;
    Adesk::Boolean isPlotGeneration() const override;
    AcDbDatabase* database() const override;
    bool isBoundaryClipping() const override;
};

