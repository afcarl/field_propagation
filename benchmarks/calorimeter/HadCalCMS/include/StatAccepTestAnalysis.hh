#ifndef StatAccepTestAnalysis_h 
#define StatAccepTestAnalysis_h 1

#include "globals.hh"
#include <vector>
#include <map>
#include <set>
#include "G4ThreeVector.hh"

class G4Step;
class G4Track;
class G4ParticleDefinition;


class StatAccepTestAnalysis {

public:

  static StatAccepTestAnalysis* getInstance();

  ~StatAccepTestAnalysis();
  
  static inline bool getIsHistogramOn();
  static inline void setIsHistogramOn( const bool choice );
  // Get/Set method for the "isHistogramOn" boolean variable.
  // The Set method allows to switch on/off all the histograms,
  // with the only exception of the ntuple which is always saved.

public:
  
  void init( const G4int numberOfReplicasIn, 
	     const G4int numberOfReadoutLayersIn,
	     const G4int numberOfRadiusBinsIn,
	     const G4double radiusBinIn );
  void init();
  void finish();
  void close();
  // The first method is called by StatAccepTestDetectorConstruction
  // only when the geometry is constructed the first time or changed.  
  // The second and third methods are called by StatAccepTestRunAction, 
  // at the beginning and at the end of a Run, respectively.
  // The fourth method, close(), it is called at the end of
  // job, i.e. in the destructor of StatAccepTestRunAction.

  void fillNtuple( float incidentParticleId, 
		   float incidenteParticleEnergy, 
                   float totalEnergyDepositedInActiveLayers,
                   float totalEnergyDepositedInCalorimeter );
  // This method is supposed to be called at most once for each event.  

  void fillShowerProfile( G4int replica, const G4double radius, const G4double edep,
			  const G4int particlePDG, const G4double particleEkin );
  // This method is called by StatAccepTestSensitiveCalorimeter at each step
  // in the active layer of the calorimeter.

  void infoStep( const G4Step* aStep );
  // This method is called by StatAccepTestSteppingAction at each step.

  void infoTrack( const G4Track* aTrack );
  // This method is called by StatAccepTestTrackingAction when each track
  // is created.

  void endOfEvent( const G4double timeEventInSec );
  // Inform that the event if finished: this is useful to calculate
  // properly the statistical error of the quantities defined per
  // event.

  static inline double getInfTimeWindow();
  static inline double getSupTimeWindow();
  // Returns the limits of the considered time window.

private:
  
  StatAccepTestAnalysis();

  static bool isHistogramOn; // To have only the ntuple in the HBOOK file.

  static bool is2DHistogramStepLvsEOn; // To switch on the 2D histograms of
                                       // step length vs. step energy.
 
  static bool isHistogramSpectrumUnweightedOn;  // To switch on/off the
  static bool isHistogramSpectrumWeightedOn;    // unweighted/weighted histograms.

  static bool isCountingProcessesOn; // To switch on/off the counting of processes.

  static bool isMapParticleNamesOn;  // To switch on/off the map of particle names.

  static G4double infParticleEkin_electron; // Energy thresholds for electrons,
  static G4double supParticleEkin_electron; // only for electron-information.
  static G4double infParticleEkin_muon;     // Energy thresholds for muons,
  static G4double supParticleEkin_muon;     // only for muon-information.
  static G4double infParticleEkin_pion;     // Energy thresholds for pions,
  static G4double supParticleEkin_pion;     // only for pion-information.
  static G4double infParticleEkin_kaon;     // Energy thresholds for kaons,
  static G4double supParticleEkin_kaon;     // only for kaon-information.
  static G4double infParticleEkin_proton;   // Energy thresholds for protons,
  static G4double supParticleEkin_proton;   // only for proton-information.
  static G4double infParticleEkin_nuclei;   // Energy thresholds for nuclei,
  static G4double supParticleEkin_nuclei;   // only for nuclei-information.

  static G4double infTimeWindow;  // To study the energy deposition as a
  static G4double supTimeWindow;  // function of time we define a time window.

  void classifyParticle( const bool isTrack, const G4ParticleDefinition* particleDef );
  
  static StatAccepTestAnalysis* instance;
  
  G4int numberOfEvents;

  G4int numberOfReplicas;
  G4int numberOfReadoutLayers;
  G4int numberOfActiveLayersPerReadoutLayer;
  G4int numberOfRadiusBins;  // Number of bins in the transverse profile
  G4double radiusBin;        // Size of the first bin in the transverse profile.
                             // The other bins have a bin size that is 
                             // as follows: second bin has a size that is twice
                             // the first bin size; the third one has a size
                             // that is three times the first bin size, and so on.

  std::vector< G4double > longitudinalProfile;
  std::vector< G4double > transverseProfile; 

  // To print, at the end of the Run, the average energy deposits, <E>,
  // and its error, in the whole calorimeter, in each readout layer, 
  // and in each radius bin, we need to collect the sum of the energy 
  // deposits and the sum of the square of energy deposits.
  // We also consider the maximum total energy deposit in an event,
  // to check for energy non conservation.
  G4int primaryParticleId;
  G4double beamEnergy;
  G4double sumEdepAct, sumEdepAct2;
  G4double sumEdepTot, sumEdepTot2, maxEdepTot;
  G4int countEnergyNonConservation;
  std::vector< G4double > sumL;
  std::vector< G4double > sumL2;
  std::vector< G4double > sumR;
  std::vector< G4double > sumR2;

  // Keep also separate information on the visible energy, total energy,
  // and the longitudinal and transverse shower profile contributions 
  // of the following particles:
  // e-  and  e+  together
  G4double sumEdepAct_electron, sumEdepAct_electron2;
  G4double sumEdepTot_electron, sumEdepTot_electron2;
  std::vector< G4double > sumL_electron;
  std::vector< G4double > sumL_electron2;
  std::vector< G4double > sumR_electron;
  std::vector< G4double > sumR_electron2;
  std::vector< G4double > longitudinalProfile_electron;
  std::vector< G4double > transverseProfile_electron; 
  // mu-  and  mu+  together
  G4double sumEdepAct_muon, sumEdepAct_muon2;
  G4double sumEdepTot_muon, sumEdepTot_muon2;
  std::vector< G4double > sumL_muon;
  std::vector< G4double > sumL_muon2;
  std::vector< G4double > sumR_muon;
  std::vector< G4double > sumR_muon2;
  std::vector< G4double > longitudinalProfile_muon;
  std::vector< G4double > transverseProfile_muon; 
  // pi-  and  pi+  together
  G4double sumEdepAct_pion, sumEdepAct_pion2;
  G4double sumEdepTot_pion, sumEdepTot_pion2;
  std::vector< G4double > sumL_pion;
  std::vector< G4double > sumL_pion2;
  std::vector< G4double > sumR_pion;
  std::vector< G4double > sumR_pion2;
  std::vector< G4double > longitudinalProfile_pion;
  std::vector< G4double > transverseProfile_pion; 
  // k-  and  k+  together
  G4double sumEdepAct_kaon, sumEdepAct_kaon2;
  G4double sumEdepTot_kaon, sumEdepTot_kaon2;
  std::vector< G4double > sumL_kaon;
  std::vector< G4double > sumL_kaon2;
  std::vector< G4double > sumR_kaon;
  std::vector< G4double > sumR_kaon2;
  std::vector< G4double > longitudinalProfile_kaon;
  std::vector< G4double > transverseProfile_kaon; 
  // proton  and  anti-proton  together:
  G4double sumEdepAct_proton, sumEdepAct_proton2;
  G4double sumEdepTot_proton, sumEdepTot_proton2;
  std::vector< G4double > sumL_proton;
  std::vector< G4double > sumL_proton2;
  std::vector< G4double > sumR_proton;
  std::vector< G4double > sumR_proton2;
  std::vector< G4double > longitudinalProfile_proton;
  std::vector< G4double > transverseProfile_proton; 
  // nuclei: all particles with PDG code = 0 and neutrons together
  // (since G4 8.1, neutrons can deposit energy, which corresponds
  //  to elastic recoiled nuclei below a certain kinetic energy);
  G4double sumEdepAct_nuclei, sumEdepAct_nuclei2;
  G4double sumEdepTot_nuclei, sumEdepTot_nuclei2;
  std::vector< G4double > sumL_nuclei;
  std::vector< G4double > sumL_nuclei2;
  std::vector< G4double > sumR_nuclei;
  std::vector< G4double > sumR_nuclei2;
  std::vector< G4double > longitudinalProfile_nuclei;
  std::vector< G4double > transverseProfile_nuclei; 

  // All but  e-  and  e+
  G4double sumEdepAct_no_electron, sumEdepAct_no_electron2;
  // All but  mu-  and  mu+ 
  G4double sumEdepAct_no_muon, sumEdepAct_no_muon2;
  // All but  pi-  and  pi+
  G4double sumEdepAct_no_pion, sumEdepAct_no_pion2;
  // All but  k-  and  k+
  G4double sumEdepAct_no_kaon, sumEdepAct_no_kaon2;
  // All but  proton  and  anti-proton
  G4double sumEdepAct_no_proton, sumEdepAct_no_proton2;
  // All but  nuclei and neutrons
  G4double sumEdepAct_no_nuclei, sumEdepAct_no_nuclei2;

  // Due to the non-gaussian visible energy distribution, the width 
  // of it cannot be estimated as the sigma of the gaussian fit; on
  // the other hand, the rms is not appropriate because it weights 
  // too much the tails. A better estimate would be to consider the
  // "normalized deviation". To calculate it, we need to keep all
  // the values of EdepAct in a vector.
  std::vector< G4double > vecEvis;

  std::vector< G4double > vecEvis_electron;    // only e- and e+ within Ekin range
  std::vector< G4double > vecEvis_muon;        //  "  mu-  " mu+   "      "     "
  std::vector< G4double > vecEvis_pion;        //  "  pi-  " pi+   "      "     "
  std::vector< G4double > vecEvis_kaon;        //  "   K-  "  K+   "      "     "
  std::vector< G4double > vecEvis_proton;      //  "   p   " pbar  "      "     "
  std::vector< G4double > vecEvis_nuclei;      //  "  nuclei & neutrons   "     " 

  std::vector< G4double > vecEvis_no_electron; // all but e- and e+ within Ekin range
  std::vector< G4double > vecEvis_no_muon;     //  "   " mu-  " mu+   "     "    "
  std::vector< G4double > vecEvis_no_pion;     //  "   " pi-  " pi+   "     "    "
  std::vector< G4double > vecEvis_no_kaon;     //  "   "  K-  "  K+   "     "    "
  std::vector< G4double > vecEvis_no_proton;   //  "   "  p   " pbar  "     "    "
  std::vector< G4double > vecEvis_no_nuclei;   //  "  nuclei & neutrons     "    "

  // Keep the count of the total number of steps and tracks,
  // for all events (so at the end it is necessary to divide
  // by the number of events). 
  // To evaluate the statistical errors, we keep also the sum
  // of the square of the total number of steps and tracks 
  // per event.
  // We use doubles instead of integers to avoid overflow problems.
  G4double numStep;
  G4double numStepPositive, numStepNeutral, numStepNegative;
  G4double numStepNucleus, numStepPDGCodeUnrecognized;
  G4double numStepEM;             // e- , e+ , gamma
  G4double numStepEWK;            // mu- , mu+ , tau+, tau-, neutrinos
  G4double numStepHAD;            // mesons + baryons
  G4double numStepMeson, numStepBaryon;     
  G4double numStepMesonLight, numStepBaryonLight;            // u/d-hadrons    
  G4double numStepMesonStrange, numStepBaryonStrange;        // s-hadrons
  G4double numStepMesonHeavy, numStepBaryonHeavy;            // c-hadrons, and b-hadrons
  G4double numStepElectron, numStepGamma, numStepPositron;
  G4double numStepMuMinus, numStepMuPlus;
  G4double numStepTauMinus, numStepTauPlus;
  G4double numStepNeutrino;
  G4double numStepPiPlus, numStepPi0, numStepPiMinus;
  G4double numStepKPlus;
  G4double numStepKNeutral;       // K0/K0bar or K0_S/KO_L
  G4double numStepKMinus;
  G4double numStepProton, numStepAntiProton;
  G4double numStepNeutron, numStepAntiNeutron;

  G4double numStep2;
  G4double numStepPositive2, numStepNeutral2, numStepNegative2;
  G4double numStepNucleus2, numStepPDGCodeUnrecognized2;
  G4double numStepEM2;             
  G4double numStepEWK2;            
  G4double numStepHAD2;            
  G4double numStepMeson2, numStepBaryon2;     
  G4double numStepMesonLight2, numStepBaryonLight2;
  G4double numStepMesonStrange2, numStepBaryonStrange2;
  G4double numStepMesonHeavy2, numStepBaryonHeavy2;
  G4double numStepElectron2, numStepGamma2, numStepPositron2;
  G4double numStepMuMinus2, numStepMuPlus2;
  G4double numStepTauMinus2, numStepTauPlus2;
  G4double numStepNeutrino2;
  G4double numStepPiPlus2, numStepPi02, numStepPiMinus2;
  G4double numStepKPlus2;
  G4double numStepKNeutral2;
  G4double numStepKMinus2;
  G4double numStepProton2, numStepAntiProton2;
  G4double numStepNeutron2, numStepAntiNeutron2;

  G4double numTrack;
  G4double numTrackPositive, numTrackNeutral, numTrackNegative;
  G4double numTrackNucleus, numTrackPDGCodeUnrecognized;
  G4double numTrackEM;             // e- , e+ , gamma
  G4double numTrackEWK;            // mu- , mu+ , tau+, tau-, neutrinos
  G4double numTrackHAD;            // mesons + baryons
  G4double numTrackMeson, numTrackBaryon;     
  G4double numTrackMesonLight, numTrackBaryonLight;          // u/d-hadrons
  G4double numTrackMesonStrange, numTrackBaryonStrange;      // s-hadrons
  G4double numTrackMesonHeavy, numTrackBaryonHeavy;          // c-hadrons, and b-hadrons
  G4double numTrackElectron, numTrackGamma, numTrackPositron;
  G4double numTrackMuMinus, numTrackMuPlus;
  G4double numTrackTauMinus, numTrackTauPlus;
  G4double numTrackNeutrino;
  G4double numTrackPiPlus, numTrackPi0, numTrackPiMinus;
  G4double numTrackKPlus;
  G4double numTrackKNeutral;       // K0/K0bar or K0_S/KO_L
  G4double numTrackKMinus;
  G4double numTrackProton, numTrackAntiProton;
  G4double numTrackNeutron, numTrackAntiNeutron;

  G4double numTrack2;
  G4double numTrackPositive2, numTrackNeutral2, numTrackNegative2;
  G4double numTrackNucleus2, numTrackPDGCodeUnrecognized2;
  G4double numTrackEM2;
  G4double numTrackEWK2;
  G4double numTrackHAD2;
  G4double numTrackMeson2, numTrackBaryon2;     
  G4double numTrackMesonLight2, numTrackBaryonLight2;
  G4double numTrackMesonStrange2, numTrackBaryonStrange2;
  G4double numTrackMesonHeavy2, numTrackBaryonHeavy2;
  G4double numTrackElectron2, numTrackGamma2, numTrackPositron2;
  G4double numTrackMuMinus2, numTrackMuPlus2;
  G4double numTrackTauMinus2, numTrackTauPlus2;
  G4double numTrackNeutrino2;
  G4double numTrackPiPlus2, numTrackPi02, numTrackPiMinus2;
  G4double numTrackKPlus2;
  G4double numTrackKNeutral2;
  G4double numTrackKMinus2;
  G4double numTrackProton2, numTrackAntiProton2;
  G4double numTrackNeutron2, numTrackAntiNeutron2;

  // Count the track length of some particles.
  // These variables keep the sum over all tracks of all events
  // (so at the end we have to divide by the total number of
  //  tracks in all events).
  // To compute the statistical uncertainty, we keep also the sum
  // of the squares.
  // To avoid bias in the track length due to the big world volume
  // (which can affect significantly the track length of neutrons)
  // we consider only those tracks that are fully contained inside
  // the calorimeter, i.e. created and terminated inside it.
  G4double electronTrackLength;      // e- or e+
  G4double muonTrackLength;          // mu- or mu+
  G4double pionChargedTrackLength;   // pi- or pi+
  G4double protonTrackLength;        // proton (not anti_proton)
  G4double gammaTrackLength;         // gamma
  G4double pion0TrackLength;         // pi0       
  G4double neutronTrackLength;       // neutron (not anti_neutron)

  G4double electronTrackLength2;
  G4double muonTrackLength2;     
  G4double pionChargedTrackLength2;
  G4double protonTrackLength2;     
  G4double gammaTrackLength2;      
  G4double pion0TrackLength2;      
  G4double neutronTrackLength2;    

  // Monitor the kinetic energy of particles exiting the calorimeter.
  // These variables keep the sum over all events (so at the end
  // we have to divide by the number of events).
  // To evaluate the statistical errors, we keep also the sum
  // of the squares, event by event.
  G4double kinEnergyExiting;          // total
  G4double kinEnergyExitingGammas;    // due only to gammas
  G4double kinEnergyExitingNeutrons;  // due only to neutrons
  G4double kinEnergyExitingNeutrinos; // due only to neutrinos
  G4double kinEnergyExitingMuons;     // due only to muons
  G4double kinEnergyExitingElectrons; // due only to e- and e+
  G4double kinEnergyExitingOthers;    // due to other particles
  G4double numExiting;                // number of exiting particles
  G4double numExitingGammas;          // number of exiting gammas
  G4double numExitingNeutrons;        // number of exiting neutrons
  G4double numExitingNeutrinos;       // number of exiting neutrinos
  G4double numExitingMuons;           // number of exiting muons
  G4double numExitingElectrons;       // number of exiting e- and e+
  G4double numExitingOthers;          // number of other particles

  G4double kinEnergyExiting2;          
  G4double kinEnergyExitingGammas2;    
  G4double kinEnergyExitingNeutrons2;  
  G4double kinEnergyExitingNeutrinos2; 
  G4double kinEnergyExitingMuons2;     
  G4double kinEnergyExitingElectrons2; 
  G4double kinEnergyExitingOthers2;    
  G4double numExiting2;                
  G4double numExitingGammas2;          
  G4double numExitingNeutrons2;        
  G4double numExitingNeutrinos2;       
  G4double numExitingMuons2;           
  G4double numExitingElectrons2;       
  G4double numExitingOthers2;          

  // Count number of inelastic and stoppingAtRest processes.
  G4int numInelasticProcesses;
  G4int numProtonInelasticProcesses;
  G4int numAntiProtonInelasticProcesses;
  G4int numNeutronInelasticProcesses;
  G4int numAntiNeutronInelasticProcesses;
  G4int numPionPlusInelasticProcesses;
  G4int numPionMinusInelasticProcesses;
  G4int numKaonPlusInelasticProcesses;
  G4int numKaonMinusInelasticProcesses;
  G4int numKaonNeutralInelasticProcesses;
  G4int numLambdaInelasticProcesses;
  G4int numAntiLambdaInelasticProcesses;
  G4int numSigmaMinusInelasticProcesses;
  G4int numMuonMinusInelasticProcesses;
  G4int numOtherInelasticProcesses;
  G4int numStoppingAtRestProcesses;
  G4int numAntiProtonStoppingAtRestProcesses;
  G4int numNeutronStoppingAtRestProcesses;
  G4int numAntiNeutronStoppingAtRestProcesses;
  G4int numPionMinusStoppingAtRestProcesses;
  G4int numKaonMinusStoppingAtRestProcesses;
  G4int numKaonNeutralStoppingAtRestProcesses;
  G4int numLambdaStoppingAtRestProcesses;
  G4int numAntiLambdaStoppingAtRestProcesses;
  G4int numSigmaMinusStoppingAtRestProcesses;
  G4int numMuonMinusStoppingAtRestProcesses;
  G4int numOtherStoppingAtRestProcesses;

  // List of particles names and total count of the number of
  // tracks of such particle type (excluding e-, e+, gamma).
  std::map< std::string, int > mapParticleNames;

  // To improve performance, save the PDG code of particles on 
  // constant integers. It is important that they are declared
  // as constant in order to use them in switch statements.
  const G4int electronId, positronId, gammaId;
  const G4int muonMinusId, muonPlusId, tauMinusId, tauPlusId;
  const G4int eNeutrinoId, antiENeutrinoId, muNeutrinoId, antiMuNeutrinoId,
    tauNeutrinoId, antiTauNeutrinoId;
  const G4int pionPlusId, pionMinusId, pionZeroId;
  const G4int kaonMinusId, kaonPlusId, kaonZeroId, antiKaonZeroId,
    kaonShortId, kaonLongId;
  const G4int protonId, antiProtonId, neutronId, antiNeutronId;

  // Monitor the CPU time event by event.
  std::multiset<G4double> eventTimeSet;

};


inline bool StatAccepTestAnalysis::getIsHistogramOn() {
  return isHistogramOn;
}

inline void StatAccepTestAnalysis::setIsHistogramOn( const bool choice ) {
  isHistogramOn = choice;
  G4cout << "  --->  StatAccepTestAnalysis::isHistogramOn = " << isHistogramOn 
	 << "  <---" << G4endl << G4endl; 
}


inline double StatAccepTestAnalysis::getInfTimeWindow() {
  return infTimeWindow;
}

inline double StatAccepTestAnalysis::getSupTimeWindow() {
  return supTimeWindow;
}

#endif
