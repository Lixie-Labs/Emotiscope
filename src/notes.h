// ----------------------------------------------
//                  _                      _     
//                 | |                    | |    
//  _ __     ___   | |_    ___   ___      | |__  
// | '_ \   / _ \  | __|  / _ \ / __|     | '_ \ 
// | | | | | (_) | | |_  |  __/ \__ \  _  | | | |
// |_| |_|  \___/   \__|  \___| |___/ (_) |_| |_|
//                                               
// Notes for development purposes, shared with you because I love you

// ##################################################################################################################################################
// HOW TO GET JTAG WORKING ON PLATFORMIO WITH ESP32S3 DEV BOARD WITH TWO USB PORTS:
// (Doesn't seem to work on alpha 3.0.0 IDF I'm using right now)
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

// ##################################################################################################################################################
// POTENTIAL SSL workaround: YEAH NEVERMIND
/*

app.emotiscope.rocks is an SSL page and thus qualifies as a PWA

if a url parameter like "wait=true" is present, it will not automatically
hop to the discovered HTTP server on Emotiscope, but show it in a list of devices instead

This way, the user has time to see and click the "Install PWA" button

Once installed, it will no longer wait before redirecting unless no devices are found

UPDATE: Nope, PWAs still visibly complain when the connection gets downgraded to HTTP. Good security, shitty UX.
*/

// ##################################################################################################################################################
// Sub-frame audio

// If the CPU can get an audio frame at 200 FPS but the GPU can render at 500 FPS,
// it could be possible to render "extra" audio frames that make temporal sense.
//
// If we're 25% of the way between when the last audio frame was recieved and when the next one will arrive,
// a float value of 0.25 could be used to index into the previous audio chunk to break the 64 samples into 
// an even smaller (but temporally consistent) chunk.