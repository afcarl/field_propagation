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


// This code implementation is the intellectual property of
// the RD44 GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms.
//
// $Id: NTSTEventAction.hh 66356 2012-12-18 09:02:32Z gcosmo $
//
// 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

#ifndef NTSTEventAction_h
#define NTSTEventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class G4Timer;
class G4Event;
class NTSTEventActionMessenger;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo....

class NTSTEventAction : public G4UserEventAction
{
public:
  NTSTEventAction();
  ~NTSTEventAction();
  
public:
  void BeginOfEventAction(const G4Event* aEvent);
  void EndOfEventAction(const G4Event* aEvent);
  
  void SetDrawFlag(G4String val)  {drawFlag = val;};
  
private:
  G4Timer* EventTime;
  double MeanUserEventTime;
  double RmsUserEventTime;
  double MeanRealEventTime;
  double RmsRealEventTime;
  double NumberOfEvents;
  double MeanVertices;
  double RmsVertices;
  double MeanTracks;
  double RmsTracks;
  G4int    calorimeterCollID;                // Hits collection ID
  
  G4String drawFlag;                         // control the drawing of event
  NTSTEventActionMessenger*  eventMessenger;
};

#endif

    
