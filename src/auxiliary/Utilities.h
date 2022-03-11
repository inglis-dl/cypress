#ifndef UTILITIES_H
#define UTILITIES_H

#include <QObject>

class Utilities
{
   Q_GADGET

public:    
   static inline double interp(const double& a, const double& b, const double& t)
   {
     return (a + (b-a)*t);
   }
};

#endif // UTILITIES_H
