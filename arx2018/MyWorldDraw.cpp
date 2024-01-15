#include "pch.h"
#include "MyWorldDraw.h"

#include "MyGiContext.h"

MyShellData::MyShellData()
    : mpVertexList(NULL),
    mpEdgeData(NULL),
    mpFaceData(NULL),
    mpFaceList(NULL),
    mpVertexData(NULL)
{
}

MyShellData::~MyShellData()
{
    delete[] mpVertexList;
    delete mpEdgeData;
    delete mpFaceData;
    delete[] mpFaceList;
    delete mpVertexData;
}

Acad::ErrorStatus MyShellData::setFaceData(int colorIndex) {
    auto nf = numberOfFaces();
    short* pColors = new short[nf];
    for (auto i = 0; i < nf; ++i)
        pColors[i] = colorIndex;

    mpFaceData = new AcGiFaceData;
    mpFaceData->setColors(pColors);

    return Acad::eOk;
}

unsigned int MyShellData::numberOfFaces() {
    unsigned int numFaces = 0;
    Adesk::Int32* pTempFaceList = mpFaceList;

    for (unsigned int i = 0; i < mFaceListSize; ) {
        ++numFaces;
        unsigned int facePoints = *pTempFaceList;
        pTempFaceList += (facePoints + 1);
        i += (facePoints + 1);
    }
    return numFaces;
}

unsigned int MyShellData::numberOfEdges() {
    int numEdges = 0;

    for (unsigned int i = 0; i < mFaceListSize; ) {
        numEdges += mpFaceList[i];
        i += mpFaceList[i + 1];
    }
    return numEdges;
}




ACRX_NO_CONS_DEFINE_MEMBERS(MyWorldDraw, AcGiWorldDraw)

MyWorldDraw::MyWorldDraw()
    : mDeviation(0.1)
{
    ads_printf(_T("\nCustomWorldDraw::CustomWorldDraw"));
    mpShellData = new MyShellData;
    mpContext = new MyGiContext;
    mpSubEntityTraits = new MyCustomSubEntityTraits;
    mpWorldGeometry = new MyCustomWorldGeometry(this);
}

MyWorldDraw::~MyWorldDraw() {
    delete mpSubEntityTraits;
    delete mpWorldGeometry;
    delete mpContext;
    // Once the worldDraw object gets ShellData, the custom
    // draw entity will hold its pointer.  Hence mpShellData
    // is not deleted here, it is deleted by the CustomDrawEntity's
    // destructor.
}

AcGiRegenType MyWorldDraw::regenType() const
{
    ads_printf(_T("\nCustomWorldDraw::regenType"));
    return kAcGiHideOrShadeCommand;
}

Adesk::Boolean MyWorldDraw::regenAbort() const
{
    ads_printf(_T("\nCustomWorldDraw::regenAbort"));
    return Adesk::kFalse;
}

AcGiSubEntityTraits& MyWorldDraw::subEntityTraits() const
{
    ads_printf(_T("\nCustomWorldDraw::subEntityTraits"));
    return *mpSubEntityTraits;
}

AcGiGeometry* MyWorldDraw::rawGeometry() const
{
    ads_printf(_T("\nCustomWorldDraw::rawGeometry"));
    return mpWorldGeometry;
}

AcGiWorldGeometry& MyWorldDraw::geometry() const
{
    ads_printf(_T("\nCustomWorldDraw::geometry"));
    return *mpWorldGeometry;
}

Adesk::Boolean MyWorldDraw::isDragging() const
{
    ads_printf(_T("\nCustomWorldDraw::isDragging"));
    return Adesk::kFalse;
}

double MyWorldDraw::deviation(const AcGiDeviationType, const AcGePoint3d&) const
{
    ads_printf(_T("\nCustomWorldDraw::deviation"));
    return mDeviation;
}

Adesk::UInt32 MyWorldDraw::numberOfIsolines() const
{
    ads_printf(_T("\nCustomWorldDraw::numberOfIsolines"));
    return 1;
}

AcGiContext* MyWorldDraw::context()
{
    ads_printf(_T("\nCustomWorldDraw::context"));
    return mpContext;
}

