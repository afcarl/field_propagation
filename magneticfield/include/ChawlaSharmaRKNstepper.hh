// Nystrom stepper implemenation by Jason Suagee
//  Supervision / code review: John Apostolakis
//
// Sponsored by Google in Google Summer of Code 2015.
//
// First version: 27 May 2015
//
// This code is made available subject to the Geant4 license, a copy of
// which is available at
//   http://geant4.org/license

#ifndef CHAWLA_SHARMA_RKN_STEPPER_HH
#define CHAWLA_SHARMA_RKN_STEPPER_HH

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4MagIntegratorStepper.hh"
#include "G4Mag_EqRhs.hh"
#include "G4ThreeVector.hh"
#include "G4LineSection.hh"

class ChawlaSharmaRKNstepper : public G4MagIntegratorStepper
{
	public:
	ChawlaSharmaRKNstepper(G4Mag_EqRhs *EquationMotion,
					G4int numberOfVariables = 6, G4int numStateVariables = 12);
	
	~ChawlaSharmaRKNstepper();
	
	virtual void Stepper( const G4double y[],
                  const G4double dydx[],
                        G4double h,
                        G4double yout[],
                        G4double yerr[]  );
	
	virtual void DumbStepper( const G4double y[],
										//const G4double dydx[],
										G4double h,
										G4double yOut[] );
	
	//G4int IntegratorOrder() const {return 3;}; // Richardson extrapolation promotes to 4th order
   G4int IntegratorOrder() const {return 4;}; // Richardson extrapolation promotes to 4th order
   
   G4double DistChord() const;
   
	private:
   
   ////////////////////////////////////////////////////////////////
   // Private data
   ////////////////////////////////////////////////////////////////

   G4Mag_EqRhs*           m_fEq;          
   
   private:
   
   G4int nposvars, nmomvars;

   // STATE
   G4ThreeVector fInitialPoint, fMidPoint, fFinalPoint;
   // Data stored in order to find the chord
   
   // Dependent Objects, owned --- part of the STATE 
   G4double *yInitial, *yMiddle, *dydxMid, *yOneStep;
   G4double *pos, *mom, *K1, *K2, *K3, *temp_eval_pt;
   // The following arrays are used only for temporary storage
   // they are allocated at the class level only for efficiency -
   // so that calls to new and delete are not made in Stepper().
};


inline
ChawlaSharmaRKNstepper::ChawlaSharmaRKNstepper(G4Mag_EqRhs *EquationRhs,
                                     G4int numberOfVariables, 
				     G4int numStateVariables)
     : G4MagIntegratorStepper(EquationRhs,numberOfVariables,numStateVariables)
  {
      G4int nvar = std::max(this->GetNumberOfVariables(), 8);
      nposvars = nvar / 2;
      nmomvars = nvar / 2;

      yMiddle=     new G4double[nvar]; 
      dydxMid=     new G4double[nvar];
      yInitial=    new G4double[nvar];
      yOneStep= new G4double[nvar];

      K1 = new G4double[nvar]; K2 = new G4double[nvar]; K3 = new G4double[nvar];
      pos = new G4double[nposvars];
      mom = new G4double[nmomvars];
      temp_eval_pt = new G4double[nvar];

      m_fEq = EquationRhs;
  }


#endif  //CHAWLA_SHARMA_RKN_STEPPER_HH







