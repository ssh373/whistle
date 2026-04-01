#include "tflitewhistledetection.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

TFLiteWhistleDetection::TFLiteWhistleDetection() : fft_real_buffer(frame_size), fft_complex_buffer(fft_spec_size) {
    classifier_zero.loadModelFromFile(htwk::TFLiteExecuter::getTFliteModelPath() + "/whistle-classifier.tflite",
                                      {1, ffts_per_patch, patch_height, 1});
    classifier_half.loadModelFromFile(htwk::TFLiteExecuter::getTFliteModelPath() + "/whistle-classifier.tflite",
                                      {1, ffts_per_patch, patch_height, 1});

    input_c_zero = classifier_zero.getInputTensor();
    input_c_half = classifier_half.getInputTensor();
    input_c_zero_elements = classifier_zero.getElementsInputTensor();
    input_c_half_elements = classifier_half.getElementsInputTensor();

    if (input_c_zero == nullptr || input_c_half == nullptr) {
        fprintf(stderr, "Input tensor pointer is null.\n");
        fflush(stderr);
        std::exit(1);
    }
    if (input_c_zero_elements < expected_number_of_elements || input_c_half_elements < expected_number_of_elements) {
        fprintf(stderr, "Input tensor size mismatch. Need >= %d, got zero=%zu half=%zu\n",
                expected_number_of_elements, input_c_zero_elements, input_c_half_elements);
        fflush(stderr);
        std::exit(1);
    }

    fft_plan = fftwf_plan_dft_r2c_1d(frame_size, fft_real_buffer.data(), fft_complex_buffer.data(), FFTW_ESTIMATE);
    if (fft_plan == nullptr) {
        fprintf(stderr, "Failed to create FFTW plan.\n");
        fflush(stderr);
        std::exit(1);
    }
}

bool TFLiteWhistleDetection::process(const std::vector<int16_t>& vals) {

    std::lock_guard lck(mtx);
    buffer.insert(buffer.end(), vals.begin(), vals.end());

    detectionRunInProcess = false;
    bool c_zero_result = false;
    bool c_half_result = false;

    while (buffer.size() >= frame_size) {
        const size_t c_zero_offset = c_zero_current_patch_fft * patch_height;
        const size_t c_half_offset = c_half_current_patch_fft * patch_height;

        if ((c_zero_offset + patch_height) > input_c_zero_elements || (c_half_offset + patch_height) > input_c_half_elements) {
            fprintf(stderr, "Patch offset out of bounds. zero=%zu half=%zu\n", c_zero_offset, c_half_offset);
            fflush(stderr);
            return false;
        }

        std::copy(buffer.begin(), buffer.begin() + frame_size, fft_real_buffer.begin());
        fftwf_execute(fft_plan);

        for (int i = 0; i < patch_height; i++) {
            fftwf_complex& c = fft_complex_buffer[i];
            float tmp = std::sqrt(c[0] * c[0] + c[1] * c[1]);
            tmp = std::log(tmp) / 15.f;

            if (std::isnan(tmp) || std::isinf(tmp))
                tmp = 0.f;

            input_c_zero[c_zero_offset + i] = tmp;
            input_c_half[c_half_offset + i] = tmp;
        }

        // remove the first frame_size elements.
        std::vector<int16_t>(buffer.begin() + frame_size, buffer.end()).swap(buffer);

        c_zero_current_patch_fft++;
        if (c_zero_current_patch_fft == ffts_per_patch) {
            detectionRunInProcess = true;
            c_zero_current_patch_fft = 0;
            classifier_zero.execute();
            c_zero_result = c_zero_result || (*classifier_zero.getOutputTensor()) > prob_threshold;
        }

        c_half_countdown_to_start--;
        if (c_half_countdown_to_start <= 0) {
            c_half_countdown_to_start = 0;

            c_half_current_patch_fft++;
            if (c_half_current_patch_fft == ffts_per_patch) {
                detectionRunInProcess = true;
                c_half_current_patch_fft = 0;
                classifier_half.execute();
                c_half_result = c_half_result || (*classifier_half.getOutputTensor()) > prob_threshold;
            }
        }
    }

    return c_zero_result || c_half_result;
}

void TFLiteWhistleDetection::reset() {
    std::lock_guard lck(mtx);

    c_zero_current_patch_fft = 0;
    c_half_current_patch_fft = 0;
    c_half_countdown_to_start = c_half_start_offset;
}
