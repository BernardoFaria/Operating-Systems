/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */

#ifndef TIMER_H
#define TIMER_H


#include <sys/time.h>


#define TIMER_T                         struct timeval

#define TIMER_READ(time)                if(gettimeofday(&(time), NULL)){perror("gettimeofday failed"); exit(EXIT_FAILURE);}

#define TIMER_DIFF_SECONDS(start, stop) \
    (((double)(stop.tv_sec)  + (double)(stop.tv_usec / 1000000.0)) - \
     ((double)(start.tv_sec) + (double)(start.tv_usec / 1000000.0)))

#endif /* TIMER_H */
