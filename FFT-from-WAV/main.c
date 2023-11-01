  /*  WHAT THIS PROGRAM DOES
      1. Open a WAV file passed in over the command line
      2. Extract audio data + information from it e.g. sample rate
      3. Decimate that audio data and load that into a buffer
      4. Run an FFT on that buffer.
      5. Print out the real number magnitude of each frequency bucket.
  */

#define DR_WAV_IMPLEMENTATION // see ./lib/dr-wav/dr_wav.h for info 
#include "dr_wav.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../external-libs/kissfft-master/kiss_fft.h"

int main(int argc, char* argv[]) {

  /// 1. Open a WAV file passed in over the command line
  drwav wav;
  
  if (argc != 2) { printf("USAGE ./test audio-recordings/file.wav"); exit(1); }
  if (!drwav_init_file(&wav, argv[1], NULL)) { printf("Error opening and reading WAV file."); exit(1); }


  /// 2. Extract audio data from WAV
  unsigned int channels   = wav.channels;
  unsigned int sampleRate = wav.sampleRate;
  drwav_int32* interleavedPCMFrameVectorFromWAV = malloc(wav.totalPCMFrameCount * wav.channels * sizeof(drwav_int32));
  size_t numberPCMSamplesReadFromWAV = drwav_read_pcm_frames_s32(&wav, wav.totalPCMFrameCount, interleavedPCMFrameVectorFromWAV);


  /// 3. Decimate that audio data and load that into a buffer
  size_t decimatedSamplesCount = numberPCMSamplesReadFromWAV * 0.1; 
  size_t halfDecimatedSamplesCount = decimatedSamplesCount * 0.5; 

  double decimatedSamplesVector[decimatedSamplesCount];
  int step = 10;
  for (int i = 0; i < decimatedSamplesCount; i++) {
    decimatedSamplesVector[i] = interleavedPCMFrameVectorFromWAV[ i + step ];
  }

  double leftChannel[halfDecimatedSamplesCount];
  double rightChannel[halfDecimatedSamplesCount];

  int leftIndex = 0;
  int rightIndex = 0;
  for (int bufferIndex = 0; bufferIndex <= decimatedSamplesCount; bufferIndex++){
    if (bufferIndex % 2 == 0){
      leftChannel[leftIndex] = decimatedSamplesVector[bufferIndex]; 
      leftIndex++;
    } else {
      rightChannel[rightIndex] = decimatedSamplesVector[bufferIndex];
      rightIndex++;
    }
  }
  
  printf("l: %d, r: %d \n\n", leftIndex, rightIndex);
  for (int i = 0; i < 1000; i++) {
    printf("%.1f\t", rightChannel[i]);
  }
  exit(0);

  
  /// 4. Run an FFT on that buffer.
  kiss_fft_cfg cfg = kiss_fft_alloc(decimatedSamplesCount, 0, NULL, NULL);  

  // allocate on heap to avoid SO (literally and also the website)
  kiss_fft_cpx* cx_in_leftChannel   = malloc(halfDecimatedSamplesCount * sizeof(kiss_fft_cpx)); 
  kiss_fft_cpx* cx_out_leftChannel  = malloc(halfDecimatedSamplesCount * sizeof(kiss_fft_cpx));
  memset(cx_in_leftChannel, 0, halfDecimatedSamplesCount * sizeof(kiss_fft_cpx)); // set every array bucket to 0
  memset(cx_out_leftChannel, 0, halfDecimatedSamplesCount * sizeof(kiss_fft_cpx));

  kiss_fft_cpx* cx_in_rightChannel  = malloc(halfDecimatedSamplesCount * sizeof(kiss_fft_cpx));
  kiss_fft_cpx* cx_out_rightChannel = malloc(halfDecimatedSamplesCount * sizeof(kiss_fft_cpx));
  memset(cx_in_rightChannel,  0, halfDecimatedSamplesCount * sizeof(kiss_fft_cpx));
  memset(cx_out_rightChannel, 0, halfDecimatedSamplesCount * sizeof(kiss_fft_cpx));

  for (int i = 0; i < halfDecimatedSamplesCount; i++){
    cx_in_leftChannel[i].r  = leftChannel[i]; /// Real
    cx_in_leftChannel[i].i  = leftChannel[i]; /// imaginary -- we don't care about complex plane; here following API convention.
    cx_in_rightChannel[i].r = rightChannel[i];
    cx_in_rightChannel[i].i = rightChannel[i];
  }

  kiss_fft(cfg, cx_in_leftChannel, cx_out_leftChannel); 
  kiss_fft(cfg, cx_in_rightChannel, cx_out_rightChannel);

  // for (int i = 0; i < halfDecimatedSamplesCount; i++) {
  //   printf("%.1f\t", fabsf(cx_out_rightChannel[i].r));
  // }
  // exit(0);


  /// 5. Print out the real number magnitude of each frequency bucket.

  printf("sample rate: %d\n", sampleRate);
  printf("length of the WAV audio vector: %zu\n", numberPCMSamplesReadFromWAV);
  printf("length sampled of the vector: %zu\n", decimatedSamplesCount);

  for (int i = 0; i < 1000; i++) {
    // if (fabsf(cx_out_leftChannel[i].r) >= 1) {
    //   /// TODO: ask paul RE: this
    //   int frequency = i * sampleRate / halfDecimatedSamplesCount; 
    //   printf("%dHz:\t fft-mag: \033[31m%.1f\033[0m   \t SIN: %.2f \n", frequency, fabsf(cx_out_leftChannel[i].r), fabs(leftChannel[i]));
    // }
    // if (fabsf(cx_out_rightChannel[i].r) >= 1) {
      /// TODO: fix NaN error
      int frequency = i * sampleRate / halfDecimatedSamplesCount; 
      printf("%dHz:\t fft-mag: \033[31m%.1f\033[0m   \t SIN: %.2f \n", frequency, fabsf(cx_out_rightChannel[i].r), fabs(rightChannel[i]));
    // }
    if (i > sampleRate / 5) {
      break;
    }
  }

  kiss_fft_free(cfg);
  drwav_uninit(&wav);
  return 0;
}

