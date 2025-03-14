#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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
	char * buff;
	bool created = false;
	while(1) {
		char * inp = malloc(MAXCOMMSIZE);
		printf("KldSynth> ");
		fgets(inp, MAXCOMMSIZE, stdin);
		if (strcmp(slice_str(inp, buff, 0, 4), "make") == 0) {
			if (strcmp(slice_str(inp, buff, 5, strlen(inp)), "sine") == 0) {
				created = true;
				GEN_INP input;
				generate(input);
			}
		} else if (strcmp(slice_str(inp, buff, 0, 9), "frequency") == 0 && created) {
			input.freq = atoi(slice_str(inp, buff, 11, strlen(inp)));
		} else if (strcmp(inp, "exit") == 0) {
			return 0;
		}
	}
}
