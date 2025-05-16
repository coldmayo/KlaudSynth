compile:
	@gcc -o KlaudSynth -lasound -lm main.c amp.c filter.c gen.c effects.c plots.c save_waves.c `pkg-config --cflags --libs gtk+-3.0` -lgthread-2.0

run:
	@./KlaudSynth
