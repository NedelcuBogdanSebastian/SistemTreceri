/*************************************************************************************
    Copyright (C) 2024 Nedelcu Bogdan Sebastian
    This code is free software: you can redistribute it and/or modify it 
    under the following conditions:
    1. The use, distribution, and modification of this file are permitted for any 
       purpose, provided that the following conditions are met:
    2. Any redistribution or modification of this file must retain the original 
       copyright notice, this list of conditions, and the following attribution:
       "Original work by Nedelcu Bogdan Sebastian."
    3. The original author provides no warranty regarding the functionality or fitness 
       of this software for any particular purpose. Use it at your own risk.
    By using this software, you agree to retain the name of the original author in any 
    derivative works or distributions.
    ------------------------------------------------------------------------
    This code is provided as-is, without any express or implied warranties.
**************************************************************************************/

/*
    In this example we have the following:
        - we generate signals from 0 degrees to 359 degrees,
          of speciffic phase, with noise, and write the values to files named `sine_wave_x.x.txt`
          with 1.0 degree step
        - we generate the Flattop window and write to a file `flattop_window.txt`
        - we apply the window to the signal real side
        - we compute FFT
        - we compute the phase from bin 9 which is the nearest frequency from 50 Hz (51.4984130859375 Hz)
                 
    Also we have a function to henerate Hanning window, but we don't use it. 
*/

/*
   In the FFT at bin 9 we have a frequency of 51.4984130859375 Hz, and 
   between this and the 50 Hz, our fundamental, there is a phase difference.

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

*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define PI        3.1415926535897932384626433832795
#define DEG2RAD   0.01745329251994329576923690768489
#define RAD2DEG   57.295779513082320876798154814105  // 180/PI
#define EPSILON   1e-8 // Small value for zero comparison

#define NUM_POINTS 2048  // Number of points in the buffer
#define VALUES_PER_LINE 8  // Number of values to print per line

const size_t num_points = 2048;      // Number of points in the buffer
const float rms_amplitude = 0.025;   // RMS amplitude in volts
const float frequency = 50.0;        // Frequency in Hz
const float sample_rate = 11718.75;  // Sample rate in Hz (calculated to get index 9)    
const float noise_aplitude = 0.002;  // Noise level   
float signal_phase = 0.0;            // Signal phase

// Flattop Window: If the purpose of the test focus more on the energy value of a
// certain periodic signal frequency point. For example for Upeak, Upeak-peak, Urms,
// then the accuracy of its amplitude is more important, and a window with slighty
// wider lobes is better.
// The Flat Top window has a flatter passband, which helps retain accurate amplitude
// and phase information, particularly when analyzing signals where frequency peaks
// must be well-isolated. Since the window decreases the leakage, the phase extraction
// becomes more accurate. However, keep in mind that this window broadens the peaks,
// so you might sacrifice some resolution in the frequency domain.
void generate_flat_top_window(float *flattop_window, size_t num_points) {
    // Flat Top window coefficients
    const float a0 = 1.0;
    const float a1 = 1.93;
    const float a2 = 1.29;
    const float a3 = 0.388;
    const float a4 = 0.028;

    for (size_t n = 0; n < num_points; n++) {
        // Compute the normalized index value
        float factor = (2 * PI * n) / (num_points - 1);

        // Apply the Flat Top window function
        flattop_window[n] = a0
                          - a1 * cos(factor)
                          + a2 * cos(2 * factor)
                          - a3 * cos(3 * factor)
                          + a4 * cos(4 * factor);
    }
    
    // Open a file to write the values
    FILE *file = fopen("flattop_window.txt", "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file for writing\n");
        free(flattop_window);
        //return 1;
    }

    // Write window coefficients to file in the desired format
    fprintf(file, "float flattop_window[] = {\n");
    for (size_t i = 0; i < NUM_POINTS; i++) {
        fprintf(file, "%f", flattop_window[i]);
        if ((i + 1) % VALUES_PER_LINE == 0) {
            fprintf(file, ",\n");  // New line after every 8 values
        } else if (i < NUM_POINTS - 1) {
            fprintf(file, ", ");  // Comma and space between values
        }
    }
    fprintf(file, "\n};\n");  // Closing the array definition

    // Close window coefficients file
    fclose(file);    
}

/*
float myatan2(float y, float x) {
    if (fabs(x) < EPSILON) {
        if (y > 0) {
            return 90.0;   // Positive Y-axis
        } else if (y < 0) {
            return 270.0;  // Negative Y-axis
        } else {
            return 0.0;    // Origin case (x = 0, y = 0)
        }
    }

    float atan_value = atan(fabs(y) / fabs(x)) * RAD2DEG;

    if (x > 0 && y >= 0) {
        return atan_value;          // First Quadrant  (0° to 90°)
    } else if (x < 0 && y >= 0) {
        return 180.0 - atan_value;  // Second Quadrant (90° to 180°)
    } else if (x < 0 && y < 0) {
        return 180.0 + atan_value;  // Third Quadrant  (180° to 270°)
    } else {
        return 360.0 - atan_value;  // Fourth Quadrant (270° to 360°)
    }
}
*/

/* // This works OK for the test below, but not for FFT !!!
// In the FFT at bin 9 we have a frequency of 51.4984130859375 Hz, and 
// between this and 50 Hz, our fundamental, there is a phase difference.
float myatan2(float y, float x) {
    
    // Calculate the angle in radians using atan2
    float angle_rad = atan2(y, x);
    
    // Convert the angle to degrees
    float angle_deg = angle_rad * RAD2DEG;

    // Normalize the angle to be in the range [0, 360)
    if (angle_deg < 0) {
        angle_deg += 360.0;
    }

    return angle_deg;
}
*/

/*
Quadrant Determination: atan2(y, x) evaluates the angle based on the signs of 
both the x and y coordinates to determine the appropriate quadrant:
    First Quadrant: (x > 0, y > 0) → angle between 0° and 90°
    Second Quadrant: (x < 0, y > 0) → angle between 90° and 180°
    Third Quadrant: (x < 0, y < 0) → angle between 180° and 270°
    Fourth Quadrant: (x > 0, y < 0) → angle between 270° and 360° (or 0°)

Counterclockwise Measurement: The angle is measured counterclockwise from the positive x-axis:
    At the positive x-axis (i.e., (1, 0)), atan2(0, 1) returns 0 radians (0°).
    At the positive y-axis (i.e., (0, 1)), atan2(1, 0) returns π/2 radians (90°).
    At the negative x-axis (i.e., (-1, 0)), atan2(0, -1) returns π radians (180°).
    At the negative y-axis (i.e., (0, -1)), atan2(-1, 0) returns -π/2 radians 
        (or 3π/2 radians or 270° when normalized to the range [0, 2π)).
*/
/*void test_atan2() {
    printf("Testing math.h atan2:\n");
    printf("atan2(0, 0) = %.2f degrees, expected 0.00 degrees\n", atan2(0, 0) * RAD2DEG);        // 0.00 degrees
    printf("atan2(1, 1) = %.2f degrees, expected 45.00 degrees\n", atan2(1, 1) * RAD2DEG);       // 45.00 degrees
    printf("atan2(1, 0) = %.2f degrees, expected 90.00 degrees\n", atan2(1, 0) * RAD2DEG);       // 90.00 degrees
    printf("atan2(1, -1) = %.2f degrees, expected 135.00 degrees\n", atan2(1, -1) * RAD2DEG);    // 135.00 degrees
    printf("atan2(0, -1) = %.2f degrees, expected 180.00 degrees\n", atan2(0, -1) * RAD2DEG);    // 180.00 degrees
    printf("atan2(-1, -1) = %.2f degrees, expected -135.00 degrees\n", atan2(-1, -1) * RAD2DEG); // -135.00 degrees (or 225.00 degrees)
    printf("atan2(-1, 0) = %.2f degrees, expected -90.00 degrees\n", atan2(-1, 0) * RAD2DEG);    // -90.00 degrees (or 270.00 degrees)
    printf("atan2(-1, 1) = %.2f degrees, expected -45.00 degrees\n", atan2(-1, 1) * RAD2DEG);    // -45.00 degrees (or 315.00 degrees)
    printf("atan2(0, 1) = %.2f degrees, expected 0.00 degrees\n", atan2(0, 1) * RAD2DEG);        // 0.00 degrees

    printf("\n");
    printf("Testing my implementation of atan2:\n");
    printf("myatan2(0, 0) = %.2f degrees\n", myatan2(0, 0));     // 0.00 degrees
    printf("myatan2(1, 1) = %.2f degrees\n", myatan2(1, 1));     // 45.00 degrees
    printf("myatan2(1, 0) = %.2f degrees\n", myatan2(1, 0));     // 90.00 degrees
    printf("myatan2(1, -1) = %.2f degrees\n", myatan2(1, -1));   // 135.00 degrees
    printf("myatan2(0, -1) = %.2f degrees\n", myatan2(0, -1));   // 180.00 degrees
    printf("myatan2(-1, -1) = %.2f degrees\n", myatan2(-1, -1)); // 225.00 degrees
    printf("myatan2(-1, 0) = %.2f degrees\n", myatan2(-1, 0));   // 270.00 degrees  
    printf("myatan2(-1, 1) = %.2f degrees\n", myatan2(-1, 1));   // 315.00 degrees    
    printf("myatan2(0, 1) = %.2f degrees\n", myatan2(0, 1));     // 0.00 degrees
}
*/

// Calculates the FFT phase at a given frequency index.
// Input: data is complex FFT Re[V(0)],Im[V(0)], Re[V(1)],Im[V(1)],...
// Input: nn is the number of points in the data and in the FFT,
//           nn must be a power of 2
// Input: k is frequency index 0 to nn/2-1
//        E.g., if nn=16384, then k can be 0 to 8191
// Output: Phase at this frequency
// data is an array of 2*nn elements
// returns 0 if k >= nn/2
float myfftPhase (float data[], unsigned long nn, int k) {
    if (k >= nn / 2) {
        return 0.0; // out of range
    }

    // Extract the real and imaginary parts of the k-th element
    float real_part = data[2 * k];
    float imag_part = data[2 * k + 1];

    // Handle the case when both real and imaginary parts are zero (undefined phase)
    if (fabs(real_part) < EPSILON && fabs(imag_part) < EPSILON) {
        return 0.0; // phase is undefined
    }

    // Calculate the angle in radians using atan2
    float angle_rad = atan2(imag_part, real_part);

    // Convert the angle to degrees
    float angle_deg = angle_rad * RAD2DEG;

    angle_deg += 137.1126;
    
    if (angle_deg < 0.0) {
        angle_deg += 360.0;
    }
    
    return angle_deg;
}

// Helper macro to swap two float values
#define SWAP(a, b) { float temp = (a); (a) = (b); (b) = temp; }

// FFT optimized for a signal array only with real side values, no imaginary
// Input: nn is the number of points in the data and in the FFT (must be a power of 2).
// Input: data is an array of nn real elements (no imaginary part initially, from ADC)
//        Re(0),0,Re(1),0,Re(2),...Re(nn-1),0
// Output: data will be transformed to contain complex FFT coefficients where the real
//         and imaginary parts are interleaved in the same array (Re, Im, Re, Im...).
void real_fft (float data[], unsigned long nn) {
    unsigned long n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    // `nn` is the number of real samples. In the complex FFT,
    // `n` will be twice `nn` because each complex number has two parts (Re and Im).
    n = nn << 1;  // n = 2 * nn, for real + imaginary storage (even though input is real, output will be complex)

    // ---- Bit-reversal Reordering ----
    // The FFT requires the input to be in bit-reversed order to optimize
    // in-place computation. Here we rearrange the data in such order.
    j = 1;
    for (i = 1; i < n; i += 2) {
        if (j > i) {  // Swap only if j > i to avoid swapping elements back
            SWAP(data[j-1], data[i-1]);  // Swap the real part
            SWAP(data[j], data[i]);      // Swap the imaginary part (initially 0)
        }

        // Bit-reversal logic (this shifts the bits around in a specific way
        // to reorder the data).
        m = n >> 1;
        while (m >= 2 && j > m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }

    // ---- Danielson-Lanczos Recursion ----
    // This is the heart of the FFT algorithm, where the computation is performed
    // in a recursive, divide-and-conquer manner.
    mmax = 2;  // mmax starts at 2 (which means we first handle 2-element blocks)
    while (n > mmax) {
        istep = mmax << 1;  // Step size for each FFT recursion level (block size)

        // Calculate the angle (theta) for the trigonometric recurrence relation
        // that is used to compute the FFT (twiddle factors).
        theta = -2.0 * PI / mmax;
        wtemp = sin(0.5 * theta);
        wpr = -2.0 * wtemp * wtemp;  // Real part of twiddle factor
        wpi = sin(theta);            // Imaginary part of twiddle factor
        wr = 1.0;                    // Starting value for twiddle real part (cos(0) = 1)
        wi = 0.0;                    // Starting value for twiddle imaginary part (sin(0) = 0)

        // For each recursion level, we loop through the data in chunks
        // of size `mmax`, computing the FFT step for each pair of elements.
        for (m = 1; m < mmax; m += 2) {
            for (i = m; i <= n; i += istep) {
                // The FFT is performed in pairs of elements. We compute the
                // real and imaginary parts of these elements and apply the twiddle factors.
                j = i + mmax;  // This is the other element in the pair

                // Calculate the real and imaginary components of the twiddle factor for this step
                tempr = wr * data[j-1] - wi * data[j];
                tempi = wr * data[j] + wi * data[j-1];

                // Update the real and imaginary parts with the calculated values
                data[j-1] = data[i-1] - tempr;
                data[j]   = data[i]   - tempi;
                data[i-1] = data[i-1] + tempr;
                data[i]   = data[i]   + tempi;
            }

            // Update the twiddle factors for the next iteration. This is done
            // using the recurrence relation for trigonometric functions.
            wtemp = wr;
            wr = wr * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
        }

        // Double the block size for the next level of recursion.
        mmax = istep;
    }
}

// Generate a sine wave buffer with phase in degrees and write to a file
void generate_sine_wave(float *signal, size_t num_points, float rms_amplitude, float frequency, float sample_rate, float phase_degrees, float noise_amplitude, const char* filename) {
    // Convert phase from degrees to radians
    float phase_radians = phase_degrees * (PI / 180.0);

    // Calculate peak amplitude from RMS
    float peak_amplitude = rms_amplitude * sqrt(2.0);
    
    int k = 0;
    for (size_t i = 0; i < num_points * 2; i += 2) {
        // Calculate time based on sample rate
        float time = (float)k / sample_rate;

        // Add noise to the sine wave value
        float noise = ((float)rand() / RAND_MAX) * 2.0f * noise_amplitude - noise_amplitude; // Noise in range [-noise_amplitude, noise_amplitude]

        // Generate sine wave value with phase and noise
        
        // Real part
        signal[i] = peak_amplitude * sin(2.0 * PI * frequency * time + phase_radians) + noise;
                
        // Imaginary part
        signal[i + 1] = 0.0;
        
        k += 1;
    }
   
    // Write the generated buffer to a file
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    
    for (size_t i = 0; i < num_points * 2; i += 2) {
        // Write real and imaginary parts to the file
        fprintf(file, "%f %f\n", signal[i], signal[i + 1]);
    }
    
    fclose(file);
}

// Apply the Flat Top window to the signal
void apply_flattop_window(float *signal, const float *flattop_window, int num_points) {
    for (size_t n = 0; n < num_points; n++) {
        signal[n * 2] = signal[n * 2] * flattop_window[n]; // Apply the window to the real part
    }
}

// Hanning Window (Von Hann): If the test signal has multiple frequency components, 
// the spectrum is very complex, and the purpose of the test is more focused on 
// the frequency rather the energy.
void generate_hanning_window(float *window, size_t num_points) {
    for (size_t n = 0; n < num_points; n++) {
        window[n] = 0.5 * (1 - cos(2 * M_PI * n / (num_points - 1)));
    }
}

int main() {
    float *flattop_window = (float *)malloc(NUM_POINTS * sizeof(float)); // Allocate memory for the window
    float *signal = (float *)malloc(2 * NUM_POINTS * sizeof(float)); // Allocate memory for the signal

    //test_atan2();
    //printf("\n");

    // Generate Flattop Window coefficients and write to file
    generate_flat_top_window(flattop_window, NUM_POINTS);
    
    while (signal_phase < 359.0) {
        // Create a file name that includes the signal phase
        char filename[30];  // Adjust the size as needed
        sprintf(filename, "sine_wave_%.1f.txt", signal_phase);
    
        // Generate the sine wave with phase and noise and write to file
        generate_sine_wave(signal, num_points, rms_amplitude, frequency, sample_rate, signal_phase, noise_aplitude, filename);
    
        // Apply the Flattop window to the signal
        apply_flattop_window(signal, flattop_window, num_points);
    
        // Compute FFT
        real_fft(signal, 2048);
    
        float phase = myfftPhase(signal, 2048, 9);    
        printf("Signal phase: %.4f, Computed phase %.4f\n", signal_phase, phase);
    
        signal_phase += 1.0; 
    }

    // Free allocated memory
    free(signal);
    free(flattop_window);
    
    printf("\nPress any key...\n");
    getchar();  // Waits for a keypress

    return 0;
}
