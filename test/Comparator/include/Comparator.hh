//
// class Comparator
//
// Class description:
//
// Comparator is the class for cross-checking Geant4 steppers accuracy.
// Their accuracy is checked by calling Compare() function. Parameter
// useDriver if true forces it to use G4MagInt_Driver, if false stepper's
// errors are not taken to account.
//
// History:
// - Created: D. Sorokin, May 11th 2016.
// --------------------------------------------------------------------

#ifndef COMPARATOR_HH
#define COMPARATOR_HH

#include "G4Types.hh"
#include "G4coutDestination.hh"
#include "G4Mag_EqRhs.hh"
#include "G4MagneticField.hh"
#include "G4MagIntegratorDriver.hh"
#include "G4Mag_UsualEqRhs.hh"

#include "G4Proton.hh"
#include "G4DynamicParticle.hh"

#include "ChordFinder.hh"
#include "G4ChordFinder.hh"
#include "BSChordFinder.hh"

#include "BulirschStoerDenseDriver.hh"
#include "BulirschStoerDriver.hh"

#include "G4ClassicalRK4.hh"

#include "G4CachedMagneticField.hh"
#include <fstream>
#undef G4MagInt_Driver
enum mode{
    Default,
    Verbose,
    Silent
};


class Comparator{
public:
    Comparator(G4DynamicParticle* pDynParticle, G4MagneticField* pfield);
    ~Comparator();

    //main functions
    void CrossCheck(const G4double testData[], const G4double refData[], G4int mode = Default);
    template <class testStepper, class refStepper>
    void Compare(const G4double stepLen, const G4int NSteps, const bool useDriver, const G4int verbosity);

    //compare with Bulirsch-Stoer driver
    template <class refStepper>
    void CompareWithBS(const G4double path, const G4int verb);

    //setters
    void setParticle(G4DynamicParticle* pDynParticle);
    void setField(G4MagneticField* pfield);
    void setPostition(const G4ThreeVector& newPos);
    void setMinDriverStep(const G4double newHmin);
    void setPrecision(const G4double newPrec);

private:

    void initTracks();
    void initEquation();


    G4FieldTrack* testTrack;
    G4FieldTrack* refTrack;
    G4MagneticField *field;
    G4Mag_UsualEqRhs *equation;

    //
    G4DynamicParticle* dynParticle;
    G4ThreeVector pos;
    G4double hmin;
    G4double precision;

    static const G4int N = G4FieldTrack::ncompSVEC;

    G4int diffSteps;
    G4double maxDiff;
};


template <class testStepper, class refStepper>
void Comparator::Compare(const G4double stepLen, const G4int NSteps, const bool useDriver, const G4int verbosity){
    G4double yIn[N],yOut[N],yErr[N],dydx[N];
    G4double yInRef[N],yOutRef[N],yErrRef[N],dydxRef[N];

    testStepper testSt(equation);
    refStepper refSt(equation);
    diffSteps = 0;
    if (verbosity == Verbose){
        G4cout<<"test position                ref position \n ";
    }

    if (useDriver){
        G4MagInt_Driver testDr(hmin,&testSt);
        G4MagInt_Driver refDr(hmin,&refSt);
        for (G4int i = 0; i < NSteps; ++i){
            testDr.AccurateAdvance(*testTrack,stepLen,precision,stepLen);
            refDr.AccurateAdvance(*refTrack,stepLen,precision,stepLen);

            testTrack->DumpToArray(yOut);
            refTrack->DumpToArray(yOutRef);

            CrossCheck(yOut,yOutRef,verbosity);
        }
    }
    else{
        for (G4int i = 0; i < NSteps; ++i){
            testTrack->DumpToArray(yIn);
            refTrack->DumpToArray(yInRef);

            testSt.RightHandSide(yIn, dydx);
            testSt.Stepper(yIn,dydx,stepLen,yOut,yErr);

            refSt.RightHandSide(yInRef, dydxRef);
            refSt.Stepper(yInRef,dydxRef,stepLen,yOutRef,yErrRef);

            testTrack->LoadFromArray(yOut,N);
            refTrack->LoadFromArray(yOutRef,N);

            CrossCheck(yOut,yOutRef,verbosity);
        }
    }

    if (verbosity == Silent){
        G4cout<<"diffSteps# "<<diffSteps<<" maxDiff "<<maxDiff<<G4endl;
    }
}

template <class refStepper>
void Comparator::CompareWithBS(const G4double path, const G4int /*verb*/)
{
    G4ClassicalRK4 rk4(equation);
    refStepper refSt(equation);
    G4VIntegrationDriver* refDriver = new G4MagInt_Driver(hmin,&refSt); //deleted by G4ChordFinder
    G4ChordFinder refChordFinder(refDriver);
    G4VIntegrationDriver* BSDriver = new BulirschStoerDriver(hmin,equation);
    //G4VIntegrationDriver* BSDriver = new G4MagInt_Driver(hmin,&rk4);
    //BulirschStoerDenseDriver* BSDriver = new BulirschStoerDenseDriver(hmin,equation);

    G4ChordFinder testChordFinder(BSDriver);
    //refChordFinder.SetDeltaChord(1*cm);
    //testChordFinder.SetDeltaChord(1*cm);

    //unused variables for G4ChordFinder
    const G4ThreeVector vec(0,0,0);
    G4double latestSafetyRadius = 0;

    G4double pathRest = path;
    G4double step;
    G4double yRef[N], yTest[N];
    std::ofstream outRef("outRef.txt");
    std::ofstream outBS("outBS.txt");

    G4double eps = precision;
    G4double pathRest2 = pathRest;

    while (pathRest > hmin){

       // Advance the method under test
        step = testChordFinder.AdvanceChordLimited(*testTrack,pathRest2,eps,vec,latestSafetyRadius);
        pathRest -= step;

        // Advance reference method
        G4double tempRest = step;
        while(tempRest > hmin)
        {
          G4double h = refChordFinder.AdvanceChordLimited(*refTrack,tempRest,eps,vec,latestSafetyRadius);
          tempRest -= h;
        }

        // Obtain results
        refTrack->DumpToArray(yRef);
        testTrack->DumpToArray(yTest);

        CrossCheck(yTest,yRef,Default);

        // Output positions 
        outRef << yRef[0]<< "  "<<yRef[1]<< "  "<<yRef[2] << G4endl;
        outBS << yTest[0]<< "  "<<yTest[1]<< "  "<<yTest[2] << G4endl;
    }

    /*------    
    // Part 2 -- ... ?
    G4CachedMagneticField* cachedField = static_cast<G4CachedMagneticField*>(field);

    while (pathRest > hmin){
        step = refChordFinder.AdvanceChordLimited(*refTrack,pathRest,eps,vec,latestSafetyRadius);
        pathRest -= step;
        refTrack->DumpToArray(y);
        outRef << y[0]<< "  "<<y[1]<< "  "<<y[2] << G4endl;
        // outRef << y[3]<< "  "<<y[4]<< "  "<<y[5] << G4endl;
    }
    G4cout<<"ref calls: "<<cachedField->GetCountCalls()<<G4endl;
    cachedField->ClearCounts();
    //refChordFinder.PrintStatistics();

    pathRest = path;
    while (pathRest > hmin){
        step = testChordFinder.AdvanceChordLimited(*testTrack,pathRest,eps,vec,latestSafetyRadius);
        pathRest -= step;
        testTrack->DumpToArray(y);
        outBS << y[0]<< "  "<<y[1]<< "  "<<y[2] << G4endl;
    }
    G4cout<<"BS calls: "<<cachedField->GetCountCalls()<<G4endl;
     --------*/
}



#endif
