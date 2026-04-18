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
- ls
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

- You can either impose filters on waves individually (wave specific focus) or on all waves at once (global)

### Commands

- focus f
	- Switch focus, by defualt it is set to global focus, but can also switch it to wave specific
	- f = global or wave
- pf
	- previous filter: switches onto the previous
