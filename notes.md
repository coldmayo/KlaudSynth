I am planning on building a synthesizer, it is an application that will generate/alter wave forms and save them to sound files (i.e. wav file).

Basics:
- A synthesizer is basically a software that has an ocillator that generates a waveform and the user is able to change its pitch
	- A filter is used to change the timbre
	- An ampilifer is used to control the volume + more effects
- Filters: Used to get rid of frequencies we don't want
	- High pass filters: passes signals with a frequency higher than the cuttoff frequency
	- Low pass: passes signals with a frequency lower than the cuttoff frequency
	- Band pass:

Goal:
- I want to implement this code for a real hardware synthesizer powered by the STM32
	- Check the output.pdf file to see what I am thinking on that end

Example code to review:
- Generate sound from ALSA and libasound: https://gist.github.com/ghedo/963382/815c98d1ba0eda1b486eb9d80d9a91a81d995283
