import os
import numpy as np
import matplotlib.pyplot as plt

def plot_waves(data):
    left_ch = data[:, 0]
    right_ch = data[:, 1]
        
    # Create time axis (assuming 44.1kHz sample rate)
    time = np.arange(len(left_ch)) / 44100

    # Plot
    plt.figure(figsize=(12, 6))
    # Left Channel Waveform
    plt.subplot(2, 1, 1)
    plt.plot(time, left_ch, 'b')
    plt.title("Left Channel")
    plt.ylabel("Amplitude")
    plt.xlim(0, 0.1)

    # Right Channel Waveform
    plt.subplot(2, 1, 2)
    plt.plot(time, right_ch, 'r')
    plt.title("Right Channel")
    plt.xlabel("Time (s)")
    plt.ylabel("Amplitude")
    plt.xlim(0, 0.1)

    plt.tight_layout()
    #plt.show()
    plt.savefig("plot.png")
    plt.close()

def plot_freq(data):
    left_ch = data[:, 0]
    right_ch = data[:, 1]

    # Finding the FFT
    time = np.arange(len(left_ch)) / 44100
    fft_result_left = np.fft.fft(left_ch)
    fft_result_right = np.fft.fft(right_ch)

    freqs_l = np.fft.fftfreq(len(left_ch))
    freqs_r = np.fft.fftfreq(len(right_ch))

    # Plotting
    plt.figure(figsize=(12, 6))

    # Left Channel Frequencies
    plt.subplot(2, 1, 1)
    plt.plot(freqs_l, np.abs(fft_result_left), 'b')
    plt.title("Left Channel")
    plt.ylabel("Magnitude")
    plt.xlabel("Frequency")

    # Right Channel Frequencies
    plt.subplot(2, 1, 2)
    plt.plot(freqs_r, np.abs(fft_result_right), 'r')
    plt.title("Right Channel")
    plt.ylabel("Magnitude")
    plt.xlabel("Frequency")

    plt.tight_layout()
    plt.savefig("freq_plot.png")

def main():
    try:
        # Read data from file
        data = np.loadtxt("audio_samples.txt")

        plot_waves(data)
        plot_freq(data)
    except Exception as e:
        print(f"Error plotting: {e}")

if __name__ == "__main__":
    main()
