// The Butcher table of the Higham & Hall 5(4)7 method is:
//
//   0  |
//  2/9 |      2/9
//  1/3 |      1/12	     1/4
//  1/2 |      1/8       0          3/8
//  3/5 |      91/500    -27/100    78/125    8/125
//   1  |      -11/20    27/20      12/5      -36/5    5
//   1  |      1/12      0          27/32     -4/3     125/96    5/48
//----------------------------------------------------------------------------
//             1/12      0          27/32     -4/3     125/96    5/48    0
//             2/15      0          27/80     -2/15    25/48     1/24    1/10

#include "RK547FEq1.hh"
#include "G4LineSection.hh"
#include "Utils.hh"

using namespace magneticfield;

namespace {
    void copyArray(G4double dst[], const G4double src[]) {
        memcpy(dst, src, sizeof(G4double) * G4FieldTrack::ncompSVEC);
    }
}

RK547FEq1::RK547FEq1(G4EquationOfMotion *EqRhs, G4int integrationVariables)
   : G4MagIntegratorStepper(EqRhs, integrationVariables)
{
}

void RK547FEq1::makeStep(const G4double yInput[],
                         const G4double dydx[],
                         const G4double hstep,
                         G4double yOutput[],
                         G4double* dydxOutput,
                         G4double* yError) const
{
    G4double yTemp[G4FieldTrack::ncompSVEC];
    for (int i = GetNumberOfVariables(); i < GetNumberOfStateVariables(); ++i){
        yOutput[i] = yTemp[i] = yInput[i];
    }

    G4double ak2[G4FieldTrack::ncompSVEC],
             ak3[G4FieldTrack::ncompSVEC],
             ak4[G4FieldTrack::ncompSVEC],
             ak5[G4FieldTrack::ncompSVEC],
             ak6[G4FieldTrack::ncompSVEC];

    const G4double
       b21 = 2./9.,
       b31 = 1./12., b32 = 1./4.,
       b41 = 1./8., b42 = 0., b43 = 3./8.,
       b51 = 91./500., b52 = -27./100., b53 = 78./125., b54 = 8./125.,

       b61 = -11./20., b62 = 27./20., b63 = 12./5.,
           b64 = -36./5., b65 = 5.,

       b71 = 1./12.,    b72 = 0., b73 = 27./32.,
            b74 = -4./3., b75 = 125./96., b76 = 5./48.;

    const G4double
       dc1 = b71 - 2./15.,
       dc2 = b72 - 0.,
       dc3 = b73 - 27./80.,
       dc4 = b74 + 2./15.,
       dc5 = b75 - 25./48.,
       dc6 = b76 - 1./24.,
       dc7 = 0. - 1./10.;

    //RightHandSide(yInput, dydx);
    for(int i = 0; i < GetNumberOfVariables(); ++i)
        yTemp[i] = yInput[i] + hstep * b21 * dydx[i];

    RightHandSide(yTemp, ak2);
    for(int i = 0; i < GetNumberOfVariables(); ++i)
        yTemp[i] = yInput[i] + hstep * (b31 * dydx[i] + b32 * ak2[i]);

    RightHandSide(yTemp, ak3);
    for(int i = 0;i < GetNumberOfVariables(); ++i)
        yTemp[i] = yInput[i] + hstep * (b41 * dydx[i] + b42 * ak2[i] +
                                        b43 * ak3[i]);

    RightHandSide(yTemp, ak4);
    for(int i = 0; i < GetNumberOfVariables(); ++i)
        yTemp[i] = yInput[i] + hstep * (b51 * dydx[i] + b52 * ak2[i] +
                                        b53 * ak3[i] + b54 * ak4[i]);

    RightHandSide(yTemp, ak5);
    for(int i = 0; i < GetNumberOfVariables(); ++i)
        yTemp[i] = yInput[i] + hstep * (b61 * dydx[i] + b62 * ak2[i] +
                                        b63 * ak3[i] + b64 * ak4[i] +
                                        b65 * ak5[i]);

    RightHandSide(yTemp, ak6);
    for(int i = 0; i < GetNumberOfVariables(); ++i)
        yOutput[i] = yInput[i] + hstep * (b71 * dydx[i] + b72 * ak2[i] +
                                          b73 * ak3[i] + b74 * ak4[i] +
                                          b75 * ak5[i] + b76 * ak6[i]);

    if (dydxOutput && yError) {
        RightHandSide(yOutput, dydxOutput);
        for(int i = 0; i < GetNumberOfVariables(); ++i)
            yError[i] = hstep * (dc1 * dydx[i] + dc2 * ak2[i] + dc3 * ak3[i] +
                                 dc4 * ak4[i] + dc5 * ak5[i] + dc6 * ak6[i] +
                                 dc7 * dydxOutput[i]);
    }
}

void RK547FEq1::Stepper(const G4double yInput[],
                        const G4double dydx[],
                        G4double hstep,
                        G4double yOutput[],
                        G4double yError[])
{
    copyArray(yIn_, yInput);
    copyArray(dydx_, dydx);
    hstep_ = hstep;

    makeStep(yInput, dydx, hstep, yOutput, dydxOut_, yError);

    copyArray(yOut_, yOutput);
}

void RK547FEq1::ComputeRightHandSide(const G4double y[], G4double dydx[])
{
    const G4ThreeVector yEnd = makeVector(yOut_, Value3D::Position);
    const G4ThreeVector yCurrent = makeVector(y, Value3D::Position);
    const G4double distance = (yCurrent - yEnd).mag() / hstep_;

    if (distance < 1e-3) {
        copyArray(dydx, dydxOut_);
    } else {
        G4MagIntegratorStepper::ComputeRightHandSide(y, dydx);
    }
}

G4double RK547FEq1::DistChord() const
{
    const G4ThreeVector begin = makeVector(yIn_, Value3D::Position);
    const G4ThreeVector end = makeVector(yOut_, Value3D::Position);

    G4double yMid[G4FieldTrack::ncompSVEC];
    makeStep(yIn_, dydx_, hstep_ / 2., yMid);
    const G4ThreeVector mid = makeVector(yMid, Value3D::Position);

    return G4LineSection::Distline(mid, begin, end);
}
