import numpy as np
import matplotlib.pyplot as plt

def generate_sine_wave(num_points, rms_amplitude, frequency, sample_rate, phase_degrees, noise_amplitude):
    # Convert phase from degrees to radians
    phase_radians = np.deg2rad(phase_degrees)

    # Calculate peak amplitude from RMS
    peak_amplitude = rms_amplitude * np.sqrt(2.0)

    # Time array
    t = np.arange(num_points) / sample_rate
    
    # Generate the sine wave
    sine_wave = peak_amplitude * np.sin(2.0 * np.pi * frequency * t + phase_radians)
    
    # Generate noise
    noise = np.random.uniform(-noise_amplitude, noise_amplitude, num_points)
    
    # Add noise to the sine wave
    noisy_signal = sine_wave + noise
    
    return t, noisy_signal, sine_wave

# Parameters from your specification
num_points = 2048                # Number of points in the buffer
rms_amplitude = 0.025            # RMS amplitude in volts
frequency = 50.0                 # Frequency in Hz
sample_rate = 11718.75           # Sample rate in Hz
signal_phase = 175.0               # Phase in degrees
noise_amplitude = 0.002         # Small noise amplitude

# Generate the sine wave with noise
t, noisy_signal, sine_wave = generate_sine_wave(num_points, rms_amplitude, frequency, sample_rate, signal_phase, noise_amplitude)

# Plotting
#plt.figure(figsize=(12, 6))
plt.plot(t, noisy_signal, label='Noisy Signal', color='red', alpha=0.7)
plt.plot(t, sine_wave, label='Sine Wave', color='blue', alpha=0.5)
plt.title('Sine Wave with Noise')
plt.xlabel('Time (s)')
plt.ylabel('Amplitude (V)')
plt.legend()
plt.grid()
#plt.xlim(0, t[-1])
#plt.ylim(-0.03, 0.03)  # Adjusting limits to show the signal properly
plt.show()
