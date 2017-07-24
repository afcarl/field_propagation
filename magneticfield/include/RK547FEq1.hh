#ifndef RK547FEq1_HH
#define RK547FEq1_HH

#include "G4MagIntegratorStepper.hh"
#include "G4FieldTrack.hh"

class RK547FEq1 : public G4MagIntegratorStepper {
public:
    RK547FEq1(G4EquationOfMotion* EqRhs, G4int integrationVariables = 6);

    virtual void Stepper(
        const G4double yInput[],
        const G4double dydx[],
        G4double hstep,
        G4double yOutput[],
        G4double yError[]) override;

    void Stepper(
        const G4double yInput[],
        const G4double dydx[],
        G4double hstep,
        G4double yOutput[],
        G4double yError[],
        G4double dydxOutput[]);

    RK547FEq1(const RK547FEq1&) = delete;
    RK547FEq1& operator = (const RK547FEq1&) = delete;

    virtual G4double DistChord() const override;
    virtual G4int IntegratorOrder() const override { return 4; }

private:
    void makeStep(
        const G4double yInput[],
        const G4double dydx[],
        const G4double hstep,
        G4double yOutput[],
        G4double* dydxOutput = nullptr,
        G4double* yError = nullptr) const;

    G4double fyIn[G4FieldTrack::ncompSVEC],
             fdydx[G4FieldTrack::ncompSVEC],
             fyOut[G4FieldTrack::ncompSVEC],
             fdydxOut[G4FieldTrack::ncompSVEC];
    G4double fhstep;
};

#endif
