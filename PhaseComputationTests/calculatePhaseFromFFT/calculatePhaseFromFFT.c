
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define PI        3.1415926535897932384626433832795
#define DEG2RAD   0.01745329251994329576923690768489
#define RAD2DEG   57.295779513082320876798154814105  // 180/PI
#define EPSILON   1e-8 // Small value for zero comparison

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
}


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

	//angle_deg = fmodf((angle_deg + 137.11), 360.0);

	//angle_deg += 137.11;

	angle_deg -= 222.884444;

	// Normalize the angle to be in the range [0, 360)
	if (angle_deg < 0) {
		angle_deg += 360.0;
	}

	//if (angle_deg >= 360.0)
	//    return 0.0;

	// Adjust phase based on phase difference between bin 9 an 50 Hz
	// ( Value is not equal to theory, like always :))) )
	//angle_deg = fmodf((angle_deg - 222.884444), 360.0);

	// Normalize the angle to be in the range [0, 360)
	if (angle_deg < 0) {
		angle_deg += 360.0;
	}

	return angle_deg;
}

// Helper macro to swap two float values
#define SWAP(a, b) { float temp = (a); (a) = (b); (b) = temp; }

// Original FFT function (now we use one optimized only for real side computation)
// Input: nn is the number of points in the data and in the FFT,
//           nn must be a power of 2
// Input: data is sampled voltage v(0),0,v(1),0,v(2),...v(nn-1),0 versus time
// Output: data is complex FFT Re[V(0)],Im[V(0)], Re[V(1)],Im[V(1)],...
// data is an array of 2*nn elements
void real_fft (float data[], unsigned long nn) {
    unsigned long n,mmax,m,j,istep,i;
    double wtemp,wr,wpr,wpi,wi,theta;
    double tempr,tempi;

    n = nn << 1;  // n is the size of data array (2*nn)
    j = 1;
    for (i=1; i<n; i+=2) {
        if (j > i) {   // bit reversal section
            SWAP(data[j-1], data[i-1]);
            SWAP(data[j], data[i]);
        }

        m = n >> 1;

        while ((m >= 2)&&(j > m)) {
            j = j - m;
            m = m >> 1;
        }

        j = j + m;
    }

    // Danielson-Lanczos section
    mmax = 2;
    while (n > mmax) {    // executed log2(nn) times
        istep = mmax << 1;
        theta = -6.283185307179586476925286766559 / mmax;
        // the above line should be + for inverse FFT
        wtemp = sin(0.5 * theta);
        wpr = -2.0 * wtemp * wtemp;  // real part
        wpi = sin(theta);        // imaginary part
        wr = 1.0;
        wi = 0.0;

        for (m=1; m<mmax; m+=2) {
            for (i=m; i<=n; i=i+istep) {
                j = i + mmax;
                tempr     = wr * data[j-1] - wi * data[j]; // Danielson-Lanczos formula
                tempi     = wr * data[j] + wi * data[j-1];
                data[j-1] = data[i-1] - tempr;
                data[j]   = data[i] - tempi;
                data[i-1] = data[i-1] + tempr;
                data[i]   = data[i] + tempi;
            }

            wtemp = wr;
            wr = wr * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
        }

        mmax = istep;
    }
}

// Generate a sine wave buffer with phase in degrees
void generate_sine_wave(float *signal, size_t num_points, float rms_amplitude, float frequency, float sample_rate, float phase_degrees, float noise_amplitude) {
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

	const size_t num_points = 2048;      // Number of points in the buffer
	const float rms_amplitude = 0.025;   // RMS amplitude in volts
	const float frequency = 50.0;        // Frequency in Hz
	const float sample_rate = 11718.75;  // Sample rate in Hz (calculated to get index 9)
	float signal_phase = 0.0;
	const float noise_amplitude = 0.002;

	float *flattop_window = (float *)malloc(num_points * sizeof(float)); // Allocate memory for the window
	float *signal = (float *)malloc(2 * num_points * sizeof(float)); // Allocate memory for the signal

	// Generate Flattop Window coefficients and write to file
	generate_flat_top_window(flattop_window, num_points);


	while (signal_phase < 359.0) {

		// Generate the sine wave with phase and noise
		generate_sine_wave(signal, num_points, rms_amplitude, frequency, sample_rate, signal_phase, noise_amplitude);

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
