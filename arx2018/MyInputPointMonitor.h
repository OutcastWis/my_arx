
#include "acedinpt.h"

class MyInputPointMonitor : public AcEdInputPointMonitor {
public:
    virtual Acad::ErrorStatus monitorInputPoint(const AcEdInputPoint& input, AcEdInputPointMonitorResult& output);
    virtual bool excludeFromOsnapCalculation(const AcArray<AcDbObjectId>& nestedEntity, Adesk::GsMarker gsSelectionMark);
};