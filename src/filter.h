#ifndef filter_h_INCLUDED
#define filter_h_INCLUDED

float low_pass(float input, float*prev_out, float alpha, float*prev_inp); 
float high_pass(float input, float*prev_out, float alpha, float*prev_inp);

#endif // filter_h_INCLUDED
