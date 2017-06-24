#ifndef UTILS_HH
#define UTILS_HH

#include "G4Types.hh"
#include "G4ThreeVector.hh"
#include "G4IntegrationObserver.hh"

namespace magneticfield {

enum class Value3D {
    Position = 0,
    Momentum = 3,
    Spin = 9
};

enum class Value1D {
    KineticEnergy = 6,
    LabTime = 7,
    ProperTime = 8
};

G4double extractValue(const G4double array[], const Value1D& value);
G4double extractValue2(const G4double array[], const Value1D& value);

G4double extractValue(const G4double array[], const Value3D& value);
G4double extractValue2(const G4double array[], const Value3D& value);

G4ThreeVector makeVector(const G4double array[], const Value3D& value);

G4double relativeError(
    const G4double y[],
    const G4double yerr[],
    const G4double hstep,
    const G4double errorTolerance,
    G4IntegrationObserver& observer);

} //magneticfield

#endif