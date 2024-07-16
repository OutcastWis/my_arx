#include "pch.h"
#include "jig.h"

#include <dbxutil.h>
#include <geassign.h>

#include "MyJig3d.h"

namespace wzj {

    namespace detail {
        void jig3d() {
            try {
                resbuf rb;
                ads_getvar(_T("cvport"), &rb);
                if (rb.resval.rint == 1) // 1表示paper space, 2表示model space
                    throw CString(_T("Jig3d cannot be used in layout mode"));

                ads_name ename;
                ads_point pt;
                int rt;
                if ((rt = acedEntSel(_T("Select object:"), ename, pt)) == RTCAN)
                    return;
                if (rt != RTNORM)
                    throw CString(_T("Invalid selection. Try again!"));
                AcDbObjectId id;
                acdbGetObjectId(id, ename);

                MyJig3d jig;
                acdbUcs2Wcs(pt, pt, Adesk::kFalse);
                jig.init(id, asPnt3d(pt), rb.resval.rint);
                jig.engage();
            }
            catch (const CString& err) {
                acutPrintf(_T("ERROR:%s "), (const TCHAR*)err);
            }
        }
    }


    void jig::init_impl() {
        acedRegCmds->addCommand(_T("MY_COMMAND_JIG"), _T("GLOBAL_JIG3D"), _T("LOCAL_JIG3D"), ACRX_CMD_MODAL, &detail::jig3d);
    }

    void jig::stop_impl() {
        acedRegCmds->removeGroup(_T("MY_COMMAND_JIG"));
    }
}