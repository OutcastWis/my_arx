﻿// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include <aced.h>
#include <AcExtensionModule.h>


#include "command.h"

// 用到了MFC资源, 需要进行维护. 不能和CAD的冲突
AC_IMPLEMENT_EXTENSION_MODULE(theArxDLL);

static AFX_EXTENSION_MODULE arxmfcDLL;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        theArxDLL.AttachInstance(hModule);
        if (!AfxInitExtensionModule(arxmfcDLL, hModule)) // 初始化MFC扩展模块
            return 0;
        // Insert this DLL into the resource chain
        new CDynLinkLibrary(arxmfcDLL);
        break;
    }
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        theArxDLL.DetachInstance();
        AfxTermExtensionModule(arxmfcDLL);
        break;
    }
    return TRUE;
}



extern "C" AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* appId)
{
    switch (msg) {
    case AcRx::kInitAppMsg:
        acrxDynamicLinker->unlockApplication(appId);
        acrxDynamicLinker->registerAppMDIAware(appId); // 多文档. 否则默认单文档
        init_app(appId);
        break;
    case AcRx::kUnloadAppMsg:
        unload_app();
    }
    return AcRx::kRetOK;
}
