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


// ###############################################################################################
// Tasks 3/6/24
/*

- Redesign feet and spacer layer to interface together to prevent foot swivel
- Resize logo on front
- Design microphone adapter PCB
- Design back plate window to QR code
- Calc final screw lengths
- Make shared cut cork file
- Make shared cut diffuser file
- Make shared cut wood file
- Design nut cavity in USB brace

*/

// ##############################################################################
// Live Instrument Classifier
/*

SETUP

- Write a Python script which can iterate over all dataset items in the "train" partition of the OpenMIC 2018 dataset
- Port Emotiscope's Goertzel engine to a standalone C++ program that takes a waveform chunk input and replicates the hardware spectral output
- Validate that outputs match between software and hardware
- For each OGG file input, downsample, mono, and cut it into as many CHUNK_SIZE waveform chunks as possible
- Re-export waveform chunk data with original instrument metadata from OpenMIC
- Same for "test" partition

TRAIN

- Use all waveform chunks + metadata to train a MLP network of size 32->32->32->32->20
- 32 input neurons are downsampled via averaging from the 64-bin spectrogram
- Three hidden layers of the same size
- This hidden network shape can be easily visualized with three color channels on LEDs
- Validate results against test set
- Test/benchmark network in other shapes/dimensions

EXPORT

- Export trained neuron weights and biases to C float arrays
- (Quantize to 16-bit?)

IMPLEMENT

- Use ESP32-S3 SIMD functions (or potentially ESP-DL if I can find at-all decent documentation on it)
- Downsample spectrogram and feed-forward on trained neural network in hardware
- Output resulting neurons in a compelling fashion like assigning colors to instrument types
- Could also filter down to just vocal presence detection too

*/