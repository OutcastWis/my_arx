﻿// MyAcUiDialog.cpp: 实现文件
//

#include "pch.h"
#include "MyAcUiDialog.h"
#include "resource.h"

#define PI  3.14159265359

// MyAcUiDialog 对话框


MyAcUiDialog::MyAcUiDialog(CWnd* pParent /*=nullptr*/)
	: CAcUiDialog(IDD_DIALOG1, pParent)
{

}

MyAcUiDialog::~MyAcUiDialog()
{
}

void MyAcUiDialog::DisplayPoint()
{
    m_ctrlXPtEdit.SetWindowText(m_strXPt);
    m_ctrlXPtEdit.Convert();

    m_ctrlYPtEdit.SetWindowText(m_strYPt);
    m_ctrlYPtEdit.Convert();

    m_ctrlZPtEdit.SetWindowText(m_strZPt);
    m_ctrlZPtEdit.Convert();
}

bool MyAcUiDialog::ValidatePoint()
{
    return m_ctrlXPtEdit.Validate() & m_ctrlYPtEdit.Validate() & m_ctrlZPtEdit.Validate();
}

void MyAcUiDialog::DisplayAngle()
{
    m_ctrlAngleEdit.SetWindowText(m_strAngle);
    m_ctrlAngleEdit.Convert();
}

bool MyAcUiDialog::ValidateAngle()
{
    return m_ctrlAngleEdit.Validate();
}

void MyAcUiDialog::DisplayBlocks()
{
    AcDbBlockTable* pBlockTable;
    acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);

    // Iterate through the block table and disaply the names in the list box.
    const TCHAR* pName;
    AcDbBlockTableIterator* pBTItr;
    if (pBlockTable->newIterator(pBTItr) == Acad::eOk) {
        while (!pBTItr->done()) {
            AcDbBlockTableRecord* pRecord;
            if (pBTItr->getRecord(pRecord, AcDb::kForRead) == Acad::eOk) {
                pRecord->getName(pName);
                m_ctrlBlockListBox.InsertString(-1, pName);
                pRecord->close();
            }
            pBTItr->step();
        }
    }
    pBlockTable->close();
}

void MyAcUiDialog::DisplayRegApps()
{
    AcDbRegAppTable* pRegAppTable;
    acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pRegAppTable, AcDb::kForRead);

    // Iterate through the reg app table and disaply the names in the list box.
    const TCHAR* pName;
    AcDbRegAppTableIterator* pItr;
    if (pRegAppTable->newIterator(pItr) == Acad::eOk) {
        while (!pItr->done()) {
            AcDbRegAppTableRecord* pRecord;
            if (pItr->getRecord(pRecord, AcDb::kForRead) == Acad::eOk) {
                pRecord->getName(pName);
                m_ctrlRegAppComboBox.InsertString(-1, pName);
                pRecord->close();
            }
            pItr->step();
        }
    }
    pRegAppTable->close();
}

void MyAcUiDialog::DoDataExchange(CDataExchange* pDX)
{
	CAcUiDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(AsdkAcUiDialogSample)
    DDX_Control(pDX, IDC_COMBO_REGAPPS, m_ctrlRegAppComboBox);
    DDX_Control(pDX, IDC_LIST_BLOCKS, m_ctrlBlockListBox);
    DDX_Control(pDX, IDC_BUTTON_POINT, m_ctrlPickButton);
    DDX_Control(pDX, IDC_BUTTON_ANGLE, m_ctrlAngleButton);
    DDX_Control(pDX, IDC_EDIT_ANGLE, m_ctrlAngleEdit);
    DDX_Control(pDX, IDC_EDIT_XPT, m_ctrlXPtEdit);
    DDX_Control(pDX, IDC_EDIT_YPT, m_ctrlYPtEdit);
    DDX_Control(pDX, IDC_EDIT_ZPT, m_ctrlZPtEdit);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MyAcUiDialog, CAcUiDialog)
    //{{AFX_MSG_MAP(AsdkAcUiDialogSample)
    ON_BN_CLICKED(IDC_BUTTON_POINT, OnButtonPoint)
    ON_BN_CLICKED(IDC_BUTTON_ANGLE, OnButtonAngle)
    ON_CBN_KILLFOCUS(IDC_COMBO_REGAPPS, OnKillfocusComboRegapps)
    ON_EN_KILLFOCUS(IDC_EDIT_ANGLE, OnKillfocusEditAngle)
    ON_EN_KILLFOCUS(IDC_EDIT_XPT, OnKillfocusEditXpt)
    ON_EN_KILLFOCUS(IDC_EDIT_YPT, OnKillfocusEditYpt)
    ON_EN_KILLFOCUS(IDC_EDIT_ZPT, OnKillfocusEditZpt)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


// MyAcUiDialog 消息处理程序


void MyAcUiDialog::OnButtonPoint()
{
    // Hide the dialog and give control to the editor
    BeginEditorCommand();

    ads_point pt;

    // Get a point
    if (acedGetPoint(NULL, _T("\nPick a point: "), pt) == RTNORM) {
        // If the point is good, continue
        CompleteEditorCommand();
        m_strXPt.Format(_T("%g"), pt[X]);
        m_strYPt.Format(_T("%g"), pt[Y]);
        m_strZPt.Format(_T("%g"), pt[Z]);
        DisplayPoint();
    }
    else {
        // otherwise cancel the command (including the dialog)
        CancelEditorCommand();
    }
}

BOOL MyAcUiDialog::OnInitDialog()
{
    // Set the dialog name for registry lookup and storage
    // 注册表位置: HKEY_CURRENT_USER\SOFTWARE\Autodesk\AutoCAD\R22.0\...\Profiles\...\Dialogs\ 下
    SetDialogName(_T("arx2018:MyAcUiDialog"));

    CAcUiDialog::OnInitDialog();

    // 控制子控件的移动
    DLGCTLINFO  dlgSizeInfo[] = {
        { IDC_STATIC_GROUP1, ELASTICX, 20 },
        { IDC_STATIC_GROUP1, ELASTICY, 100 },
        { IDC_EDIT_XPT, ELASTICX, 20 },
        { IDC_EDIT_YPT, ELASTICX, 20 },
        { IDC_EDIT_ZPT, ELASTICX, 20 },
        { IDC_EDIT_ANGLE, ELASTICX, 20 },

        { IDC_STATIC_GROUP2, MOVEX, 20 },
        { IDC_STATIC_GROUP2, ELASTICY, 100 },
        { IDC_STATIC_GROUP2, ELASTICX, 80 },
        { IDC_LIST_BLOCKS, MOVEX, 20 },
        { IDC_LIST_BLOCKS, ELASTICY, 100 },
        { IDC_STATIC_TEXT2, MOVEX, 20 },
        { IDC_STATIC_TEXT2, MOVEY, 100 },
        { IDC_LIST_BLOCKS, ELASTICX, 80 },
        { IDC_STATIC_TEXT2, ELASTICX, 80 },

        { IDC_STATIC_GROUP3, MOVEY, 100 },
        { IDC_STATIC_GROUP3, ELASTICX, 20 },
        { IDC_COMBO_REGAPPS, MOVEY, 100 },
        { IDC_COMBO_REGAPPS, ELASTICX, 20 },
        { IDC_STATIC_TEXT3, MOVEY, 100 },
        { IDC_STATIC_TEXT3, ELASTICX, 20 },

        { IDOK, MOVEX, 100 },
        { IDCANCEL, MOVEX, 100 },
    };
    const DWORD numberofentries = sizeof dlgSizeInfo / sizeof DLGCTLINFO;
    SetControlProperty(dlgSizeInfo, numberofentries);

    // Must be within a 100 unit cube centered about 0,0,0
    m_ctrlXPtEdit.SetRange(-50.0, 50.0);
    m_ctrlYPtEdit.SetRange(-50.0, 50.0);
    m_ctrlZPtEdit.SetRange(-50.0, 50.0);

    // Must be between 0 and 90 degrees
    m_ctrlAngleEdit.SetRange(0.0, 90.0 /*(PI/2.0)*/);

    // Assign a title for the dialog
    SetWindowText(_T("MyAcUiDialog Sample"));

    // Load the default bitmaps
    m_ctrlPickButton.AutoLoad();
    m_ctrlAngleButton.AutoLoad();

    // Get and Display the preserved data from the registry
    if (!GetDialogData(_T("ANGLE"), m_strAngle))
        m_strAngle = _T("0.0");
    if (!GetDialogData(_T("POINTX"), m_strXPt))
        m_strXPt = _T("0.0");
    if (!GetDialogData(_T("POINTY"), m_strYPt))
        m_strYPt = _T("0.0");
    if (!GetDialogData(_T("POINTZ"), m_strZPt))
        m_strZPt = _T("0.0");

    DisplayPoint();
    DisplayAngle();
    DisplayBlocks();
    DisplayRegApps();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void MyAcUiDialog::OnButtonAngle()
{
    BeginEditorCommand();

    // Setup the default point for picking an angle
    // based on the m_strXPt, m_strYPt and m_strZPt values
    ads_point pt;
    acdbDisToF(m_strXPt, -1, &pt[X]);
    acdbDisToF(m_strYPt, -1, &pt[Y]);
    acdbDisToF(m_strZPt, -1, &pt[Z]);

    double angle;

    // Get a point from the user
    if (acedGetAngle(pt, _T("\nPick an angle: "), &angle) == RTNORM) {
        // If we got an angle, go back to the dialog
        CompleteEditorCommand();
        // Convert the acquired radian value to degrees since the 
        // AcUi control can convert that to the other formats.
        m_strAngle.Format(_T("%g"), angle * (180.0 / PI));
        DisplayAngle();
    }
    else {
        // otherwise cancel the command (including the dialog)
        CancelEditorCommand();
    }
}

void MyAcUiDialog::OnKillfocusComboRegapps()
{
    CString strFromEdit;
    m_ctrlRegAppComboBox.GetWindowText(strFromEdit);
    if (m_ctrlRegAppComboBox.FindString(-1, strFromEdit) == CB_ERR) {
        if (acdbRegApp(strFromEdit) == RTNORM)
            m_ctrlRegAppComboBox.AddString(strFromEdit);
    }
}

void MyAcUiDialog::OnOK()
{
    if (!ValidatePoint()) {
        AfxMessageBox(_T("Sorry, Point out of desired range. Try again or Cancel"));
        m_ctrlXPtEdit.SetFocus();
        return;
    }

    if (!ValidateAngle()) {
        AfxMessageBox(_T("Sorry, Angle out of desired range. Try again or Cancel"));
        m_ctrlAngleEdit.SetFocus();
        return;
    }

    CAcUiDialog::OnOK();

    // Store the data into the registry
    SetDialogData(_T("ANGLE"), m_strAngle);
    SetDialogData(_T("POINTX"), m_strXPt);
    SetDialogData(_T("POINTY"), m_strYPt);
    SetDialogData(_T("POINTZ"), m_strZPt);
}

void MyAcUiDialog::OnKillfocusEditAngle()
{
    // Get and update text the user typed in.
    m_ctrlAngleEdit.Convert();
    m_ctrlAngleEdit.GetWindowText(m_strAngle);
}

void MyAcUiDialog::OnKillfocusEditXpt()
{
    // Get and update text the user typed in.
    m_ctrlXPtEdit.Convert();
    m_ctrlXPtEdit.GetWindowText(m_strXPt);
}

void MyAcUiDialog::OnKillfocusEditYpt()
{
    // Get and update text the user typed in.
    m_ctrlYPtEdit.Convert();
    m_ctrlYPtEdit.GetWindowText(m_strYPt);
}

void MyAcUiDialog::OnKillfocusEditZpt()
{
    // Get and update text the user typed in.
    m_ctrlZPtEdit.Convert();
    m_ctrlZPtEdit.GetWindowText(m_strZPt);
}
