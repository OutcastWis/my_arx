#pragma once

#include <acgi.h>

class MyShellData {

    //  Class to store the trapped shell Data.  A pointer to this
    //  class is obtained from the MyWorldDraw class which
    //  fills the data for this class.

public:
    MyShellData();
    ~MyShellData();

    //  setFaceData function will allocate memory for the mpFaceData
    //  member.
    Acad::ErrorStatus setFaceData(int colorIndex);

    unsigned int    numberOfFaces();
    unsigned int    numberOfEdges();

    //  Member Data

    Adesk::UInt32       mNumVertices;
    AcGePoint3d* mpVertexList;
    Adesk::UInt32       mFaceListSize;
    Adesk::Int32* mpFaceList;
    AcGiEdgeData* mpEdgeData;
    AcGiFaceData* mpFaceData;
    AcGiVertexData* mpVertexData;
};


class MyWorldDraw : public AcGiWorldDraw
{
public:

    ACRX_DECLARE_MEMBERS(MyWorldDraw);
    MyWorldDraw();
    virtual ~MyWorldDraw();

    virtual AcGiRegenType           regenType() const;
    virtual Adesk::Boolean          regenAbort() const;
    virtual AcGiSubEntityTraits& subEntityTraits() const;
    virtual AcGiGeometry* rawGeometry() const;
    virtual AcGiWorldGeometry& geometry() const;
    virtual Adesk::Boolean          isDragging() const;
    virtual double                  deviation(const AcGiDeviationType,
        const AcGePoint3d&) const;
    virtual Adesk::UInt32           numberOfIsolines() const;
    virtual AcGiContext* context();

    MyShellData* shellData() { return mpShellData; }
    void setDeviation(double dev) { mDeviation = dev; }

    // some member data
private:
    MyShellData* mpShellData;
    AcGiSubEntityTraits* mpSubEntityTraits;
    AcGiWorldGeometry* mpWorldGeometry;
    AcGiContext* mpContext;
    double                  mDeviation;

};

