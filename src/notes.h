// ----------------------------------------------
//                  _                      _     
//                 | |                    | |    
//  _ __     ___   | |_    ___   ___      | |__  
// | '_ \   / _ \  | __|  / _ \ / __|     | '_ \ 
// | | | | | (_) | | |_  |  __/ \__ \  _  | | | |
// |_| |_|  \___/   \__|  \___| |___/ (_) |_| |_|
//                                               
// Notes for development purposes, shared with you because I love you

// TODO: Remove FastLED Dependency

// ##################################################################################################################################################
// HOW TO GET JTAG WORKING ON PLATFORMIO WITH ESP32S3 DEV BOARD WITH TWO USB PORTS:
//
// https://community.platformio.org/t/how-to-use-jtag-built-in-debugger-of-the-esp32-s3-in-platformio/36042
// +
// https://community.platformio.org/t/cannot-run-builtin-debugger-on-esp32-s3-board/36384/9

// ##################################################################################################################################################
// The ASCII art generator for the headers https://patorjk.com/software/taag/#p=display&f=Big

// ##################################################################################################################################################
// ESP-DSP lets you sqrt() an entire array at once, but I'm not sure if there's a SIMD "ae32" version, just an ANSI one
/*
esp_err_t dsps_sqrt_f32_ansi(const float *input, float *output, int len)
{
    if (NULL == input) return ESP_ERR_DSP_PARAM_OUTOFRANGE;
    if (NULL == output) return ESP_ERR_DSP_PARAM_OUTOFRANGE;

    for (int i = 0 ; i < len ; i++) {
        output[i] = dsps_sqrtf_f32_ansi(input[i]);
    }
    return ESP_OK;
}
*/