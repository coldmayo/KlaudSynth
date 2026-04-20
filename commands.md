# List of commands

## General

## Comments

- The sound struct can contain one or more waves which can all be played at once

### Commands

- pan
	- audio distribution if stereo
- p
	- play waves and their applied effects, filters, etc
- stop
	- stop playing sound
- veiw
	- see frequency distribution and total wave representation

## Waves and their types

### Comments

- Every time a new wave is made it becomes the current wave
	- You can see the index of the current wave in Wave[i]> where you type commands
	- This is the wave that commands are done to

### Commands
- new_wave
	- creates a new wave, defualts to sine wave at a C note
- msine
	- makes the current wave into a sine wave
- msq
	- makes the current wave into a square wave
- msaw
	- makes the current wave into a saw wave
- mtri
	- makes the current wave into a triangle wave
- note N O
	- N: note name
		- C, C#, D, D#, E, F, F#, G, G#, A, A#, B
	- O: octave
		- Octave above/below middle
			- -1, 1, etc
- ls wave
	- list all waves and its type
- cw n
	- change wave to switch focus (current wave) to
	- n (optional): index of wave
- nw
	- Next wave; switch current wave to next wave in index
- pw
	- Previous wave; switch current wave to previous wave in index
- rm wave
	- remove current wave

## Filters

### Comments

- Focus: You must set the focus to determine if you are adding/editing filters for a specific wave or the global master chain.
- Filter Limit: There is a maximum of 5 filters per wave and 5 filters for the global chain.

### Commands

- focus f
	- Switch focus, by defualt it is set to global focus, but can also switch it to wave specific
	- f = global or wave
- lpf / hpf / bpf / notch: Adds a Low Pass, High Pass, Band Pass, or Notch filter.
- lpf2 / hpf2: Adds a 2nd order version of the filter.
- pf
	- previous filter: switches onto the previous
- cutoff X
	- Sets the cutoff frequency (20Hz - 20,000Hz) for the currently selected filter.
- res X:
	- Sets the resonance (Q) for the currently selected filter.
- ls filter [global/wave]
	- Lists the filters in the chosen chain.
- cf n:
	- Change focus to filter at index n in the current chain.
- nf:
	- Switch to the Next Filter in the chain
- pf:
	- Switch to the Previous Filter in the chain.

## Effects

### Comments

- Effects are applied to the global sound structure.
- You must first add an effect using the fx command before you can adjust its parameters.

### Commands

- fx [type]
	- Adds a new effect to the chain
	- type: chorus, reverb, delay, flanger, phaser
- ls fx
	- Lists all active effects and their internal parameters (Mix, Feedback, LFO, etc.)
- mix X
	- Sets the Dry/Wet mix (0-100) for the currently selected effect
- feedback X:
	- Sets the feedback amount (0-100) for the currently selected effect
- time X:
	- Sets the time parameter (0.0 - 2.0) for effects like Delay
- pan X:
	- Sets the stereo distribution (-1.0 for Left, 1.0 for Right, 0.0 for Center).
- cfx n
	- change effect to switch focus (current fx) to
	- n (optional): index of fx
- nfx
	- Next effect; switch current effect to next effect in index
- pfx
	- Previous effect; switch current effect to previous effect in index
