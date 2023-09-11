#pragma once

#include <map>

#include <aced.h>
#include <AcApDMgr.h>

#include "icmd.h"


namespace wzj {

    class MyDocReactor : public AcApDocManagerReactor
    {
    public:
        virtual void documentCreated(AcApDocument* pDoc);
        virtual void documentToBeActivated(AcApDocument* pDoc);
        virtual void documentToBeDestroyed(AcApDocument* pDoc);

    };


    class MyCommandSubRecord : public AcRxObject {
    public:
        MyCommandSubRecord() = default;
        int count = 0;
        double elapsedTime = 0;
    };

    class MyCommandRecord : public AcRxObject {
    public:
        void add(int flags, int count, double time);
        void add(const MyCommandRecord* record);

        void write(LPCTSTR cmd_name, FILE* stat_file);

        int count() const;
        double elapsedTime() const;

    protected:
        // {stat, record}. stat是命令行状态, 由detail::getCurrentStateFlags()所得
        std::map<int, MyCommandSubRecord> m_subRecords;
    };


    class MyCommandReactor : public AcEditorReactor {
    public:
        virtual void commandWillStart(const ACHAR* cmdStr)override ;
        virtual void commandEnded(const ACHAR* cmdStr)override ;
        virtual void commandCancelled(const ACHAR* cmdStr)override ;
        virtual void commandFailed(const ACHAR* cmdStr)override ;

        virtual void lispWillStart(const ACHAR* firstLine) override;
        virtual void lispEnded()override;
        virtual void lispCancelled()override;

        virtual void beginSave(AcDbDatabase* pDwg, const ACHAR* pIntendedName)override;

        virtual void beginWblock(AcDbDatabase* pTo, AcDbDatabase* pFrom,const AcGePoint3d*& insertionPoint)override;
        virtual void beginWblock(AcDbDatabase* pTo,AcDbDatabase* pFrom, AcDbObjectId blockId)override;
        virtual void beginWblock(AcDbDatabase* pTo, AcDbDatabase* pFrom)override;
    };

    struct MyCmdCountDocData {
        MyCmdCountDocData();
        MyCmdCountDocData(const MyCmdCountDocData&);
        MyCmdCountDocData& operator=(const MyCmdCountDocData&);

        void recordAndPop();

        AcApDocument* doc;

        // handle "nested" commands. Max depth is 5
        int                cmdIndex;   // To handle "nested" commands
        MyCommandRecord* cmdRcd[5];
        double             cmdStartTime[5];
        long               cmdFlags[5];

        // LISP is its own, non-reentrant, beast.
        MyCommandRecord* lispRcd;
        double             lispStartTime;
        long               lispFlags;
    };

    class cmd_count : public icmd {
    public:
        static cmd_count* instance() {
            static cmd_count one;
            return &one;
        }

        const TCHAR* name() const override {
            return _T("cmd_count");
        }

        void stop_impl() override;

        void init_impl() override;
        // 在docData中寻找pDoc, 若不存在, 则加入. 返回索引
        int lookupDoc(AcApDocument* pDoc);

        void recordCommandDuration(const TCHAR* pszCmdStr);

        Adesk::Boolean updateCumulativeStats();

        Adesk::Boolean readCumulativeStats();


    private:
        cmd_count() = default;

    public:
        // Gets notification of command and LISP evaluation events. 
        // Always in use.
        // MDI-Safe
        MyCommandReactor* cmdr = NULL;


        // Running statistics for this AutoCAD session, or since last query.
        // Always in use.
        // MDI-Safe
        AcRxDictionary* sessionStats = NULL; 

        /* Cumulative Invocation counts.  All
          queries of cumulative stats are
          preceded by a merge, and resetting
          of session stats.  Active only
          during query and update operations.
          NULL otherwise. */
        // MDI-Safe
        AcRxDictionary* cumulativeStats = NULL;

        // create data for cumulative file. MDI-Safe
        TCHAR createDate[1024];
        // Name of stat file. "+5" means one for EOS, four for .txt. MDI-Safe
        TCHAR statFileName[1024 + 5];

        MyCmdCountDocData curDocGlobals; // MDI-Safe

        // The entire array of document-realted global structs.
        AcArray<MyCmdCountDocData>    docData; // MDI-Safe

        // Indicates that a SAVE/WBLOCK has occurred, which we take as a signal to
        // save the stats file also, after waiting for active commands to finish up
        // JMC
        Adesk::Boolean bStatsFileUpdatePending; // MDI-Safe
       

        MyDocReactor* pDocReactor;
    };
}