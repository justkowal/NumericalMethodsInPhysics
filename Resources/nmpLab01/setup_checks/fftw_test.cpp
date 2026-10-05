// Ungraded environment check: a four-point complex FFT round trip.
#include <fftw3.h>
#include <algorithm>
#include <cmath>
#include <iostream>

int main() {
    constexpr int n = 4;
    fftw_complex* input = fftw_alloc_complex(n);
    fftw_complex* spectrum = fftw_alloc_complex(n);
    fftw_complex* recovered = fftw_alloc_complex(n);
    if (!input || !spectrum || !recovered) {
        std::cerr << "FAIL: FFTW allocation\n";
        fftw_free(input); fftw_free(spectrum); fftw_free(recovered);
        return 1;
    }
    fftw_plan forward = fftw_plan_dft_1d(n, input, spectrum, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_plan inverse = fftw_plan_dft_1d(n, spectrum, recovered, FFTW_BACKWARD, FFTW_ESTIMATE);
    if (!forward || !inverse) {
        std::cerr << "FAIL: FFTW planning\n";
        if (forward) fftw_destroy_plan(forward);
        if (inverse) fftw_destroy_plan(inverse);
        fftw_free(input); fftw_free(spectrum); fftw_free(recovered);
        return 1;
    }
    // Initialize after planning so a different planning flag cannot erase the test input.
    for (int j = 0; j < n; ++j) {
        input[j][0] = (j == 0 ? 1.0 : 0.0);
        input[j][1] = 0.0;
    }
    fftw_execute(forward);
    bool finite = true;
    double spectrum_error = 0.0;
    for (int j = 0; j < n; ++j) {
        finite = finite && std::isfinite(spectrum[j][0]) && std::isfinite(spectrum[j][1]);
        spectrum_error = std::max(spectrum_error,
            std::hypot(spectrum[j][0] - 1.0, spectrum[j][1]));
    }
    fftw_execute(inverse);
    double roundtrip_error = 0.0;
    for (int j = 0; j < n; ++j) {
        const double re = recovered[j][0] / n;
        const double im = recovered[j][1] / n;
        const double expected = (j == 0 ? 1.0 : 0.0);
        finite = finite && std::isfinite(re) && std::isfinite(im);
        roundtrip_error = std::max(roundtrip_error, std::hypot(re - expected, im));
    }
    fftw_destroy_plan(forward); fftw_destroy_plan(inverse);
    fftw_free(input); fftw_free(spectrum); fftw_free(recovered);
    std::cout << "Forward-spectrum error = " << spectrum_error << '\n'
              << "Normalized round-trip error = " << roundtrip_error << '\n';
    if (!finite || spectrum_error > 1e-12 || roundtrip_error > 1e-12) {
        std::cerr << "FAIL: Fourier check outside tolerance\n";
        return 1;
    }
    std::cout << "FFTW check PASS\n";
    return 0;
}
