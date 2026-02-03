#include <alsa/asoundlib.h>
#include <soundcapturing.h>
#include <tflitewhistledetection.h>

#define ALSA_PCM_NEW_HW_PARAMS_API

void SoundCapturing::process(std::string device_name) {
    int rc;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;

    /* Open PCM device for recording (capture). */
    rc = snd_pcm_open(&handle, device_name.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        return;
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    rc = snd_pcm_hw_params_any(handle, params);
    if (rc < 0) {
        fprintf(stderr, "snd_pcm_hw_params_any: %s\n", snd_strerror(rc));
        return;
    }

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        fprintf(stderr, "snd_pcm_hw_params_set_access: %s\n", snd_strerror(rc));
        return;
    }

    /* Signed 16-bit little-endian format */
    rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    if (rc < 0) {
        fprintf(stderr, "snd_pcm_hw_params_set_format: %s\n", snd_strerror(rc));
        return;
    }

    if(snd_pcm_hw_params_set_channels(handle, params, TFLiteWhistleDetection::num_channels)) {
        printf("Couldn't set number of channels to %d\n", TFLiteWhistleDetection::num_channels);
        exit(-1);
    }

    unsigned int req_rate = TFLiteWhistleDetection::req_rate;
    unsigned long req_frame_size = TFLiteWhistleDetection::req_frame_size;
    uint32_t num_channels = TFLiteWhistleDetection::num_channels;

    rc = snd_pcm_hw_params_set_rate_near(handle, params, &req_rate, &dir);
    if (rc < 0) {
        fprintf(stderr, "snd_pcm_hw_params_set_rate_near failed: %s\n", snd_strerror(rc));
        return;
    }

    printf("Requested rate %d, got %d.\n", TFLiteWhistleDetection::req_rate, req_rate);
    rc = snd_pcm_hw_params_set_period_size_near(handle, params, &req_frame_size, &dir);
    if (rc < 0) {
        fprintf(stderr, "snd_pcm_hw_params_set_period_size_near failed: %s\n", snd_strerror(rc));
        return;
    }
    printf("Requested period %d, got %ld.\n", TFLiteWhistleDetection::req_frame_size, req_frame_size);
    // TFLiteWhistleDetection whistle_detection_left;
    // TFLiteWhistleDetection whistle_detection_right;
    TFLiteWhistleDetection whistle_detection;


    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        return;
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_uframes_t frames;
    rc = snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    if(rc) {
        fprintf(stderr, "snd_pcm_hw_params_get_period_size failed: %s\n", snd_strerror(rc));
        return;
    }

    std::vector<short> buffer(req_frame_size * num_channels);

    /* We want to loop for 5 seconds */
    rc = snd_pcm_hw_params_get_period_time(params, &val, &dir);
    if(rc) {
        fprintf(stderr, "snd_pcm_hw_params_get_period_time failed: %s\n", snd_strerror(rc));
        return;
    }

    while (true) {
        rc = snd_pcm_readi(handle, buffer.data(), req_frame_size);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != (int)req_frame_size) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }

        if (whistle_detection.process(buffer)) {
            printf("Whistle detected! This can trigger multiple time.\n");
            fflush(stdout);
        }
    }
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
}
