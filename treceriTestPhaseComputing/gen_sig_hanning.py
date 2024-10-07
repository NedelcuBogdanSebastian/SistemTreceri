import math
import numpy as np
import matplotlib.pyplot as plt

PI      =  3.14159265359
DEG2RAD =  0.01745329252
RAD2DEG = 180 / np.pi

def myatan2(y,x):
    v = 0
    if x > 0:
        v = math.atan(y/x)
    if y >= 0 and x < 0:
        v = PI + math.atan(y/x)
    if y < 0 and x < 0:
        v = -PI + math.atan(y/x)
    if y > 0 and x == 0:
        v = PI/2
    if y < 0 and x==0:
        v = -PI/2
    if v < 0:
        v = v + 2*PI
    return v

def get_wave_phase_as_complex(a):
    # compute FFT
    ft = np.fft.fft(a)
    return (ft[9].real, ft[9].imag)

def get_signal_phase(x, y):                
    ang = myatan2(y, x)
    #ang = np.arctan2(y, x)

    tmp = ang * RAD2DEG
    
    if tmp < 0:
        tmp = tmp + 360.0

    tmp = (tmp - 222.884444) % 360.0 # delta not quite from theory    
    return tmp

# Parameters
num_points = 2048       # Number of points in the buffer
rms_amplitude = 0.0025   # RMS amplitude in volts
frequency = 50.0        # Frequency in Hz
sample_rate = 11718.75  # Sample rate in Hz
phi = 0.8      # Phase shift

bin_freq = sample_rate / num_points
print(f'FFT frequency at bin 9: {bin_freq*9.0}')

# Calculate the peak amplitude from RMS
peak_amplitude = rms_amplitude * np.sqrt(2)

# Generate time array based on the number of points and sample rate
t_signal = np.arange(num_points) / sample_rate

# Generate the sine wave signal
# Frequency for bin 9 is 51.4984130859375 Hz, so there will be a fixed phase shift
# for the 50 Hz frequency (see get_signal_phase)
signal = peak_amplitude * np.sin(2 * np.pi * frequency * t_signal + np.deg2rad(phi))

# Apply a Hanning window
window = np.hanning(len(signal))
signal = signal * window

re,im = get_wave_phase_as_complex(signal)
phase = get_signal_phase(re, im)

print(f'Signal phase: {phase}')

# Plot the generated signal
plt.figure(figsize=(8, 4))
plt.plot(t_signal, signal, label=f'{phi} degrees sine wave', color='green')
plt.title(f'{frequency} Hz Sine Wave with RMS Amplitude of {rms_amplitude} V')
plt.xlabel('Time [s]')
plt.ylabel('Amplitude [V]')
plt.axhline(0, color='black',linewidth=0.5)
plt.grid(True)
plt.legend()
plt.show()

'''
1. Compute the True Phase Shift Between 50 Hz and frequency at FFT bin 9:

    The phase difference between the signal's true frequency (50 Hz) and the FFT bin
    frequency (51.4984 Hz) can be calculated.
    The difference in angular frequency is:
        Δω = 2π × (fbin − fsignal) = 2π × (51.4984130859375 − 50) = 2π × 1.4984130859375 ≈ 9.4148 rad/s

    If your FFT gives you the phase at bin 9 for the frequency 51.4984 Hz, the signal at 50 Hz will
    have advanced by this phase difference during the length of your signal.
    Let T be the total duration of the signal:
        T = num_points / sample_rate = 2048 / 11718.75 ≈ 0.174762666 seconds

    The total phase shift over this time T is:
        Δϕ = Δω × T = 9.4148 × 0.174762666 ≈ 1.646 radians ≈ 94.272∘

    So, the phase at bin 9 is approximately 94.272° ahead of the true phase at 50 Hz.
    
2. Apply Phase Correction:

    To correct the phase, you need to subtract this extra phase shift from the phase at bin 9 to account
    for the difference between the FFT bin frequency and the true signal frequency:
        ϕcorrected = ϕbin 9 − Δϕ

3. To prevent a 360° roll-up (i.e., phase values exceeding 360° or going negative after the correction),
   you'll want to normalize the phase into a standard range, typically between 0° and 360°
   
   Explanation:

   If ϕcorrected​ is negative, adding 360 will shift it into the positive range.
   If ϕcorrected​ exceeds 360°, taking modulo 360 will roll it back within the 0° to 360° range.

   Corrected Formula:
       ϕcorrected = (ϕbin 9 − 94.272 + 360) % 360 => ϕcorrected = (ϕbin 9 + 265.728) % 360
  
'''
