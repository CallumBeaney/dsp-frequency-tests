#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../external-libs/kissfft-master/kiss_fft.h"

double getFrequencySample (int index);
double getHighestMagnitude(kiss_fft_cpx *);
double* getNormalisedSignalVector(double, kiss_fft_cpx *);
int normaliseSingleFrequencyMagnitude(double, double);

static const double AMPLITUDE = 1.0; // 0.0 to 1.0
#define SAMPLE_RATE 4000
#define VECTOR_LENGTH 1024

#include "./harmonics.c"


int main(void) {

  double sineWaveVector[VECTOR_LENGTH];

  for (int i = 0; i < VECTOR_LENGTH; i++) {
    /// frequency bin [i] = Amp・sin(2π・f・t) + (Amp/2)・sin((2π・harmonic・t) )
    /// here we're simulating amplitude levels similar to what we'd get from IRL recording
    sineWaveVector[i] = (AMPLITUDE * sin(2.0 * M_PI * FUNDAMENTAL * i / SAMPLE_RATE )) 
                      + ((AMPLITUDE * 10) * sin(2.0 * M_PI * HARMONIC_SERIES[0] * i / SAMPLE_RATE ))
                      + ((AMPLITUDE * 5)  * sin(2.0 * M_PI * HARMONIC_SERIES[1] * i / SAMPLE_RATE ))
                      + ((AMPLITUDE * 10) * sin(2.0 * M_PI * HARMONIC_SERIES[2] * i / SAMPLE_RATE ))
                      + ((AMPLITUDE)      * sin(2.0 * M_PI * HARMONIC_SERIES[3] * i / SAMPLE_RATE ))
                      + ((AMPLITUDE * 4)  * sin(2.0 * M_PI * HARMONIC_SERIES[4] * i / SAMPLE_RATE ))
                      + ((AMPLITUDE / 2)  * sin(2.0 * M_PI * HARMONIC_SERIES[5] * i / SAMPLE_RATE ));
  }

  kiss_fft_cfg cfg = kiss_fft_alloc(VECTOR_LENGTH, 0, NULL, NULL);
  kiss_fft_cpx cx_in[VECTOR_LENGTH];
  kiss_fft_cpx cx_out[VECTOR_LENGTH];
  
  /// load input buffers with synthesised sine wave
  for (int i = 0; i < VECTOR_LENGTH; i++) {
    cx_in[i].r = sineWaveVector[i];
    cx_in[i].i = sineWaveVector[i]; /// We don't care about the imaginary plane for this; follow API convention.
  }

  kiss_fft(cfg, cx_in, cx_out); // DO FFT

  double maxMag = getHighestMagnitude(cx_out);
  // double *normalisedSignal = getNormalisedSignalVector(maxMag, cx_out);
  // free(normalisedSignal);


  printf("\nSample Rate:\t%d\t/s\nSine Freq.:\t%d\tHz\nMax Magn.:\t%d\tA\n\n", SAMPLE_RATE, (int)FUNDAMENTAL, (int)maxMag);

  printf("ind\tHz\tsine\tmagn.\tnormalised\n\n");
  for (int i = 0; i < VECTOR_LENGTH; i++) {
    if (fabsf(cx_out[i].r) >= 100) {
      int frequency = i * SAMPLE_RATE / VECTOR_LENGTH;
      int normalised = normaliseSingleFrequencyMagnitude(cx_out[i].r, maxMag);

      printf("%d\t%dHz\t", i, frequency);
      printf("%.2f\t", fabs(sineWaveVector[i]));

      if (normalised >= 20) {
        printf("\033[31m%.1f \t", fabsf(cx_out[i].r));
        printf("%d \t", normalised);
        printf("<------\033[0m");
      } else {
        printf("%.1f\t", fabsf(cx_out[i].r));
        printf("%d\t", normalised);
      }
      printf("\n");

      // printf("%dHz:\tSIN: %.2f\t\t|FFT| mag: %.1f \t nrm: %.2lf \t \033[31m<------\033[0m\n", frequency, fabs(sineWaveVector[i]), fabsf(cx_out[i].r), normalised);
    }
  }

  kiss_fft_free(cfg);
  return 0;
}


/* HELPERS BEGIN */

int normaliseSingleFrequencyMagnitude(double realValue, double maxMag) {
  return (int)(fabs(realValue / maxMag) * 100); /// return as a percentage
}

double* getNormalisedSignalVector(double maxMag, kiss_fft_cpx* signal) 
{
  double* normalisedSignalToReturn = malloc(VECTOR_LENGTH * sizeof(double)); /// Remember to free me if you use me

  for (int i = 0; i < VECTOR_LENGTH; i++) {
    double normalisedValue = fabs(signal[i].r / maxMag);
    normalisedSignalToReturn[i] = normalisedValue * 100; 
  }
  
  return normalisedSignalToReturn;
}


double getHighestMagnitude(kiss_fft_cpx* vector) 
{
  int highestVal = 0;
  for (int i = 0; i < VECTOR_LENGTH; i++) {
    if (vector[i].r > highestVal) {
      highestVal = vector[i].r;
    }
  }
  return highestVal;
}



double getFrequencySample(int index) 
{
  int harmonicSeriesLength = sizeof(HARMONIC_SERIES) / sizeof(HARMONIC_SERIES[0]);
  double valueBuilder;
  
  /// Currently not using; successively adding to valueBuilder in this way produces hamming.
  valueBuilder = (AMPLITUDE * sin(2.0 * M_PI * FUNDAMENTAL * index / SAMPLE_RATE ));
  
  for (int i = 0; i < harmonicSeriesLength; i++) {
    valueBuilder += ((AMPLITUDE / 2) * sin(2.0 * M_PI * HARMONIC_SERIES[i] * i / SAMPLE_RATE ));
  } 

  return valueBuilder;
}

