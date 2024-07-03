

# A Huge Emotiscope Update Is In Active Development

2.0.0, "The Multiplayer Update" is underway, and features:

- A re-written Emotiscope web-app, now based on the open-source Godot game engine.
- Improved networking, allowing multiple simulataneous remote controls at once (Multiplayer!)
- Harmonic Mode
- Rhythm Breeze Mode,
- Wave Mode
- Improved tempo recognition
- And million little things

## Controlling Emotiscope Using A Game Engine

Using a game engine might sound strange, but since Godot lets me export to HTML/JS via WASM, I can use Godot's built-in tools for visual UI development and its websockets interface to communicate with an Emotiscope just like before. This means no longer hosting the web-app off of Emotiscope itself, which allows me to grow the web-app to any size required to provide any functionality I can dream of. After the first load to your phone, it stays cached! This should also improve compatibility with non-Chromium browsers, since the Godot devs are always incentivized to make sure their engine works consistently across browsers.

## The Emotiscope App Is Getting Its Own Repository Soon

In exchange for no longer including the web-app inside of Emotiscope for safe keeping, I'll do everything in my power to ensure that the web-app stays archived and open sourced online, making it simple to host offline copies for yourself years from now. 

## Farewell, Arduino.

After over ten years of learning it to it's fullest potential, I've officially left the Arduino Framework behind, opting to use the raw ESP-IDF instead. Most of Emotiscope was written with lower-level IDF functions anyways, but now it's 100%. Not only has the FastLED Arduino library left the building, but now PsychicHTTP too!

