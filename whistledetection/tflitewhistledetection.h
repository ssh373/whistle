#pragma once

#include <fftw3.h>
#include <fftw_allocator.h>
#include <tfliteexecuter.h>

#include <mutex>

class TFLiteWhistleDetection {
public:
    TFLiteWhistleDetection();

    constexpr static uint32_t req_rate = 44100;
    constexpr static uint32_t req_frame_size = 441;
    constexpr static uint32_t num_channels = 1;

    constexpr static size_t frame_size = 384;
    constexpr static int ffts_per_patch = 12;
    constexpr static int patch_height = 64;

    constexpr static int expected_number_of_elements = ffts_per_patch * patch_height;

    constexpr static float prob_threshold = 0.998600;  // 0.998600;1604;15;0.9907;0.8981

    bool process(const std::vector<int16_t>& vals);

    void reset();

    bool detectionRunInProcess;

private:
    constexpr static size_t fft_spec_size = frame_size / 2 + 1;

    int c_zero_current_patch_fft = 0;
    htwk::TFLiteExecuter classifier_zero;

    int c_half_start_offset = ffts_per_patch / 2;
    int c_half_countdown_to_start = c_half_start_offset;
    int c_half_current_patch_fft = 0;
    htwk::TFLiteExecuter classifier_half;  // Starts at ffts_per_patch // 2

    float* input_c_zero;
    float* input_c_half;

    fftwf_vector<float> fft_real_buffer;
    fftwf_vector<fftwf_complex> fft_complex_buffer;
    fftwf_plan fft_plan;

    std::vector<int16_t> buffer;
    std::mutex mtx;
};
