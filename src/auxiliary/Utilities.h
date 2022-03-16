#ifndef UTILITIES_H
#define UTILITIES_H

#include <QObject>

class Utilities
{
   Q_GADGET

public:

   // linearly interpolate between a and b where 0 <= t ,+ 1
   //
   static inline double interp(const double& a, const double& b, const double& t)
   {
     return (a + (b-a)*t);
   }

   // compute the femoral neck T-Score for caucasian white female population
   //
   static inline double tscore(const double& bmd)
   {
       double M = 0.849f;     // median age matched reference value
       double L = 1.0f;       // skewness of the LMS reference curve corresponding to age peak BMD
       double sigma = 0.111f; // age matched population standard deviation
       return ((M * ((bmd / M ) - 1.0f))/(L * sigma));
   }
};

#endif // UTILITIES_H
