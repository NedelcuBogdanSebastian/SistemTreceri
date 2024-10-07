import matplotlib.pyplot as plt
import numpy as np

# Function to read the sine wave data from a file, plot it, compute FFT, and analyze
def plot_sine_wave(filename):
    # Lists to store time values and real parts
    time = []
    real_part = []
    
    # Read the data from the file
    with open(filename, 'r') as file:
        for line in file:
            # Split the line into real and imaginary parts
            values = line.split()
            if len(values) >= 2:
                real = float(values[0])
                # Append the real part and time to the lists
                real_part.append(real)
                time.append(len(time) / 11718.75)  # Assuming 11718.75 samples per second

    # Convert lists to numpy arrays for FFT computation
    real_part = np.array(real_part)
    time = np.array(time)
    
    # Plotting the real part of the sine wave
    plt.figure(figsize=(12, 6))
    plt.subplot(2, 1, 1)  # Create a 2x1 subplot
    plt.plot(time, real_part, label='Sine Wave', color='b')
    plt.title('Sine Wave')
    plt.xlabel('Time (seconds)')
    plt.ylabel('Amplitude')
    plt.grid()
    plt.legend()
    plt.xlim(0, time[-1])  # Set x-axis limit based on time

    # Compute FFT
    n = len(real_part)  # Number of samples
    fft_result = np.fft.fft(real_part)
    fft_freq = np.fft.fftfreq(n, d=(time[1] - time[0]))  # Frequency bins

    # Compute energy (magnitude squared)
    energy = np.abs(fft_result)**2

    # Find the bin index of maximum energy
    max_energy_index = np.argmax(energy)
    max_energy = energy[max_energy_index]

    # Print results
    print(f"Bin index of maximum energy: {max_energy_index}")
    print(f"Maximum energy: {max_energy}")
    phase_at_max_energy = np.angle(fft_result[max_energy_index])  # Phase in radians
    phase_at_max_energy_deg = np.degrees(phase_at_max_energy)

    # Normalize phase to be in [0, 360)
    if phase_at_max_energy_deg < 0:
        phase_at_max_energy_deg += 360

    print(f"Phase at maximum energy (radians): {phase_at_max_energy}")
    print(f"Phase at maximum energy (degrees): {phase_at_max_energy_deg}")

    # Plotting the FFT results
    plt.subplot(2, 1, 2)  # Second subplot
    plt.plot(fft_freq[:n // 2], np.abs(fft_result)[:n // 2], color='r')  # Plot positive frequencies
    plt.title('FFT of Sine Wave')
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Magnitude')
    plt.grid()
    plt.xlim(0, 1000)  # Adjust limit as needed
    plt.tight_layout()
    plt.show()

# Example usage
if __name__ == '__main__':
    plot_sine_wave('sine_wave_344.0.txt')
