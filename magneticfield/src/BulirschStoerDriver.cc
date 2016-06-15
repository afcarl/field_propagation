#include "BulirschStoerDriver.hh"
#include "G4LineSection.hh"
#include "G4SystemOfUnits.hh"
#include "ModifiedMidpoint.hh"

#include <boost/numeric/odeint.hpp>

#define ncomp G4FieldTrack::ncompSVEC

using namespace boost::numeric::odeint;

BulirschStoerDriver::BulirschStoerDriver(G4double hminimum,
                                         G4EquationOfMotion* equation,
                                         G4int integratedComponents,
                                         G4int  verb):
    BaseDriver(hminimum,equation,integratedComponents,verb),
    dummyStepper(new BSStepper(equation)),
    modifiedMidpoint(equation,integratedComponents),
    BulirschStoer(0,0,1,0,5)

{
}

BulirschStoerDriver::~BulirschStoerDriver(){
    delete dummyStepper;
}


G4bool  BulirschStoerDriver::AccurateAdvance(G4FieldTrack&  y_current,
                         G4double hstep,
                         G4double eps,
                         G4double hinitial){


    G4int fNoTotalSteps = 0;
    G4int fMaxNoSteps = 1000;
    G4double fNoBadSteps = 0;
    G4double fSmallestFraction = 1.0e-12;

    // Driver with adaptive stepsize control. Integrate starting
    // values at y_current over hstep x2 with accuracy eps.
    // On output ystart is replaced by values at the end of the integration
    // interval. RightHandSide is the right-hand side of ODE system.
    // The source is similar to odeint routine from NRC p.721-722 .


    //  Ensure that hstep > 0
    if(hstep == 0)
    {
      std::ostringstream message;
      message << "Proposed step is zero; hstep = " << hstep << " !";
      G4Exception("G4MagInt_Driver::AccurateAdvance()",
                  "GeomField1001", JustWarning, message);

      return true;
    }
    if( hstep < 0)
    {
        std::ostringstream message;
        message << "Invalid run condition." << G4endl
                << "Proposed step is negative; hstep = " << hstep << "." << G4endl
                << "Requested step cannot be negative! Aborting event.";
        G4Exception("G4MagInt_Driver::AccurateAdvance()",
                    "GeomField0003", EventMustBeAborted, message);

        return false;
    }



    //init first step size
    G4double h;
    if ( (hinitial > 0) && (hinitial < hstep)
      && (hinitial > perMillion * hstep) )
    {
       h = hinitial;
    }
    else  //  Initial Step size "h" defaults to the full interval
    {
       h = hstep;
    }

    //integration variables
    G4double y[ncomp], dydx[ncomp], yEnd[ncomp];
    y_current.DumpToArray(y);
    //copy non-integration variables to out array
    memcpy(yEnd+fnvar,y+fnvar,sizeof(G4double)*(ncomp-fnvar));

    G4double startCurveLength = y_current.GetCurveLength();
    G4double curveLength = startCurveLength;
    G4double endCurveLength = startCurveLength + hstep;

    //loop variables
    G4int nstp = 1, no_warnings = 0;
    G4double hnext, hdid;


    G4bool succeeded = true, lastStepSucceeded;

    G4int  noFullIntegr = 0, noSmallIntegr = 0 ;
    static G4ThreadLocal G4int  noGoodSteps = 0 ;  // Bad = chord > curve-len

    G4bool lastStep = false;

    do
    {
        G4ThreeVector StartPos(y[0], y[1], y[2]);

        fequation->RightHandSide(y, dydx);

        fNoTotalSteps++;

        // Perform the Integration
        if(h == 0){
            G4Exception("G4MagInt_Driver::AccurateAdvance()",
                        "GeomField0003", FatalException,
                        "Integration Step became Zero!");
        }
        else if(h > fMinimumStep){
            //step size if Ok
            OneGoodStep(y,dydx,curveLength,h,eps,hdid,hnext);
            lastStepSucceeded = (hdid == h);
        }
        else{
            // for small steps call QuickAdvance for speed
            G4FieldTrack yFldTrk(y_current);
            G4double dchord_step, dyerr, dyerr_len;   // What to do with these ?
            yFldTrk.LoadFromArray(y, ncomp);
            yFldTrk.SetCurveLength(curveLength);

            QuickAdvance(yFldTrk, dydx, h, dchord_step, dyerr_len);

            yFldTrk.DumpToArray(y);


            dyerr = dyerr_len / h;
            hdid = h;
            curveLength += hdid;

            // Compute suggested new step
            hnext = ComputeNewStepSize(dyerr/eps, h);

            //hnext= ComputeNewStepSize_WithinLimits( dyerr/eps, h);
            lastStepSucceeded = (dyerr <= eps);
        }


        lastStepSucceeded ? ++noFullIntegr : ++noSmallIntegr;


        G4ThreeVector EndPos(y[0], y[1], y[2]);

        // Check the endpoint
        G4double endPointDist = (EndPos - StartPos).mag();
        if (endPointDist >= hdid*(1. + perMillion))
        {
            fNoBadSteps++;

            // Issue a warning only for gross differences -
            // we understand how small difference occur.
            if (endPointDist >= hdid*(1.+perThousand))
            {
                ++no_warnings;
            }
        }
        else
        {
            noGoodSteps ++;
        }

        //  Avoid numerous small last steps
        if((h < eps * hstep) || (h < fSmallestFraction * startCurveLength))
        {
            // No more integration -- the next step will not happen
            lastStep = true;
        }
        else
        {
            // Check the proposed next stepsize
            if(std::fabs(hnext) < fMinimumStep)
            {
              // Make sure that the next step is at least Hmin.
              h = fMinimumStep;
            }
            else
            {
              h = hnext;
            }

            //  Ensure that the next step does not overshoot
            if (curveLength + h > endCurveLength)
            {
              h = endCurveLength - curveLength;
            }

            if (h == 0)
            {
              // Cannot progress - accept this as last step - by default
              lastStep = true;
            }
        }
    } while (((nstp++) <= fMaxNoSteps) && (curveLength < endCurveLength) && (!lastStep));
       // Have we reached the end ?
       // --> a better test might be x-x2 > an_epsilon

    succeeded = (curveLength >= endCurveLength);  // If it was a "forced" last step

    memcpy(yEnd,y,sizeof(G4double)*fnvar);

    // Put back the values.
    y_current.LoadFromArray(yEnd, ncomp);
    y_current.SetCurveLength(curveLength);

    if(nstp > fMaxNoSteps)
    {
      no_warnings++;
      succeeded = false;
    }

    return succeeded;

/*
    if (hstep > fMinimumStep){
        //G4cout<<"BulirschStoerDriver::AccurateAdvance \n";
        G4double dydx[ncomp];
        G4double y[ncomp];
        track.DumpToArray(y);
        G4double hdid = 0;
        G4double hnext = beginStep != 0 ? beginStep : hstep;
        //G4double hnext = beginStep;
        G4double hrest = hstep;
        G4double curveLength = track.GetCurveLength();
        do{
            fequation->RightHandSide(y,dydx);
            OneGoodStep(y,dydx,curveLength,hnext,eps,hdid,hnext);
            hrest -= hdid;
            hnext = std::min(hrest, hnext);
            //G4cout<<"hdid "<<hdid<<" hrest "<<hrest<<" hnext "<<hnext<<G4endl;
        }while(hrest > eps*hstep);
        //G4cout<<"driver: did step "<<stepInitial<<G4endl;
        track.SetCurveLength(curveLength);
        track.LoadFromArray(y,ncomp);
        //G4cout<<y[0]<<"  "<<y[1]<<"  "<<y[2]<<G4endl;
    }

    return  true;*/
}

G4bool  BulirschStoerDriver::QuickAdvance(G4FieldTrack& track,
                                          const G4double dydx[],
                                          G4double hstep,
                                          G4double& missDist,
                                          G4double& dyerr){

    state_type yIn, yOutMid, yOut, yOut2;
    state_type dydxIn, dydxMid;
    G4double y[ncomp];
    track.DumpToArray(y);
    memcpy(yIn.data(),y,sizeof(G4double)*ncomp);
    memcpy(dydxIn.data(),dydx,sizeof(G4double)*ncomp);
    const G4double curveLength = track.GetCurveLength();
/*
    modified_midpoint<state_type> mp;
    mp.do_step(system,yIn,dydxIn,curveLength,yOut2,hstep);

    mp.do_step(system,yIn,dydxIn,curveLength,yOutMid,hstep/2.);
    system(yOutMid,dydxMid,curveLength+hstep/2.);
    mp.do_step(system,yOutMid,dydxMid,curveLength+hstep/2.,yOut,hstep/2.);
*/

    modifiedMidpoint.do_step(yIn,dydxIn,curveLength,yOut2,hstep);
    modifiedMidpoint.do_step(yIn,dydxIn,curveLength,yOutMid,0.5*hstep);
    fequation->RightHandSide(yOutMid.data(),dydxMid.data());
    modifiedMidpoint.do_step(yOutMid,dydxMid,curveLength+0.5*hstep,yOut,0.5*hstep);

    missDist = G4LineSection::Distline(G4ThreeVector(yOutMid[0],yOutMid[1],yOutMid[2]),
                                       G4ThreeVector(yIn[0],yIn[1],yIn[2]),
                                       G4ThreeVector(yOut[0],yOut[1],yOut[2]));




    memcpy(y,yOut.data(),sizeof(G4double)*fnvar);
    track.LoadFromArray(y,ncomp);
    track.SetCurveLength(curveLength + hstep);

    dyerr = calcError(yOut, yOut2, hstep);

    return true;
}

void BulirschStoerDriver::GetDerivatives(const G4FieldTrack& track, G4double dydx[] ){
    G4double y[ncomp];
    track.DumpToArray(y);
    fequation->RightHandSide(y,dydx);
}


void  BulirschStoerDriver::OneGoodStep(G4double  y[],
                                       const G4double  dydx[],
                                       G4double& curveLength,
                                       G4double htry,
                                       G4double eps,
                                       G4double& hdid,
                                       G4double& hnext){
    //G4cout<<"OneGoodStep "<<htry<<G4endl;
    bulirsch_stoer<state_type> stepper(0,eps,1,0,/*htry*/0);
    //stepper.set_max_dt(htry);
    //stepper.set_eps_rel(eps);

    //BulirschStoer.set_max_dt(htry);
    //BulirschStoer.set_eps_rel(eps);

    hnext = htry;
    hdid = 0;

    state_type yInOut, dydxIn;
    memset(yInOut.data(),0,sizeof(G4double)*ncomp);
    memset(dydxIn.data(),0,sizeof(G4double)*ncomp);
    memcpy(yInOut.data(),y,sizeof(G4double)*fnvar);
    memcpy(dydxIn.data(),dydx,sizeof(G4double)*fnvar);

    stepper.try_step(system, yInOut, dydxIn, hdid, hnext);
    memcpy(y,yInOut.data(),sizeof(G4double)*fnvar);

    curveLength += hdid;
}

G4MagIntegratorStepper* BulirschStoerDriver::GetStepper(){
    return dummyStepper;
}