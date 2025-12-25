#include <stdio.h>
#include <stdint.h>
#include <x264.h>

int main() {
    printf("Testing x264 library...\n");
    
    // initialize x264 parameters
    x264_param_t param;
    if (x264_param_default_preset(&param, "medium", NULL) < 0) {
        printf("Error: Failed to set default preset\n");
        return -1;
    }
    
    // set custom parameters
    param.i_csp = X264_CSP_I420;
    param.i_width = 640;
    param.i_height = 480;
    param.i_fps_num = 30;
    param.i_fps_den = 1;
    
    // apply profile
    if (x264_param_apply_profile(&param, "baseline") < 0) {
        printf("Error: Failed to apply profile\n");
        return -1;
    }
    
    // open encoder
    x264_t *encoder = x264_encoder_open(&param);
    if (!encoder) {
        printf("Error: Failed to open encoder\n");
        return -1;
    }
    
    printf("x264 encoder opened successfully!\n");
    
    // get encoder info
    x264_nal_t *nals;
    int i_nals;
    x264_picture_t pic_in, pic_out;
    
    // init picture
    if (x264_picture_alloc(&pic_in, param.i_csp, param.i_width, param.i_height) < 0) {
        printf("Error: Failed to allocate picture\n");
        x264_encoder_close(encoder);
        return -1;
    }
    
    printf("Picture allocated successfully!\n");
    
    // encode one empty frame (test)
    pic_in.i_pts = 0;
    
    int frame_size = x264_encoder_encode(encoder, &nals, &i_nals, &pic_in, &pic_out);
    if (frame_size < 0) {
        printf("Error: Failed to encode frame\n");
    } else {
        printf("Frame encoded successfully! Size: %d bytes\n", frame_size);
        printf("Number of NAL units: %d\n", i_nals);
    }
    
    // clean up resources
    x264_picture_clean(&pic_in);
    x264_encoder_close(encoder);
    
    printf("x264 test completed successfully!\n");
    return 0;
}