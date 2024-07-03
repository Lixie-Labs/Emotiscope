

# A Huge Emotiscope Update Is In Active Development

2.0.0, "The Multiplayer Update" is underway, and features:

- A re-written Emotiscope web-app, now based on the open-source Godot game engine.
- Improved networking, allowing multiple simulataneous remote controls at once (Multiplayer!)
- Harmonic Mode
- Rhythm Breeze Mode
- Wave Mode
- Improved tempo recognition
- And million little things

## Controlling Emotiscope Using A Game Engine

Using a game engine might sound strange, but since Godot lets me export to HTML/JS via WASM, I can use Godot's built-in tools for visual UI development and its websockets interface to communicate with an Emotiscope just like before. This means no longer hosting the web-app off of Emotiscope itself, which allows me to grow the web-app to any size required to provide any functionality I can dream of. After the first load to your phone, it stays cached! This should also improve compatibility with non-Chromium browsers, since the Godot devs are always incentivized to make sure their game engine works consistently across browsers and devices.

## The Emotiscope App Is Getting Its Own Repository Soon

In exchange for no longer including the web-app inside of Emotiscope for safe keeping, I'll do everything in my power to ensure that the web-app stays archived and open sourced online, making it simple to host and find offline copies for yourself years from now. 

## Farewell, Arduino.

After over ten years of learning it to it's fullest potential, I've officially left the Arduino Framework behind, opting to use the raw ESP-IDF instead. Most of Emotiscope was written with lower-level IDF functions anyways, but now it's 100%. Not only has the FastLED Arduino library left the building, but now PsychicHTTP too!

## Harmonic Mode

Similar to Spectrum Mode, Harmonic Mode instead has a larger range of frequencies visible, showing the many overtones of different musical instruments at once.

## Rhythm Breeze Mode

(These modes are tough to describe without video demos yet.) Imagine: your Emotiscope pulses on time with the beat, but a psychedelic trailing effect is gently smearing the image back and forth in a swinging pattern.

## Wave Mode

Wave Mode uses auto-correlation to show periodicity in the audio signal in a pleasant pattern. This means Wave Mode will hardly react to atonal sounds like cymbals, but strongly react to vocals and melodies.

## Improved Tempo Recognition

Tempo is now derived from a *second, separate* Fourier Transform which is also always running now, and reacts not to the standard music range but from 1Hz to 12,800Hz. This better captures sibilent sounds like drum cymbals, which are a great source for tempo estimation. (This version of the Fourier Transform is the one you see in Harmonic Mode.)

The range of tempi detection has also increased 100% to a new range of 60 BPM to 188 BPM!

## Try It Early At Your Own Risk

Familiar with ESP-IDF? Don't mind a still-very-broken look at the future? Don't need the app to work in conjunction with Emotiscope yet so you have to re-compile to change any setting not changeable through touch controls? Don't mind it messing up your Emotiscope's filesystem so you have to manually re-flash it back to 1.1.0? You're in luck! 

The 2.0 branch of this repository has the newest Emotiscope Engine with these improvements included, minus the app.
