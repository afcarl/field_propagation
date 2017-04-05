#include "BulirschStoerDriver.hh"
#include "G4LineSection.hh"
#include "G4SystemOfUnits.hh"
#include "ModifiedMidpoint.hh"
#include "ModifiedMidpointDenseOut.hh"

#define ncomp G4FieldTrack::ncompSVEC

BulirschStoerDriver::BulirschStoerDriver(G4double hminimum, G4EquationOfMotion* equation,
                                         G4int integratedComponents, G4int verb):
    G4VIntegrationDriver(hminimum,equation,integratedComponents,verb),
    modifiedMidpoint(equation,integratedComponents),
    denseMidpoint(equation,integratedComponents),
    bulirschStoer(equation,integratedComponents,0,DBL_MAX),
#ifdef USE_BOOST
    boost_bulirsch_stoer(0, 0, 1, 0, 0),
#endif
    interval_sequence{2,4},
    fcoeff(1./(sqr(G4double(interval_sequence[1])/G4double(interval_sequence[0]))-1.))
{
}

BulirschStoerDriver::~BulirschStoerDriver()
{
}


G4bool  BulirschStoerDriver::AccurateAdvance(G4FieldTrack&  track, G4double hstep,
                                             G4double eps, G4double hinitial)
{
    G4int fNoTotalSteps = 0;
    G4int fMaxNoSteps = 10000;
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
    if(hstep < 0)
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
    track.DumpToArray(yCurrent);
    //copy non-integration variables to out array
    memcpy(yOut+GetNumberOfVariables(),
           yCurrent+GetNumberOfVariables(),
           sizeof(G4double)*(ncomp-GetNumberOfVariables()));

    G4double startCurveLength = track.GetCurveLength();
    G4double curveLength = startCurveLength;
    G4double endCurveLength = startCurveLength + hstep;

    //loop variables
    G4int nstp = 1, no_warnings = 0;
    G4double hnext, hdid;


    G4bool succeeded = true, lastStepSucceeded;

    G4int  noFullIntegr = 0, noSmallIntegr = 0 ;
    static G4ThreadLocal G4int  noGoodSteps = 0 ;  // Bad = chord > curve-len

    G4bool lastStep = false;

    //BulirschStoer.reset();

    G4FieldTrack yFldTrk(track);

    do
    {
        G4ThreeVector StartPos(yCurrent[0], yCurrent[1], yCurrent[2]);
#ifndef USE_BOOST
        GetEquationOfMotion()->RightHandSide(yCurrent, dydxCurrent);
#endif
        fNoTotalSteps++;

        // Perform the Integration
        if(h == 0){
            G4Exception("G4MagInt_Driver::AccurateAdvance()",
                        "GeomField0003", FatalException,
                        "Integration Step became Zero!");
        }
        else if(h > GetMinimumStep()){
            //step size if Ok
            OneGoodStep(yCurrent,dydxCurrent,curveLength,h,eps,hdid,hnext);
            lastStepSucceeded = (hdid == h);
        }
        else{
#ifdef USE_BOOST
            GetEquationOfMotion()->RightHandSide(yCurrent, dydxCurrent);
#endif
            // for small steps call QuickAdvance for speed

            G4double dchord_step, dyerr, dyerr_len;   // What to do with these ?
            yFldTrk.LoadFromArray(yCurrent, ncomp);
            yFldTrk.SetCurveLength(curveLength);

            QuickAdvance(yFldTrk, dydxCurrent, h, dchord_step, dyerr_len);

            yFldTrk.DumpToArray(yCurrent);


            dyerr = dyerr_len / h;
            hdid = h;
            curveLength += hdid;

            // Compute suggested new step
            //hnext = ComputeNewStepSize(dyerr/eps, h);
            hnext = h;

            //hnext= ComputeNewStepSize_WithinLimits( dyerr/eps, h);
            lastStepSucceeded = (dyerr <= eps);
        }


        lastStepSucceeded ? ++noFullIntegr : ++noSmallIntegr;


        G4ThreeVector EndPos(yCurrent[0], yCurrent[1], yCurrent[2]);

        // Check the endpoint
        G4double endPointDist = (EndPos - StartPos).mag();
        if (endPointDist >= hdid*(1. + perMillion))
        {
            ++fNoBadSteps;

            // Issue a warning only for gross differences -
            // we understand how small difference occur.
            if (endPointDist >= hdid*(1.+perThousand))
            {
                ++no_warnings;
            }
        }
        else
        {
            ++noGoodSteps;
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
            if(std::fabs(hnext) < GetMinimumStep())
            {
              // Make sure that the next step is at least Hmin.
              h = GetMinimumStep();
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

    //copy integrated vars to output array
    memcpy(yOut,yCurrent,sizeof(G4double)*GetNumberOfVariables());

    // upload new state
    track.LoadFromArray(yOut, ncomp);
    track.SetCurveLength(curveLength);

    if(nstp > fMaxNoSteps)
    {
      ++no_warnings;
      succeeded = false;
    }

    return succeeded;
}

G4bool  BulirschStoerDriver::QuickAdvance(G4FieldTrack& track, const G4double dydx[],
                                          G4double hstep, G4double& missDist,
                                          G4double& dyerr)
{

    track.DumpToArray(yIn);
    const G4double curveLength = track.GetCurveLength();

    denseMidpoint.set_steps(interval_sequence[0]);
    denseMidpoint.do_step(yIn, dydx, yOut, hstep, yMid, derivs[0]);

    denseMidpoint.set_steps(interval_sequence[1]);
    denseMidpoint.do_step(yIn, dydx, yOut2, hstep, yMid2, derivs[1]);

    //extrapolation
    for (G4int i = 0; i < GetNumberOfVariables(); ++i){
        yOut[i] = yOut2[i] + (yOut2[i] - yOut[i])*fcoeff;
        yMid[i] = yMid2[i] + (yMid2[i] - yMid[i])*fcoeff;
    }



    //calc chord lenght
    missDist = G4LineSection::Distline(G4ThreeVector(yMid[0],yMid[1],yMid[2]),
                                       G4ThreeVector(yIn[0],yIn[1],yIn[2]),
                                       G4ThreeVector(yOut[0],yOut[1],yOut[2]));

    //calc error
    for (G4int i = 0; i < GetNumberOfVariables(); ++i){
        yError[i] = yOut[i] - yOut2[i];
    }

    G4double errPos2 = sqr(yError[0]) + sqr(yError[1]) + sqr(yError[2]);
    G4double errMom2 = sqr(yError[3]) + sqr(yError[4]) + sqr(yError[5]);
    G4double Mom2 = sqr(yOut[3]) + sqr(yOut[4]) + sqr(yOut[5]);
    errMom2 /= Mom2;

    dyerr = std::sqrt(std::max(errPos2, errMom2*sqr(hstep)));


    //copy non-integrated variables to output array
    memcpy(yOut+GetNumberOfVariables(),
           yIn+GetNumberOfVariables(),
           sizeof(G4double)*(ncomp-GetNumberOfVariables()));

    //set new state
    track.LoadFromArray(yOut,ncomp);
    track.SetCurveLength(curveLength + hstep);

    return true;
}

#ifndef USE_BOOST
void  BulirschStoerDriver::OneGoodStep(G4double  y[], const G4double  dydx[],
                                       G4double& curveLength, G4double htry,
                                       G4double eps, G4double& hdid,
                                       G4double& hnext)
{
    hnext = htry;
    G4double curveLengthBegin = curveLength;

    // set maximum allowed error
    bulirschStoer.set_max_relative_error(eps);

    step_result res = step_result::fail;
    while (res == step_result::fail)
    {
        res = bulirschStoer.try_step(y, dydx, curveLength, yOut, hnext);
    }

    memcpy(y,yOut,sizeof(G4double)*GetNumberOfVariables());
    hdid = curveLength - curveLengthBegin;
}
#else
void  BulirschStoerDriver::OneGoodStep(G4double  y[], const G4double  /*dydx*/[],
                                       G4double& curveLength, G4double htry,
                                       G4double eps, G4double& hdid,
                                       G4double& hnext)
{
    hnext = htry;
    G4double curveLengthBegin = curveLength;

#ifdef MODIFIED_BOOST    
    // set maximum allowed error
    boost_bulirsch_stoer.set_max_relative_error(eps);
#endif
    
    boost::numeric::odeint::controlled_step_result res = boost::numeric::odeint::fail;
    state_type xIn, xOut;
    memcpy(xIn.data(), y, sizeof(G4double)*ncomp);
    auto system = [this](const state_type& x, state_type& dxdt, G4double /*t*/){
        this->GetEquationOfMotion()->RightHandSide(x.data(), dxdt.data());
    };

    while (res == boost::numeric::odeint::fail)
    {
        res = boost_bulirsch_stoer.try_step(system, xIn, curveLength, xOut, hnext);
    }

    memcpy(y,xOut.data(),sizeof(G4double)*GetNumberOfVariables());
    hdid = curveLength - curveLengthBegin;
}
#endif

G4double BulirschStoerDriver::ComputeNewStepSize(G4double /* errMaxNorm*/, G4double  hstepCurrent)
{
    return hstepCurrent;
}

G4bool BulirschStoerDriver::isDense() const
{return false;}
