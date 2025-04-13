#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include "types.h"
#include "gen.h"

#define MAXCOMMSIZE 256

char * slice_str(const char * str, char * buffer, int start, int end) {
    int j = 0;
    for (int i = start; i <= end; ++i) {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
    return buffer;
}

// notation: ./synth <frequency> <amplitude> <phase> <seconds> <sample rate> <wave form> <channels> <device>

int main(int argc, char ** argv) {
	//GEN_INP input = {atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), argv[4], argv[5], argv[6], argv[7]};

	//generate(input);
	char buff[64];
	bool playing = false;
	while(1) {
		char * inp = malloc(MAXCOMMSIZE);
		printf("KldSynth> ");
		fgets(inp, MAXCOMMSIZE, stdin);
		if (strcmp(slice_str(inp, buff, 0, 5), ":msine") == 0) {
    		printf("done");
			playing = true;
			GEN_INP input = {440, 1000, 0.5, 0, 2, 44100, "sine", "stereo", "default"};
			generate(input);
		} else if (strcmp(slice_str(inp, buff, 0, 4), ":stop") == 0) {
			if (playing) {
				
			}
		} else if (strcmp(inp, "exit") == 0) {
			return 0;
		}
	}
}
