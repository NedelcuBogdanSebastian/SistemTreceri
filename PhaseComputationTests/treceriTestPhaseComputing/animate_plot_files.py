import matplotlib.pyplot as plt
import numpy as np
import glob
import time
import re

# Constants for phase calculation
EPSILON = 1e-10
RAD2DEG = 180.0 / np.pi

# Function to calculate the phase of the k-th FFT bin
def myfftPhase(fft_data, nn, k):
    if k >= nn / 2:
        return 0.0  # Out of range

    # Extract the real and imaginary parts of the k-th element
    real_part = np.real(fft_data[k])
    imag_part = np.imag(fft_data[k])

    # Handle the case when both real and imaginary parts are zero (undefined phase)
    if np.abs(real_part) < EPSILON and np.abs(imag_part) < EPSILON:
        return 0.0  # Phase is undefined

    # Calculate the angle in radians using atan2
    angle_rad = np.arctan2(imag_part, real_part)

    # Convert the angle to degrees
    angle_deg = angle_rad * RAD2DEG

    # Apply the specific adjustment you mentioned (-222.884444)
    angle_deg -= 222.884444

    # Normalize the angle to be in the range [0, 360)
    if angle_deg < 0:
        angle_deg += 360.0
        
    # Normalize the angle to be in the range [0, 360)
    if angle_deg < 0:
        angle_deg += 360.0

    return angle_deg

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
    
    # Compute FFT
    n = len(real_part)  # Number of samples
    fft_result = np.fft.fft(real_part)
    fft_freq = np.fft.fftfreq(n, d=(time[1] - time[0]))  # Frequency bins

    # Compute energy (magnitude squared)
    energy = np.abs(fft_result)**2

    # Find the bin index of maximum energy
    max_energy_index = np.argmax(energy)
    max_energy = energy[max_energy_index]

    # Calculate phase using the translated myfftPhase function
    phase_at_max_energy_deg = myfftPhase(fft_result, n, 9)

    # Clear the current figure for the next plot
    plt.clf()

    # Plotting the real part of the sine wave
    plt.subplot(2, 1, 1)  # Create a 2x1 subplot
    #plt.plot(time, real_part, label='Sine Wave', color='b')
    plt.plot(time, real_part, color='b')
    plt.title(f'Sine Wave - {filename}')
    #plt.xlabel('Time (seconds)')
    #plt.ylabel('Amplitude')
    plt.grid()
    #plt.legend()
    plt.xlim(0, time[-1])  # Set x-axis limit based on time

    # Plotting the FFT results
    plt.subplot(2, 1, 2)  # Second subplot
    plt.plot(fft_freq[:n // 2], np.abs(fft_result)[:n // 2], color='r')  # Plot positive frequencies
    #plt.title(f'FFT of Sine Wave - {filename}')
    #plt.xlabel('Frequency (Hz)')
    #plt.ylabel('Magnitude')
    plt.grid()
    plt.xlim(0, 1000)  # Adjust limit as needed
    plt.tight_layout()

    # Print phase information
    print(f"File: {filename}")
    #print(f"Bin index of maximum energy: {max_energy_index}")
    #print(f"Maximum energy: {max_energy}")
    print(f"Phase at maximum energy (degrees): {phase_at_max_energy_deg}")

    # Draw the plot
    plt.draw()
    plt.pause(0.05)  # Pause for 1 second to simulate animation

# Function to extract the phase number from the filename
def extract_phase(filename):
    # Use regex to extract the floating-point number after 'sine_wave_'
    match = re.search(r"sine_wave_(\d+\.\d+)\.txt", filename)
    if match:
        return float(match.group(1))  # Return the phase as a float
    return 0.0

# Main function to animate the sine wave plots
if __name__ == '__main__':
    # Get all the sine wave files that match the pattern
    files = glob.glob('sine_wave_*.txt')
    
    # Sort files based on the numeric phase extracted from the filename
    files = sorted(files, key=extract_phase)

    # Initialize the plot
    #plt.figure(figsize=(12, 6))

    # Loop through all the files and plot each one
    for file in files:
        plot_sine_wave(file)
        #time.sleep(1)  # Optional additional delay between file loads

    # Keep the final plot on screen after animation ends
    plt.show()
