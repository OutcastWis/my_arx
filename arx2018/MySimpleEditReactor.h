#pragma once

#include <aced.h>

class MySimpleEditReactor : public AcEditorReactor
{
public:
    MySimpleEditReactor();
    ~MySimpleEditReactor();

    // the drawing database pDwg was saved to the file specified by pActualName. 
    void saveComplete(AcDbDatabase* pDwg, const TCHAR* pActualName) override;
};

