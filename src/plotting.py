import os
import numpy as np
import matplotlib.pyplot as plt

def main():
    try:
        # Read data from file
        data = np.loadtxt("audio_samples.txt")
        left_ch = data[:, 0]
        right_ch = data[:, 1]
        
        # Create time axis (assuming 44.1kHz sample rate)
        time = np.arange(len(left_ch)) / 44100

        
        # Plot
        plt.figure(figsize=(12, 6))
        plt.subplot(2, 1, 1)
        plt.plot(time, left_ch, 'b')
        plt.title("Left Channel")
        plt.ylabel("Amplitude")
        plt.xlim(0, 0.1)
        
        plt.subplot(2, 1, 2)
        plt.plot(time, right_ch, 'r')
        plt.title("Right Channel")
        plt.xlabel("Time (s)")
        plt.ylabel("Amplitude")
        plt.xlim(0, 0.1)
        
        plt.tight_layout()
        #plt.show()
        plt.savefig("plot.png")
        
    except Exception as e:
        print(f"Error plotting: {e}")

if __name__ == "__main__":
    main()
