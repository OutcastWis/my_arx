#ifdef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#endif 


#include "pch.h"
#include "cmd_count.h"

#include <rxkernel.h>

#include <Ac64BitHelpers.h>
#include <acdocman.h>
#include <rxregsvc.h>
#include <adesk.h>
#include <appinfo.h>
#include <rxmfcapi.h>

// defined in rxmfcapi.cpp - (exported but not published)
DWORD       acedCommandActive();         // direct access to CMDACTIVE sysvar (avoided getvar for speed reasons)
//BOOL        acedManualInputProvided(AcApDocument* pAcTargetDocument); // last token was provided by a carbon based life form
BOOL        acedMenuInputProvided();


// 在Object2013及以后, acrxProductKey这个全局函数没了, 所以用下面宏替换
#define acrxProductKey acdbHostApplicationServices()->getMachineRegistryProductRootKey

// cmd active flags
#define CMDSTATE_REGULAR      0x00001;
#define CMDSTATE_TRANSPARENT  0x00002;
#define CMDSTATE_SCRIPT       0x00004;
#define CMDSTATE_DIALOG       0x00008;
#define CMDSTATE_DDE          0x00010;
#define CMDSTATE_LISP         0x00020;
#define CMDSTATE_ARX          0x00040;
// addition flag
#define CMDSTATE_MENU         0x01000; //
#define CMDSTATE_MANUAL       0x02000; // this bit is set when it looks like a human invoked the command


#define SECONDS_PER_DAY        86400  // Number of seconds in a day
#define STAT_FILENAME_VERSION    1.4  // Version of statistics file format
#define VERSION_LENGTH           100  // Length of version string
#define LOGINNAME_LENGTH        1024  // Length of LOGINNAME that we use.
#define MAX_STR_LENGTH        4*1024  // More than enough for any string... (see  aclinedb.cpp's k_nCharBufferMaxLen)
#define BASE_DATE            2453450  // this is: 3/20/2005 (a sunday)
#define MAX_CMD_LENGTH ACRX_MAX_CMDNAME_LEN


#define PRODUCTL 21
static ACHAR product[PRODUCTL] = { NULL };  // MDI-Safe. 缓存产品名称
static ACHAR abort_msg[255];              // MDI-Safe

namespace wzj {

    static cmd_count* global_one = cmd_count::instance();
    static const ACHAR* gAppSubKey = ACRX_T("cmd_count");
    static const ACHAR* gAppFileName = ACRX_T("arx2018.arx");

    TCHAR abort_msg[255] = {};

    namespace detail{

        AcRxDictionary* initStatDictionary() {
            return acrxKernel->newAcRxDictionary(0, Adesk::kTrue, Adesk::kTrue);
        }
        // 获取当前命令行状态
        int getCurrentStateFlags() {
            int flags = acedCommandActive();

            if (acedMenuInputProvided())
                flags |= CMDSTATE_MENU;

           /* if (acedManualInputProvided(curDoc()))
                flags |= CMDSTATE_MANUAL;*/

            return flags;
        }

        void printStats(AcRxDictionary* statDict) {
            AcRxDictionaryIterator* iter;

            acutPrintf(ACRX_T("\n\nCommand                 Count     Elapsed Time\n"));
            acutPrintf(ACRX_T("----------------------  -------   ----------\n"));
            for (iter = statDict->newIterator(AcRx::kDictSorted); !iter->done(); iter->next()) {
                // Write out the command string.
                acutPrintf(ACRX_T("%s"), iter->key());

                // Try for a little reasonable text alignment,
                // assuming 24 characters for a command or lisp name is sufficient.
                // But, we'll go to the MAX_CMD_LENGTH, plus one blank, if we must.
                int nbrOfSpaces = 24 - _tcslen(iter->key());
                do
                    acutPrintf(ACRX_T(" "));
                while (--nbrOfSpaces > 0);

                MyCommandRecord* pRecord = (MyCommandRecord*)iter->object();
                if (pRecord == NULL)
                    break;
                // Now the count, plus elapsed time, in primate-comprehensible form.
                double eTime = pRecord->elapsedTime();
                int TempElapsedTime = (int)eTime;
                int hours = TempElapsedTime / 3600;
                int minutes = (TempElapsedTime % 3600) / 60;
                acutPrintf(ACRX_T(/*MSG0*/"%7i   %4d:%0*d:%0*.*f\n"),
                    pRecord->count(),
                    hours,
                    2, minutes,
                    5, 2,
                    eTime
                    - (hours * 3600) - (minutes * 60));

            }

            delete iter;
        }

        int RegGetInt(HKEY hKey, const ACHAR* subKey, const ACHAR* name)
        {
            HKEY hSubKey;
            DWORD type;
            DWORD value = 0;
            DWORD size = sizeof(value);
            if (RegOpenKeyEx(hKey, subKey, 0, KEY_QUERY_VALUE, &hSubKey) == ERROR_SUCCESS)
            {
                if ((RegQueryValueEx(hSubKey, name, 0, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS) ||
                    (type != REG_DWORD))
                    value = 0;
                RegCloseKey(hSubKey);
            }
            return value;
        }

        int RegPutInt(HKEY hKey, const ACHAR* subKey, const ACHAR* name, int value)
        {
            HKEY hSubKey;
            DWORD disp;
            int retval = 0;
            if (RegCreateKeyEx(hKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, &disp) == ERROR_SUCCESS)
            {
                if (RegSetValueEx(hSubKey, name, 0, REG_DWORD, (LPBYTE)&value, sizeof(value)) == ERROR_SUCCESS)
                    retval = 1;
                RegCloseKey(hSubKey);
            }
            return retval;
        }
        // 读取是否自动加载的标志位. true表示自动加载
        bool getStartUpLoad()
        {
            ACHAR appKey[1024];
            wsprintf(appKey, ACRX_T(/*MSGO*/"%s\\Applications\\%s"), acrxProductKey(), gAppSubKey);
            return (0 != (AcadApp::kOnAutoCADStartup & RegGetInt(HKEY_CURRENT_USER, appKey, ACRX_T(/*MSGO*/"LOADCTRLS"))));
        }
        // 将是否自动加载的标志位写入注册表
        void setStartUpLoad(bool loadOnStartUp)
        {
            ACHAR appKey[1024];
            wsprintf(appKey, ACRX_T(/*MSGO*/"%s\\Applications\\%s"), acrxProductKey(), gAppSubKey);
            //  register app
            int r = (AcadApp::kOnCommandInvocation | AcadApp::kOnLoadRequest);
            if (loadOnStartUp)
                r = (r | AcadApp::kOnAutoCADStartup);
            RegPutInt(HKEY_CURRENT_USER, appKey, ACRX_T(/*MSGO*/"LOADCTRLS"), r);
        }

        void commandStats() {
            TCHAR option_str[132];

            ads_initget(0, _T("Cumulative Session"));

            option_str[0] = _T('S');
            option_str[1] = _T('\0');

            switch (ads_getkword(_T("\nCumulative/<Session>:"), option_str)) {
            case RTKWORD:
                ads_getinput(option_str);
            default:
                break;
            }

            if (option_str[0] == _T('C') || option_str[0] == _T('c')) {
                global_one->recordCommandDuration(nullptr);
                if (!global_one->updateCumulativeStats()) {
                    acutPrintf(ACRX_T(/*MSGO*/"\nA Command or LISP expression is active"));
                    acutPrintf(ACRX_T(/*MSGO*/" in at least one document,"));
                    acutPrintf(ACRX_T(/*MSGO*/"\nSession Statistics may not be printed or saved now.\n"));
                    return;
                }

                ads_printf(_T("\n\nCumulative Command Usage Statistics"));
                printStats(global_one->cumulativeStats);
                acutPrintf(ACRX_T(/*MSGO*/"\nNOTE: Session Statistics have been recorded and reset.\n"));
                delete global_one->cumulativeStats;
                global_one->cumulativeStats = NULL;
            }
            else {
                acutPrintf(ACRX_T(/*MSGO*/"\n\nCurrent Session Command Usage Statistics"));
                printStats(global_one->sessionStats);
            }
        }

        void cmdCommandCount() {
            ACHAR reply[50];
            ACHAR prompt[255];
            _stprintf(prompt, ACRX_T("\nLoad CmdCount on start up? <%s>: "), getStartUpLoad() ? _T("Y") : _T("N"));
            //"Load command count on start up?"
            acedInitGet(0, ACRX_T("Yes No _ Yes No"));
            if (acedGetKword(prompt, reply) == RTNORM)
                setStartUpLoad(!_tcscmp(reply, ACRX_T("Yes")));
        }
        // 可以理解为日志. 对cmdStr表示的行为进行计数. flags表示命令行状态. positive确定计数为+1还是-1
        MyCommandRecord* bumpCount(const TCHAR* cmdStr, int flags, Adesk::Boolean positive) {
            MyCommandRecord* cmdRcd = (MyCommandRecord*)global_one->sessionStats->at(cmdStr);
            if (cmdRcd == NULL) {
                // here is where we create the official command stat entry names, so
                // we apply da rules here:
                // 1)  Keep it short
                // 2) first blank or tab termintates the string.
                ACHAR truncatedStr[ACRX_MAX_CMDNAME_LEN + 1];
                if (_tcslen(cmdStr) > ACRX_MAX_CMDNAME_LEN) {
                    _tcsncpy(truncatedStr, cmdStr, ACRX_MAX_CMDNAME_LEN);
                    truncatedStr[ACRX_MAX_CMDNAME_LEN] = EOS;   // Not all implementations
                                                          //  of strncpy() add the 
                                                          // EOS.
                }
                else
                    _tcscpy(truncatedStr, cmdStr);
                // Truncate string at first blank or tab.
                ACHAR* blankChar = _tcschr(truncatedStr, ACRX_T(' '));
                if (blankChar)
                    *blankChar = EOS;
                /* Now look for a TAB character. */
                blankChar = _tcschr(truncatedStr, ACRX_T('\t'));
                if (blankChar)
                    *blankChar = EOS;

                cmdRcd = (MyCommandRecord*)global_one->sessionStats->at(truncatedStr); // try again to locate it
                if (cmdRcd == NULL) {
                    cmdRcd = new MyCommandRecord();
                    if (cmdRcd == NULL)
                        return NULL;
                    global_one->sessionStats->atPut(truncatedStr, cmdRcd);
                }
            }

            int offset = (positive) ? 1 : -1;
            cmdRcd->add(flags, offset, 0.0);

            return cmdRcd;
        }

        ACHAR* getProductName()
        {
            /* Save the Product Name is a static variable, so that we only need
               to actually get it the first time we execute. */
            if (product[0] == NULL) {
                struct resbuf result;
                if (acedGetVar(ACRX_T(/*NOXLATE*/"PRODUCT"), &result) != RTNORM) {
                    /* If we can't get the "PRODUCT" name, assume "AutoCAD" */
                    _tcscpy(product, ACRX_T(/*NOXLATE*/"My_ARX2018"));
                }
                else {
                    _tcscpy(product, result.resval.rstring);
                    acdbFree(result.resval.rstring);
                }
            }
            return product;
        }


        // return double representing Julian Date, values of which
        // may be directly subtracted for elapsed time measurement.
        double getCurTime()
        {
            resbuf currentTime;
            if (acedGetVar(ACRX_T(/*NOXLATE*/"DATE"), &currentTime) != RTNORM) {
                // This should never happen.
                _stprintf(abort_msg, ACRX_T("%.*s Command\nStatistics Gatherer\nFailure 1"),
                    PRODUCTL, getProductName());
                acrx_abort(abort_msg);
            }
            return currentTime.resval.rreal;
        }

        //               return an integer indicating number of elased time in
        //               seconds between input startTime and endTime, which are
        //               assumed to represent Julian dates, as described in
        //               Autodesk Customization guides, as the "DATE" variable format
        //               as opposed to "CDATE" format.
        //
        double elapsedTimeInSeconds(double endTime, double startTime)
        {
            return ((endTime - startTime) * SECONDS_PER_DAY); 
        }

        // 记录命令运行时长
        void recordElapsedCmdTime(MyCommandRecord*& commandRecord, double startTime, int flags) {
            commandRecord->add(flags,0, elapsedTimeInSeconds(getCurTime(), startTime));
        }

        // return filename of the form:
        //  <loginname>_yyyy_mm_dd.txt
        // ...where the date portion changes once every seven days.
        // Note: We no longer search the AutoCAD path due to confusion 
        //       among customers. (per Shaan Hurley request)
        // 
        LPCTSTR cmdcount_filename()
        {
            static CString csFilename;

            CString csLogin;
            resbuf tempRes;
            tempRes.rbnext = NULL;
            if (acedGetVar(ACRX_T(/*NOXLATE*/"LOGINNAME"), &tempRes) == RTNORM) {
                csLogin = tempRes.resval.rstring;
                csLogin.Replace(ACRX_T(/*NOXLATE*/" "), ACRX_T(/*NOXLATE*/"")); // remove spaces and tabs.
                csLogin.Replace(ACRX_T(/*NOXLATE*/"\t"), ACRX_T(/*NOXLATE*/""));
            }
            else
                csLogin = ACRX_T(/*NOXLATE*/"Unknown");

            CString csDate;
            tempRes.restype = RTREAL;
            if (acedGetVar(_T(/*MSG0*/"DATE"), &tempRes) == RTNORM) {
                //gotta do some date voodoo... (just need year, month and day)
                int today = (int)tempRes.resval.rreal;
                int filedate = BASE_DATE + (((int)((today - BASE_DATE) / 7.0)) * 7); // round to chunks of 7 days.
                int a;
                if (filedate < 2299161)
                    a = filedate;
                else {
                    int alpha = (int)((filedate - 1867216.25) / 36524.25);    // Gregorian calendar
                    a = (filedate + 1 + alpha) - (int)(alpha / 4);
                }

                int b = a + 1524;
                int c = (int)((b - 122.1) / 365.25);
                int d = (int)(365.25 * c);
                int e = (int)((b - d) / 30.6001);
                int m = (int)(e - ((e < 14) ? 1 : 13));
                int day = (int)(b - d - ((int)(30.6001 * e)));
                int y = (int)(c - ((m > 2) ? 4716 : 4715));
                csDate.Format(ACRX_T(/*NOXLATE*/"%0*d-%0*d-%0*d"), 4, y, 2, m, 2, day);
            }

            CString csFilePath;
            CString csRegPath;
            csRegPath.Format(ACRX_T(/*NOXLATE*/"%s\\Applications\\AcadCmdCount"), acrxProductKey());
            HKEY hKey;
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, csRegPath, 0, KEY_QUERY_VALUE, &hKey)) {

                DWORD dwType = 0;
                DWORD dwDataSize = MAX_STR_LENGTH * sizeof(TCHAR);
                RegQueryValueEx(hKey, ACRX_T(/*NOXLATE*/"StatPath"), NULL, &dwType, (LPBYTE)csFilePath.GetBuffer(dwDataSize / sizeof(TCHAR)), &dwDataSize);
                csFilePath.ReleaseBuffer();
                RegCloseKey(hKey);
            }
            if (csFilePath.IsEmpty()) {
                //nothing found in reg so use "My Documents"
                resbuf rb;
                if (acedGetVar(ACRX_T(/*NOXLATE*/"MYDOCUMENTSPREFIX"), &rb) == RTNORM) {
                    csFilePath = rb.resval.rstring;
                    acdbFree(rb.resval.rstring);
                }
            }
            csFilePath.TrimRight(ACRX_T(/*NOXLATE*/"\\"));

            csFilename.Format(ACRX_T(/*NOXLATE*/"%s\\%s_%s.txt"), csFilePath, csLogin, csDate);
            return csFilename;
        }

        void countCommand(const ACHAR* cmdStr)
        {
            auto& curDocGlobals = global_one->curDocGlobals;

            // Can't handle zero-doc
            if (curDocGlobals.doc == NULL)
                return;

            // If nesting a command, record the time spent in the pushed command, and
            // stop counting it.
            if (curDocGlobals.cmdIndex > 0)
                recordElapsedCmdTime(curDocGlobals.cmdRcd[curDocGlobals.cmdIndex - 1],
                    curDocGlobals.cmdStartTime[curDocGlobals.cmdIndex - 1],
                    curDocGlobals.cmdFlags[curDocGlobals.cmdIndex - 1]
                );
            int flags = getCurrentStateFlags();
            curDocGlobals.cmdRcd[curDocGlobals.cmdIndex] = bumpCount(cmdStr, flags, Adesk::kTrue);
            curDocGlobals.cmdFlags[curDocGlobals.cmdIndex] = flags;
            curDocGlobals.cmdStartTime[curDocGlobals.cmdIndex] = getCurTime();
            curDocGlobals.cmdIndex++;

        }

        void printCurrentDateTime(AcFILE& statFile)
        {
            resbuf tempRes;
            int jtd_day;
            double jtd_msec;
            int ctd_year, ctd_month, ctd_day, ctd_hour, ctd_minute, ctd_second;
            double rem;
            long t, j, y, d;

            // Get Julian date and convert it to calendar time and date
            // As in Calendar Time and Date, in case you were wondering what
            // the "ctd_" prefix used pervasively in this routine stands for.
            if (acedGetVar(ACRX_T(/*NOXLATE*/"DATE"), &tempRes) != RTNORM) {
                _stprintf(abort_msg, ACRX_T("%.*s Command\nStatistics Gatherer\nFailure 2"),
                    PRODUCTL, getProductName());
                acrx_abort(abort_msg);
            }
            jtd_day = (int)tempRes.resval.rreal;
            jtd_msec = tempRes.resval.rreal - (int)tempRes.resval.rreal;
            j = jtd_day - 1721119L;
            t = 4 * j - 1;
            y = t / 146097L;
            j = t - 146097L * y;
            d = j / 4;
            j = (4 * d + 3) / 1461;
            d = (4 * d - 1461L * j + 7) / 4;
            ctd_month = (5 * d - 3) / 153;
            ctd_day = ((5 * d) - 3 - 153 * ctd_month + 5) / 5;
            ctd_year = (short)((100 * y) + j);
            if (ctd_month >= 10) {
                ctd_month -= 9;
                ctd_year++;
            }
            else
                ctd_month += 3;

            ctd_hour = (short)(jtd_msec * 24);
            rem = (jtd_msec * 24) - ctd_hour;
            ctd_minute = (short)(rem * 60);
            rem = (rem * 60) - ctd_minute;
            ctd_second = (short)(rem * 60);

            // print current date and time
            statFile.fprintf(ACRX_T("%0*d/%0*d/%0*d %0*d:%0*d:%0*d"),
                2, ctd_month,
                2, ctd_day,
                4, ctd_year,
                2, ctd_hour,
                2, ctd_minute,
                2, ctd_second);
        }
        // 将注册的命令信息写入注册表. 用于demand load机制
        void updateRegistry()
        {
            AcadAppInfo appInfo;

            appInfo.setAppName(gAppSubKey);

            appInfo.setModuleName(ads_getappname());

            CString appDesc(gAppSubKey);
            appDesc += _T(" Application Desc");
            appInfo.setAppDesc(appDesc);

            int nLoadReasons = AcadApp::kOnAutoCADStartup
                | AcadApp::kOnCommandInvocation
                | AcadApp::kOnLoadRequest;

            appInfo.setLoadReason(AcadApp::LoadReasons(nLoadReasons));

            appInfo.writeToRegistry(); // 要写在writeCommandNameToRegistry前
            appInfo.writeCommandNameToRegistry(_T("GLOBAL_CMDSTAT"), _T("LOCAL_CMDSTAT"));
            appInfo.writeCommandNameToRegistry(_T("GLOBAL_CMDCOUNT"), _T("LOCAL_CMDCOUNT"));


            CString groupName = ACRX_T(/*MSG0*/"WZJ_COMMAND_CMD_COUNT");
            appInfo.writeGroupNameToRegistry(groupName);
        }

    }


    //##################################################################################


    void MyDocReactor::documentCreated(AcApDocument* pDoc)
    {
        // Log document creations, for the heck of it..
        // Note that bumpCount is independent of document-specific data.
        detail::bumpCount(ACRX_T(/*NOXLATE*/"#DOC_CREATED"), detail::getCurrentStateFlags(), Adesk::kTrue);

        // add an entry for the document, if some other notification hasn't
        // already done so.
        global_one->lookupDoc(pDoc);
    }

    void MyDocReactor::documentToBeActivated(AcApDocument* pDoc)
    {
        auto& curDocGlobals = global_one->curDocGlobals;
        auto& docData = global_one->docData;

        if (pDoc == curDocGlobals.doc)
            detail::bumpCount(ACRX_T(/*NOXLATE*/"#DOC_ACTIVATED(SAME)"), detail::getCurrentStateFlags(), Adesk::kTrue);
        else {
            // If switching documents, record the time spent on the
            // current document's active command/lisp and reset the
            // start time in the new document's active command/lisp.
            int i;
            if (curDocGlobals.doc != NULL) {
                if ((i = curDocGlobals.cmdIndex - 1) >= 0)
                    detail::recordElapsedCmdTime(curDocGlobals.cmdRcd[i], curDocGlobals.cmdStartTime[i], curDocGlobals.cmdFlags[i]);
                if (curDocGlobals.lispRcd != NULL)
                    detail::recordElapsedCmdTime(curDocGlobals.lispRcd, curDocGlobals.lispStartTime, curDocGlobals.lispFlags);
                docData[global_one->lookupDoc(curDocGlobals.doc)] = curDocGlobals;
            }
            if (pDoc == NULL) {
                // This isn't supposed to happen...
                assert(pDoc != NULL);
                // But in production, just log it and keep going...
                detail::bumpCount(ACRX_T(/*NOXLATE*/"#NULL_DOC_ACTIVATED"), detail::getCurrentStateFlags(), Adesk::kTrue);
                curDocGlobals.doc = NULL;
                return;
            }
            curDocGlobals = (docData[global_one->lookupDoc(pDoc)]);
            double currentTime = detail::getCurTime();
            if ((i = curDocGlobals.cmdIndex - 1) >= 0)
                curDocGlobals.cmdStartTime[i] = currentTime;
            curDocGlobals.lispStartTime = currentTime;
            detail::bumpCount(ACRX_T(/*NOXLATE*/"#DOC_SWITCHED"), detail::getCurrentStateFlags(), Adesk::kTrue);
        }
    }

    void MyDocReactor::documentToBeDestroyed(AcApDocument* pDoc)
    {
        auto& curDocGlobals = global_one->curDocGlobals;
        auto& docData = global_one->docData;

        if (pDoc == NULL) {
            // This isn't supposed to happen...
            assert(pDoc != NULL);
            // But in production, just log it and keep going...
            detail::bumpCount(ACRX_T(/*NOXLATE*/"#NULL_DOC_DESTROYED"), detail::getCurrentStateFlags(), Adesk::kTrue);
            return;
        }
        int i = global_one->lookupDoc(pDoc);
        // The current document being destroyed?
        if (pDoc == curDocGlobals.doc) {
            // If Anything is still active in the document being destroyed,
            // log it as done now.
            if (curDocGlobals.lispRcd != NULL) {
                detail::recordElapsedCmdTime(curDocGlobals.lispRcd, curDocGlobals.lispStartTime, curDocGlobals.lispFlags);
                curDocGlobals.lispRcd = NULL;
            }
            global_one->recordCommandDuration(NULL);
            curDocGlobals.doc = NULL;
        }
        // Remove the document data
        docData.removeAt(i);
    }



    //##################################################################################


    void MyCommandRecord::add(int flags, int count, double time)
    {
        MyCommandSubRecord& subRecord = m_subRecords[flags]; // locate or create the needed subrecord.
        subRecord.count += count;
        subRecord.elapsedTime += time;
    }

    void MyCommandRecord::add(const MyCommandRecord* record)
    {
        if (record == NULL)
            return;
        for (auto iter = record->m_subRecords.begin(); iter != record->m_subRecords.end(); iter++)
        {
            int flags = iter->first;
            const MyCommandSubRecord& subRecord = iter->second;
            add(flags, subRecord.count, subRecord.elapsedTime); //call overloaded version above
        }
    }

    void MyCommandRecord::write(LPCTSTR cmd_name, AcFILE& stat_file)
    {
        if (!stat_file.isOpen() || (cmd_name == NULL) || (cmd_name[0] == _T('\0')))
            return;

        for (auto iter = m_subRecords.begin(); iter != m_subRecords.end(); iter++) {
            int flags = iter->first;
            MyCommandSubRecord& subRecord = iter->second;
            // Write out the command string.
            stat_file.fprintf(_T(/*MSG0*/"%s\t%d\t%7i\t%12.2f\n"), cmd_name, flags, subRecord.count, subRecord.elapsedTime);
        }
    }

    int MyCommandRecord::count() const
    {
        int n = 0;
        for (auto iter = m_subRecords.begin(); iter != m_subRecords.end(); iter++)
            n += iter->second.count;
        return n;
    }

    double MyCommandRecord::elapsedTime() const
    {
        double t = 0;
        for (auto iter = m_subRecords.begin(); iter != m_subRecords.end(); iter++)
            t += iter->second.elapsedTime;
        return t;
    }


    //##################################################################################

     void MyCommandReactor::commandWillStart(const ACHAR* cmdStr)
    {
         detail::countCommand(cmdStr);
    }

    void MyCommandReactor::commandEnded(const ACHAR* cmdStr)
    {
        global_one->recordCommandDuration(cmdStr);
    }

    void MyCommandReactor::commandCancelled(const ACHAR* cmdStr)
    {
        global_one->recordCommandDuration(cmdStr);
    }

    void MyCommandReactor::commandFailed(const ACHAR* cmdStr)
    {
        global_one->recordCommandDuration(cmdStr);
    }

    void MyCommandReactor::lispWillStart(const ACHAR* firstLine)
    {
        int flags = detail::getCurrentStateFlags() | CMDSTATE_LISP;
        global_one->curDocGlobals.lispRcd = detail::bumpCount(firstLine, flags, Adesk::kTrue);
        global_one->curDocGlobals.lispFlags = flags;
        global_one->curDocGlobals.lispStartTime = detail::getCurTime();
    }

    void MyCommandReactor::lispEnded()
    {
        auto& curDocGlobals = global_one->curDocGlobals;

        if (curDocGlobals.lispRcd != NULL)
            detail::recordElapsedCmdTime(curDocGlobals.lispRcd, curDocGlobals.lispStartTime, curDocGlobals.lispFlags);
        curDocGlobals.lispRcd = NULL;
        if (global_one->bStatsFileUpdatePending && global_one->updateCumulativeStats())
            /* We have completed an operation such as SAVE or WBLOCK since
               this lisp expression began, and this is a convenient time to
               at least try to update the
               stats file with the current counts.  This must be marked pending
               and done after command completion because cmdRcd[] points to a
               CommandRecord object in *sessionStats which is deleted in
               updateCumulativeStats()called below.  updateCumulativeStats
               will return kFalse if any documents have any active command
               or LISP expressions, in which case, this will have to wait.
               JMC, WCA 7/15/98 */
            global_one->bStatsFileUpdatePending = Adesk::kFalse;
    }

    void MyCommandReactor::lispCancelled()
    {
        lispEnded();  // Same thing, really.
    }

    void MyCommandReactor::beginSave(AcDbDatabase* pDwg, const ACHAR* pIntendedName)
    {
        global_one->bStatsFileUpdatePending = Adesk::kTrue;
    }

    void MyCommandReactor::beginWblock(AcDbDatabase* pTo, AcDbDatabase* pFrom, const AcGePoint3d*& insertionPoint)
    {
        global_one->bStatsFileUpdatePending = Adesk::kTrue;
    }

    void MyCommandReactor::beginWblock(AcDbDatabase* pTo, AcDbDatabase* pFrom, AcDbObjectId blockId)
    {
        global_one->bStatsFileUpdatePending = Adesk::kTrue;
    }

    void MyCommandReactor::beginWblock(AcDbDatabase* pTo, AcDbDatabase* pFrom)
    {
        global_one->bStatsFileUpdatePending = Adesk::kTrue;
    }

    //##################################################################################


    MyCmdCountDocData::MyCmdCountDocData() : doc(NULL), cmdIndex(0), lispRcd(NULL), lispStartTime(0), lispFlags(0)
    {
    }

    MyCmdCountDocData::MyCmdCountDocData(const MyCmdCountDocData& src)
    {
        operator=(src);
    }

    MyCmdCountDocData& MyCmdCountDocData::operator=(const MyCmdCountDocData& src)
    {
        doc = src.doc;

        cmdIndex = src.cmdIndex;
        int i;
        for (i = 0; i < cmdIndex; i++) {
            cmdRcd[i] = src.cmdRcd[i];
            cmdStartTime[i] = src.cmdStartTime[i];
            cmdFlags[i] = src.cmdFlags[i];
        }
        lispRcd = src.lispRcd;
        lispStartTime = src.lispStartTime;
        lispFlags = src.lispFlags;
        return *this;
    }

    void MyCmdCountDocData::recordAndPop()
    {
        cmdIndex--;

        // Record the time spent in the ended command
        detail::recordElapsedCmdTime(cmdRcd[cmdIndex],
            cmdStartTime[cmdIndex],
            cmdFlags[cmdIndex]
        );
        // restart the clock in the suspended command, if there is one.
        if (cmdIndex > 0)
            cmdStartTime[cmdIndex - 1] = detail::getCurTime();
    }


    //##################################################################################

    void cmd_count::stop() {
        // If the current document is still valid, record what it was
        // last doing.  It should be gone, however.
        if (curDocGlobals.doc != NULL)
            pDocReactor->documentToBeDestroyed(curDocGlobals.doc);
        acDocManager->removeReactor(pDocReactor);
        delete pDocReactor;

        // Add 'em in, if you can.
        updateCumulativeStats();
        delete cumulativeStats;
        cumulativeStats = NULL;

        acedEditor->removeReactor(cmdr);
        delete cmdr;

        acedRegCmds->removeGroup(ACRX_T(/*NOXLATE*/"WZJ_COMMAND_CMD_COUNT"));
    }

    void cmd_count::init() {
        cmdr = new MyCommandReactor;
        statFileName[0] = EOS;
        
        sessionStats = detail::initStatDictionary();

        acedEditor->addReactor(cmdr);
        acedRegCmds->addCommand(_T("WZJ_COMMAND_CMD_COUNT"),_T("GLOBAL_CMDSTAT"), _T("LOCAL_CMDSTAT"), ACRX_CMD_MODAL, &detail::commandStats);
        acedRegCmds->addCommand(_T("WZJ_COMMAND_CMD_COUNT"),_T("GLOBAL_CMDCOUNT"),_T("LOCAL_CMDCOUNT"),ACRX_CMD_MODAL, &detail::cmdCommandCount);

        bStatsFileUpdatePending = Adesk::kFalse;

        // Initialize Global struct array
        docData.setLogicalLength(0);

        // Fill array from existing documents
        AcApDocumentIterator* pDocIter = acDocManager->newAcApDocumentIterator();

        for (; !pDocIter->done(); pDocIter->step())
            // add an entry for the document, if some other notification hasn't already done so.
            lookupDoc(pDocIter->document());

        delete pDocIter;

        // Establish current document, if there is one yet.
        if ((docData.length() > 0) && (acDocManager->curDocument() != NULL))
            curDocGlobals = docData[lookupDoc(acDocManager->curDocument())];
        else
            curDocGlobals.doc = NULL;

        pDocReactor = new MyDocReactor;
        acDocManager->addReactor(pDocReactor);

        detail::updateRegistry(); // 写注册表, 用于demand load机制
    }

    int cmd_count::lookupDoc(AcApDocument* pDoc) {
        if (pDoc == nullptr)
            return -1;

        for (int i = 0; i < docData.length(); i++)
            if (docData[i].doc == pDoc)
                return i;

        MyCmdCountDocData new_data;
        new_data.doc = pDoc;
        docData.append(new_data);

        return docData.length() - 1;
    }

    void cmd_count::recordCommandDuration(const TCHAR* pszCmdStr) {
        if (curDocGlobals.doc == NULL)
            return;

        // Verify that the command ending is the one on the top of the stack in the current
        // document.  If not, go fish through other documents.  If we are still empty handed,
        // give up.
        MyCommandRecord* curCmdRcd = NULL;
        if (pszCmdStr != NULL)
            curCmdRcd = (MyCommandRecord*)sessionStats->at(pszCmdStr);
        else if (curDocGlobals.cmdIndex > 0)
            curCmdRcd = curDocGlobals.cmdRcd[curDocGlobals.cmdIndex - 1];
        bool swappedCurDoc = false;

        if ((curDocGlobals.cmdIndex <= 0) || (curCmdRcd != curDocGlobals.cmdRcd[curDocGlobals.cmdIndex - 1])) {
            bool foundDoc = false;
            int i = 0;
            for (; !foundDoc && (i < docData.length()); i++) {
                foundDoc = (docData[i].cmdIndex > 0) &&
                    (docData[i].cmdRcd[docData[i].cmdIndex - 1] == curCmdRcd);
            }
            if (foundDoc)
                docData[i - 1].recordAndPop();
        }
        else {
            if (curDocGlobals.cmdIndex > 0)
                curDocGlobals.recordAndPop();
            int i = lookupDoc(curDocGlobals.doc);
            assert(i >= 0);
            docData[i] = curDocGlobals;
        }
        // Active command stack has been fully popped and we have a pending
        // statistics file update?
        if (bStatsFileUpdatePending && updateCumulativeStats())
            /* We have completed an operation such as SAVE or WBLOCK since
               this lisp expression began, and this is a convenient time to
               at least try to update the
               stats file with the current counts.  This must be marked pending
               and done after command completion because cmdRcd[] points to a
               CommandRecord object in *sessionStats which is deleted in
               updateCumulativeStats()called below.  updateCumulativeStats
               will return kFalse if any documents have any active command
               or LISP expressions, in which case, this will have to wait.
               JMC, WCA 7/15/98 */
            bStatsFileUpdatePending = Adesk::kFalse;
    }

    Adesk::Boolean cmd_count::updateCumulativeStats()
    {
        // If anything is going on in any document, now is NOT a good
        // time to accumulate stats, because each command/LISP expression
        // in progress has pointers into session stats.
        // This could be fixed with more recoding than I have time to do
        // now.  WCA 7/15/98
        for (int i = docData.length() - 1; i >= 0; i--) {
            // Make sure our info is up to date.
            if (docData[i].doc == curDocGlobals.doc)
                docData[i] = curDocGlobals;
            if ((docData[i].cmdIndex > 0) ||
                (docData[i].lispRcd != NULL))
                return Adesk::kFalse;
        }

        if (!readCumulativeStats()) {
            acutPrintf(ACRX_T("\nWarning: Could not find Command Statistics file.\n"));
            acutPrintf(ACRX_T("Will try to create it.\n"));
        }

        AcRxDictionaryIterator* iter;

        // Loop over current session stats, and merge them into cumulative stats.
        if (sessionStats) {
            for (iter = sessionStats->newIterator(); !iter->done(); iter->next()) {
                MyCommandRecord* sessionCmdRcd = (MyCommandRecord*)iter->object();
                MyCommandRecord* cumulativeCmdRcd = (MyCommandRecord*)cumulativeStats->at(iter->key());
                if (!cumulativeCmdRcd) {
                    // First time, add it.
                    cumulativeCmdRcd = new MyCommandRecord();
                    cumulativeStats->atPut(iter->key(), cumulativeCmdRcd);
                }
                // add the values
                cumulativeCmdRcd->add(sessionCmdRcd);
            }

            delete iter;
        }

        // Now that it has been added in, wipe out the current session Stats;
        delete sessionStats;
        sessionStats = detail::initStatDictionary();

        // Open the cumulative stat file, creating it if necessary.
        ACHAR statFilePath[512];

        _tcscpy(statFilePath, detail::cmdcount_filename());
        assert(_tcslen(statFilePath) > 0);//this should always succeed

        // Open the file
        AcFILE statFile;
        statFile.fopen(statFilePath, ACRX_T(/*NOXLATE*/"w"));

        if (!statFile.isOpen()) {
            // Bad permission in our chosen directory.  Give up.
            acedAlert(ACRX_T(/*MSGO*/"Warning: Could not create Command Statistics file."));
            return Adesk::kTrue;
        }

        // Print version number of STATFILE
        statFile.fprintf(ACRX_T(/*MSGO*/"v%04.1f\n"), STAT_FILENAME_VERSION);

        // Print create date of STATFILE
        if (!*createDate) {
            statFile.fprintf(ACRX_T(/*MSGO*/"Created:               "));
            detail::printCurrentDateTime(statFile);
            statFile.fprintf(ACRX_T(/*MSGO*/"\n"));
        }
        else
            statFile.fprintf(ACRX_T(/*MSGO*/"%s\n"), createDate);

        // Print date last modified for STATFILE
        statFile.fprintf(ACRX_T(/*MSGO*/"Last Modified:         "));
        detail::printCurrentDateTime(statFile);
        statFile.fprintf(ACRX_T(/*MSGO*/"\n"));

        resbuf tempRes;
        // Print LOGINNAME
        if (acedGetVar(ACRX_T(/*NOXLATE*/"LOGINNAME"), &tempRes) != RTNORM) {
            _stprintf(abort_msg, ACRX_T("%.*s Command\nStatistics Gatherer\nFailure 1"),
                PRODUCTL, detail::getProductName());
            acrx_abort(abort_msg);
        }
        statFile.fprintf(ACRX_T(/*NOXLATE*/"LOGINNAME:             %s\n"), tempRes.resval.rstring);
        acdbFree(tempRes.resval.rstring);
        // Print ACAD serial number
        if (acedGetVar(ACRX_T(/*NOXLATE*/"_PKSER"), &tempRes) != RTNORM) {
            _stprintf(abort_msg, ACRX_T("%.*s Command\nStatistics Gatherer\nFailure 1"),
                PRODUCTL, detail::getProductName());
            acrx_abort(abort_msg);
        }
        statFile.fprintf(ACRX_T(/*NOXLATE*/"%.*s Serial Number: %s\n"), PRODUCTL, detail::getProductName(),
            tempRes.resval.rstring);
        acdbFree(tempRes.resval.rstring);

        // Print ACAD version
        if (acedGetVar(ACRX_T(/*NOXLATE*/"_VERNUM"), &tempRes) != RTNORM) {
            _stprintf(abort_msg, ACRX_T("%.*s Command\nStatistics Gatherer\nFailure 1"),
                PRODUCTL, detail::getProductName());
            acrx_abort(abort_msg);
        }
        statFile.fprintf(ACRX_T(/*NOXLATE*/"%.*s version:       %s\n"), PRODUCTL, detail::getProductName(),
            tempRes.resval.rstring);
        acdbFree(tempRes.resval.rstring);


        for (iter = cumulativeStats->newIterator(AcRx::kDictSorted); !iter->done(); iter->next()) {
            MyCommandRecord* pRec = ((MyCommandRecord*)iter->object());
            if (pRec != NULL)
                pRec->write(iter->key(), statFile);
        }

        statFile.fclose();

        delete iter;
        return Adesk::kTrue;
    }

    Adesk::Boolean cmd_count::readCumulativeStats() {
        // If we got any hanging around, get rid of 'em.
        if (cumulativeStats)
            delete cumulativeStats;

        cumulativeStats = detail::initStatDictionary();

        // Open the file
        AcFILE statFile;
        statFile.fopen(detail::cmdcount_filename(), ACRX_T(/*NOXLATE*/"r"));
        if (!statFile.isOpen()) {
            *createDate = NULL;
            return Adesk::kFalse; // Most likely, it hasn't been created yet.
                                  // one other likely reason is 
                                  // that another process has it open for write,
                                  // but we're talkin' a millisecond window...
        }

        ACHAR cmdName[MAX_CMD_LENGTH + 1 /*for the EOS*/];
        int cmdCount;
        double cmdElapsedTime;

        ACHAR dummyString[MAX_STR_LENGTH + 1];
        ACHAR newline[MAX_STR_LENGTH + 1];

        ACHAR versionString[VERSION_LENGTH + 1];

        // Read line containing STAT_FILENAME_VERSION
        statFile.fgets(newline, MAX_STR_LENGTH);
        _stscanf_s(newline, ACRX_T("%s"), versionString);



        ACHAR stat_buf[10];
        _stprintf(stat_buf, ACRX_T("v0%1.1f"), STAT_FILENAME_VERSION);
        if (_tcscmp(versionString, stat_buf) != 0)
        {

            // Whoops, you've got an old version of the statfile...
            acutPrintf(ACRX_T("\nWarning: Incompatible version of STATFILE.\n"));
            statFile.fclose();
            return Adesk::kFalse;
        }

        // Read line containing Date File Created
        statFile.fgets(newline, MAX_STR_LENGTH);
        _stscanf_s(newline, ACRX_T("\n%[^\n]"), createDate);

        // Read line containing Date File Last Modified
        statFile.fgets(newline, MAX_STR_LENGTH);
        _stscanf_s(newline, ACRX_T("\n%[^\n]"), dummyString);

        // Read line containing LOGINNAME 
        statFile.fgets(newline, MAX_STR_LENGTH);
        _stscanf_s(newline, ACRX_T("\n%[^\n]"), dummyString);

        // Read line containing AutoCAD serial number
        statFile.fgets(newline, MAX_STR_LENGTH);
        _stscanf_s(newline, ACRX_T("\n%[^\n]"), dummyString);

        // Read line containing AutoCAD version
        statFile.fgets(newline, MAX_STR_LENGTH);
        _stscanf_s(newline, ACRX_T("\n%[^\n]"), dummyString);

        int flags;
        while (statFile.fgets(newline, MAX_STR_LENGTH) != NULL)
        {
            _stscanf_s(newline, ACRX_T("%s %i %i %le"), cmdName, &flags, &cmdCount, &cmdElapsedTime);

            // get the record for this command, or create it on the fly if needed.
            // then add the data: flags, count, time
            MyCommandRecord* pRecord = (MyCommandRecord*)cumulativeStats->at(cmdName);
            if (pRecord == NULL) {
                pRecord = new MyCommandRecord();
                if (pRecord == NULL)
                    break;
                cumulativeStats->atPut(cmdName, pRecord);
            }
            pRecord->add(flags, cmdCount, cmdElapsedTime);
        }
        statFile.fclose();
        return Adesk::kTrue;
    }



    //##################################################################################



  
   
    

}