//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: NTSTFieldSetup.hh,v 1.2 2007-10-26 09:51:28 gcosmo Exp $
// GEANT4 tag $Name: not supported by cvs2svn $
//
//
// A class for control of the Magnetic Field of the detector.

#ifndef NTSTFieldSetup_H
#define NTSTFieldSetup_H

#include "G4MagneticField.hh"
#include "G4UniformMagField.hh"
#include "G4RevisedChordFinder.hh"

class NTSTGradientField;
class NTSTTabulatedField3d;
class NTSTField;

class G4FieldManager;
class G4ChordFinder;
class G4Mag_UsualEqRhs;
class G4MagIntegratorStepper;
class NTSTFieldMessenger;


class NTSTFieldSetup
{
public:
  NTSTFieldSetup(G4MagneticField *commonField) ;  //  The value of the field
  NTSTFieldSetup() ;               //  A zero field

  ~ NTSTFieldSetup() ;  
      
  
  void SetStepperType( G4int i) { fStepperType = i ; }
  void SetFieldName( G4int i)  { fFieldName = i ; }
  void SetStepper();
  void SetField();

  void SetMinStep(G4double s) { fMinStep = s ; }
  void SetGrad(G4double s) { fMinStep = s ; }
  void SetMinEpsilon(G4double s) { fMinEpsilon = s ; }
  void SetMaxEpsilon(G4double s) { fMaxEpsilon = s ; }

  void InitialiseAll();    //  Set parameters and call method below
  void CreateStepperAndChordFinder();

  void SetFieldValue(G4double      fieldValue) ;
  G4ThreeVector GetConstantFieldValue();

  void GetChordFinderStats();  
  void GetFieldCallStats();
 
   

protected:

      // Find the global Field Manager

  G4FieldManager*         GetGlobalFieldManager() ;   // static 

  G4FieldManager*         fFieldManager ;
  G4RevisedChordFinder*   fChordFinder ;
  G4Mag_UsualEqRhs*       fEquation ; 
  G4MagneticField*        fMagneticField ; 

   NTSTGradientField *pAField1;
   NTSTTabulatedField3d *pAField2;
   NTSTField * ffield;

  G4MagIntegratorStepper* fStepper ;
  G4MagIntegratorStepper* pStepper ;
  G4int                   fStepperType ;
  G4int                   fFieldName ;
  G4double                fMinStep ;
  G4double                fGradofField;
  G4double                fMaxEpsilon;
  G4double                fMinEpsilon;

  NTSTFieldMessenger*      fFieldMessenger;

};

#endif
