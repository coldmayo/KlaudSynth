#ifndef filter_h_INCLUDED
#define filter_h_INCLUDED

double LPF(double alpha, double curr_sample, double * prev_output);
double LPF_2(double input, double * prev_inp_1, double * prev_inp_2, double * prev_out_1, double * prev_out_2, double Q, double cutoff_freq, int sample_rate);
double HPF(double alpha, double curr_sample, double * prev_out, double * prev_inp);
double HPF_2(double input, double * prev_inp_1, double * prev_inp_2, double * prev_out_1, double * prev_out_2, double Q, double cutoff_freq, int sample_rate);
double BPF(double input, double * prev_inp_1, double * prev_inp_2, double * prev_out_1, double * prev_out_2, double Q, double center_freq, int sample_rate);
double Notch(double input, double center_freq, double * prev_inp_1, double * prev_inp_2, double * prev_out_1, double * prev_out_2, double Q, double sample_rate);
double peakingEQ(double input, double G, double* prev_inp_1, double* prev_inp_2, double* prev_out_1, double* prev_out_2, double Q, double center_freq, int sample_rate);
double lowShelf(double input, double G_db, double* prev_inp_1, double* prev_inp_2, double* prev_out_1, double* prev_out_2, double Q, double center_freq, int sample_rate);
double highShelf(double input, double G_db, double* prev_inp_1, double* prev_inp_2, double* prev_out_1, double* prev_out_2, double Q, double center_freq, int sample_rate);
double all_pass(double input, double center_freq, double* prev_inp_1, double* prev_inp_2, double* prev_out_1, double* prev_out_2, double Q, int sample_rate);

#endif // filter_h_INCLUDED
