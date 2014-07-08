

#include "G4ChargeState.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "sqrt.h"

#include <blaze/math/StaticVector.h>

using blaze::StaticVector;

typedef StaticVector<G4double, 3UL> Blaze3DVec;
typedef StaticVector<G4double, 8UL> Blaze8DVec;

template 
<class Field_t>
class TMagFieldEquation : public G4Mag_UsualEqRhs
{
    public:

        typedef Field_t T_Field;
        
        TMagFieldEquation(T_Field* f)
            : G4Mag_UsualEqRhs(f)
        {
            itsField = f;
        }

        virtual ~TMagFieldEquation(){;}

        __attribute__((always_inline)) 
        void GetFieldValue(const G4double Point[4],
                G4double Field[]) const
        {
            itsField->T_Field::GetFieldValue(Point, Field);
        }

        __attribute__((always_inline)) 
        void RightHandSide(const G4double y[], G4double dydx[] )
            //	const
        {
                G4double Field[G4maximum_number_of_field_components]; 
                G4double  PositionAndTime[4];
                PositionAndTime[0] = y[0];
                PositionAndTime[1] = y[1];
                PositionAndTime[2] = y[2];
                PositionAndTime[3] = y[7];   
                GetFieldValue(PositionAndTime, Field) ;
                TEvaluateRhsGivenB(y, Field, dydx);
        }

        __attribute__((always_inline)) 
        void TEvaluateRhsGivenB( const G4double y[],
                const G4double B[3],
                G4double dydx[] ) const
        {
            Blaze3DVec Bv(3, B);
            Blaze3DVec yv(y[3],y[4],y[5]);
            Blaze8DVec dydxv;

            //inner product
            G4double momentum_mag_square = (yv, yv);

            G4double inv_momentum_magnitude = vdt::fast_isqrt_general( momentum_mag_square, 4);
            G4double cof = FCof()*inv_momentum_magnitude;

            dydx[0] = y[3]*inv_momentum_magnitude;       //  (d/ds)x = Vx/V
            dydx[1] = y[4]*inv_momentum_magnitude;       //  (d/ds)y = Vy/V
            dydx[2] = y[5]*inv_momentum_magnitude;       //  (d/ds)z = Vz/V

            dydx[3] = cof*(y[4]*B[2] - y[5]*B[1]) ;  // Ax = a*(Vy*Bz - Vz*By)
            dydx[4] = cof*(y[5]*B[0] - y[3]*B[2]) ;  // Ay = a*(Vz*Bx - Vx*Bz)
            dydx[5] = cof*(y[3]*B[1] - y[4]*B[0]) ;  // Az = a*(Vx*By - Vy*Bx)

            return ;
        }

    private:
        enum { G4maximum_number_of_field_components = 24 };
        T_Field *itsField;
};

