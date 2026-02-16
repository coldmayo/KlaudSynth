#include <math.h>

#include "types.h"

double LFO_imp(double input, int sample_rate, LFO * lfo) {
    double phase_inc = lfo->rate / lfo->sample_rate;
    lfo->curr_phase += phase_inc;

    if (lfo->curr_phase >= 1.0) {
        lfo->curr_phase -= 1.0;
    }

    double output = 0.0;
    switch (lfo->type) {
		case WAVE_SINE:
    		output = sin(lfo->curr_phase * 2.0 * M_PI);
    		break;
    	case WAVE_SAW:
        	output = (lfo->curr_phase * 2.0) - 1.0;
        	break;
        case WAVE_SQUARE:
            output = (lfo->curr_phase < 0.5) ? 1.0 : -1.0;
            break;
        case WAVE_TRI:
            double saw = (lfo->curr_phase * 2.0) - 1.0;
            output = 2.0 * fabs(saw) - 1.0;
            break;
        default:
            output = 0.0;
            break;
    }

    return input + (output * lfo->depth);
}

