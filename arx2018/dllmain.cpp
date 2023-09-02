// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include <aced.h>
#include <AcExtensionModule.h>


#include "command.h"

// 用到了MFC资源, 需要进行维护. 不能和CAD的冲突
AC_IMPLEMENT_EXTENSION_MODULE(theArxDLL);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        theArxDLL.AttachInstance(hModule);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        theArxDLL.DetachInstance();
        break;
    }
    return TRUE;
}



extern "C" AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* appId)
{
    switch (msg) {
    case AcRx::kInitAppMsg:
        acrxDynamicLinker->unlockApplication(appId);
        acrxDynamicLinker->registerAppMDIAware(appId);
        init_app();
        break;
    case AcRx::kUnloadAppMsg:
        unload_app();
    }
    return AcRx::kRetOK;
}
