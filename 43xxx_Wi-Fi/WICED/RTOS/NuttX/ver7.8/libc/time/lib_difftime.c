/* lib_difftime.c */

#include <time.h>

double difftime(const time_t time1, const time_t time0)
  {
    if (time1 >= time0)
      {
        return time1 - time0;
      }
    else
      {
        return -(double) (time0 - time1);
      }
  }
