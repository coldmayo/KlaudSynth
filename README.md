# Klaud Synth

My goal to make a lightweight synthesizer in C to be used with Linux.

I wanted to learn DSP and I love music so this felt like an obvious choice for a project.

## Capabilities

- Sine, Saw, Triangle, and Square wave options
- LPF/HPF (1st and 2nd order), Bandpass, Notch, and more filters
- chorus, delay, reverb, and more effects
- LFO support
- Envelope editing
- FFT/IFFT from scratch
- 1D Convolution from scratch

See the commands.md file for more info

## Setup/Requirements

This will only work if your OS uses the Advanced Linux Sound Architecture (ALSA). So I assume non-Linux machines are unable to run this.

```
# clone repo
$ git clone https://github.com/coldmayo/KlaudSynth.git

# build executable
$ cd src
$ make

# build virtual environment
$ ./build_venv.sh

# run program
$ make run
```

The virtual environment is for GUI made with Python. Might replace with GTK with C++ soon.
