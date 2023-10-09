// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

#include <rxdlinkr.h>
#include "MyBlockOrderFilter.h"
#include "MyBlockOrderIndex.h"

AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* pkt)
{
    switch (msg) {
    case AcRx::kInitAppMsg:
    {
        //allow unloading
        acrxDynamicLinker->unlockApplication(pkt);
        //we are MDI aware
        acrxDynamicLinker->registerAppMDIAware(pkt);

        MyBlockOrderFilter::rxInit();
        MyBlockOrderIndex::rxInit();
        acrxBuildClassHierarchy();

        break;
    }
    case AcRx::kUnloadDwgMsg:

        deleteAcRxClass(MyBlockOrderFilter::desc());
        deleteAcRxClass(MyBlockOrderIndex::desc());

        break;
    }
    return AcRx::kRetOK;
}


