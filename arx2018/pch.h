// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H


#if defined(_DEBUG) && !defined(AC_FULL_DEBUG)
#error _DEBUG should not be defined except in internal Adesk debug builds
#endif

// 添加要在此处预编译的标头
#include "framework.h"

#include <tchar.h>

// AUTOCAD
#include <aced.h>
#include <acedads.h>  // aced全局函数
#include <adslib.h>
#include <dbents.h>
#include <dbapserv.h>
#include <dbobjptr.h> // AcDbObjectPointer
#include <axobjref.h> // AcAxObjectRefPtr
#include <rxmfcapi.h> // 和MFC相关的接口. 例如获取App, View, Frame等

#endif //PCH_H
