#ifndef init_h_INCLUDED
#define init_h_INCLUDED

#include "types.h"

FILTER * init_filter(FILTER * filter, int index, FILTER_TYPES type);
WAVE * init_wave(WAVE * dawave, int i);
SOUND * init_sound();

#endif // init_h_INCLUDED
