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
// $Id: G4MagIntegratorDriver.hh 104203 2017-05-17 17:35:02Z japost $
//
//
// class G4MagInt_Driver
//
// Class description:
//
// Provides a driver that talks to the Integrator Stepper, and insures that 
// the error is within acceptable bounds.

// History:
// - Created. J.Apostolakis.
// --------------------------------------------------------------------

#ifndef G4MAGINT_DRIVER_HH
#define G4MAGINT_DRIVER_HH

#include "G4Types.hh"
#include "G4FieldTrack.hh"
#include "G4VIntegrationDriver.hh"
#include "G4MagIntegratorStepper.hh"

class G4MagInt_Driver: public G4VIntegrationDriver {
public:

    G4MagInt_Driver(
        G4double hminimum,
        G4MagIntegratorStepper* pItsStepper,
        G4int numberOfComponents = 6,
        G4int statisticsVerbosity = 1);

    virtual ~G4MagInt_Driver() override;

    G4MagInt_Driver(const G4MagInt_Driver&) = delete;
    const G4MagInt_Driver& operator =(const G4MagInt_Driver&) = delete;

    // Above drivers for integrator (Runge-Kutta) with stepsize control.
    // Integrates ODE starting values y_current
    // from current s (s=s0) to s=s0+h with accuracy eps.
    // On output ystart is replaced by value at end of interval.
    // The concept is similar to the odeint routine from NRC p.721-722.
    virtual G4bool AccurateAdvance(
        G4FieldTrack&  y_current,
        G4double hstep,
        G4double eps,            // Requested y_err/hstep
        G4double hinitial=0) override;  // Suggested 1st interval


    // QuickAdvance just tries one Step - it does not ensure accuracy.
    virtual G4bool QuickAdvance(
        G4FieldTrack& y_val,      // INOUT
        const G4double dydx[],
        G4double hstep,       // IN
        G4double& dchord_step,
        G4double& dyerr) override;

    virtual void GetDerivatives(
        const G4FieldTrack &y_curr,  // const, INput
         G4double dydx[]) const override;  //  OUTput

    // Taking the last step's normalised error, calculate
    // a step size for the next step.
    // Do not limit the next step's size within a factor of the
    // current one.
    virtual G4double ComputeNewStepSize(
        G4double errMaxNorm,    // normalised error
        G4double hstepCurrent) override; // current step size

    inline virtual void SetVerboseLevel(G4int newLevel) override;
    inline virtual G4int GetVerboseLevel() const override;

    virtual G4EquationOfMotion* GetEquationOfMotion() override
    {
        return pIntStepper->GetEquationOfMotion();
    }

    virtual void SetEquationOfMotion(G4EquationOfMotion* equation) override
    {
        pIntStepper->SetEquationOfMotion(equation);
    }


     inline G4double GetHmin() const;
     inline G4double Hmin() const;     // Obsolete
     inline G4double GetSafety() const;
     inline G4double GetPshrnk() const;
     inline G4double GetPgrow() const;
     inline G4double GetErrcon() const;

        // Accessors.

     inline void RenewStepperAndAdjust(G4MagIntegratorStepper *pItsStepper);
        // Sets a new stepper pItsStepper for this driver. Then it calls
        // ReSetParameters to reset its parameters accordingly.

     inline void ReSetParameters(G4double new_safety= 0.9 );
        //  i) sets the exponents (pgrow & pshrnk), 
        //     using the current Stepper's order, 
        // ii) sets the safety
        // ii) calculates "errcon" according to the above values.

     inline void SetSafety(G4double valS);
     inline void SetPshrnk(G4double valPs);
     inline void SetPgrow (G4double valPg);
     inline void SetErrcon(G4double valEc);
        // When setting safety or pgrow, errcon will be set to a 
        // compatible value.

     inline G4double ComputeAndSetErrcon();

     inline const G4MagIntegratorStepper* GetStepper() const;
     inline G4MagIntegratorStepper* GetStepper();

    // This takes one Step that is as large as possible while
    // satisfying the accuracy criterion of:
    // yerr < eps * |y_end-y_start|
    void OneGoodStep(
        G4double ystart[], // Like old RKF45step()
        const G4double dydx[],
        G4double& x,
        G4double htry,
        G4double eps,      //  memb variables ?
        G4double& hdid,
        G4double& hnext) ;


    // Taking the last step's normalised error, calculate
    // a step size for the next step.
    // Limit the next step's size within a range around the current one.
    G4double ComputeNewStepSize_WithinLimits(
        G4double  errMaxNorm,    // normalised error
        G4double  hstepCurrent); // current step size


     inline G4int    GetMaxNoSteps() const;
     inline void     SetMaxNoSteps( G4int val); 
        //  Modify and Get the Maximum number of Steps that can be
        //   taken for the integration of a single segment -
        //   (ie a single call to AccurateAdvance).

public:  // without description

     inline void SetHmin(G4double newval);


     inline G4double GetSmallestFraction() const; 
     void     SetSmallestFraction( G4double val ); 

protected:  // without description
     void WarnSmallStepSize( G4double hnext, G4double hstep, 
                             G4double h,     G4double xDone,
                             G4int noSteps);
     void WarnTooManyStep( G4double x1start, G4double x2end, G4double xCurrent);
     void WarnEndPointTooFar (G4double  endPointDist, 
                              G4double  hStepSize , 
                              G4double  epsilonRelative,
                              G4int     debugFlag);
        //  Issue warnings for undesirable situations

     void PrintStatus(  const G4double*      StartArr,
                              G4double       xstart,
                        const G4double*      CurrentArr, 
                              G4double       xcurrent, 
                              G4double       requestStep, 
                              G4int          subStepNo );
     void PrintStatus(  const G4FieldTrack&  StartFT,
                        const G4FieldTrack&  CurrentFT, 
                              G4double       requestStep, 
                              G4int          subStepNo );
     void PrintStat_Aux( const G4FieldTrack& aFieldTrack,
                               G4double      requestStep, 
                               G4double      actualStep,
                               G4int         subStepNo,
                               G4double      subStepSize,
                               G4double      dotVelocities );       
       //  Verbose output for debugging

     void PrintStatisticsReport();
       //  Report on the number of steps, maximum errors etc.


private:
     G4double ShrinkStepSize(G4double h, G4double errmax_sq) const;
     G4double GrowStepSize(G4double h, G4double errmax_sq) const;
     G4double RelativeError(
         const G4double y[],
         const G4double yerr[],
         const G4double h,
         const G4double eps_rel_max,
         const G4int iter);
     //  INVARIANTS 

     G4double  fMinimumStep;
        // Minimum Step allowed in a Step (in absolute units)
     G4double  fSmallestFraction;      //   Expected range 1e-12 to 5e-15;  
        // Smallest fraction of (existing) curve length - in relative units
        //  below this fraction the current step will be the last 

     const G4int  fNoIntegrationVariables;  // Number of Variables in integration
     const G4int  fMinNoVars;               // Minimum number for FieldTrack
     const G4int  fNoVars;                  // Full number of variable

     G4int   fMaxNoSteps;
     static const G4int  fMaxStepBase;  

     G4double safety;
     G4double pshrnk;   //  exponent for shrinking
     G4double pgrow;    //  exponent for growth
     G4double errcon;
        // Parameters used to grow and shrink trial stepsize.

     static const G4double max_stepping_increase;
     static const G4double max_stepping_decrease;
        // Maximum stepsize increase/decrease factors.

     G4int    fStatisticsVerboseLevel;

     // ---------------------------------------------------------------
     // DEPENDENT Objects
     G4MagIntegratorStepper *pIntStepper;

     // ---------------------------------------------------------------
     //  STATE

     unsigned long fNoTotalSteps, fNoBadSteps, fNoSmallSteps, fNoInitialSmallSteps;
     unsigned long fNoCalls; 
     G4double fDyerr_max, fDyerr_mx2;
     G4double fDyerrPos_smTot, fDyerrPos_lgTot, fDyerrVel_lgTot; 
     G4double fSumH_sm, fSumH_lg; 
        // Step Statistics 

     G4int  fVerboseLevel;   // Verbosity level for printing (debug, ..)
        // Could be varied during tracking - to help identify issues
};

#include "G4MagIntegratorDriver.icc"

#endif /* G4MagInt_Driver_Def */
