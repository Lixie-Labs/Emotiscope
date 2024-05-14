
# 1.1 | The Color Update

**This is the first update your Emotiscope has ever had!** 

While I've done extensive testing to make sure everything runs smoothly, please be sure to follow the next section carefully and report any issues to me with your method of choice!

----

## HOW TO UPDATE YOUR EMOTISCOPE:

**To update your Emotiscope to the latest version:**

- Open the Top Menu in the app
- Tap "Check Firmware Update"
- A dialog should appear telling you that a new version is available.
- *Once the update has begun, **DO NOT** turn off Emotiscope!*
- After 1-2 minutes, your Emotiscope will reboot on its own.
- You may need to refresh the app, but only do so if the update has 100% completed.

----

## NEW FEATURES:

#### New Toggle: Reverse Color Range

You can now reverse the direction that the Color Range slider operates in! For example, if your Color Slider is set to red, pulling up on the the Color Range slider will stretch the color toward blue instead of green if Reverse Color Range is enabled.

#### New Toggle: Auto Color Cycle

Turning on Auto Color Cycle will let Emotiscope take over control of Color, automatically cycling it during moments of high musical intensity.

#### Toggles are now tappable

You don't need to pull up or down any more, but you still can if you'd like.

#### Better Color Reproduction

Emotiscope has a new temporal dithering algorithm based on error diffusion and RNG, which basically means it can now show deeper colors with less artifacts! Try turning that Background Slider *allllll* the way down now.

#### Magic Touch

After updating, try this:

1. Turn off your Emotiscope by holding your fingers on the top.
2. Reach for it again, but this time *stop short just before touching it.*

***Neat, huh?***

(As a side note, Emotiscope now automatically calibrates its touch sensors to whatever environment it's placed in, meaning the "Calibrate Touch" entry has been removed from the Top Menu.)

#### Automatic Background Noise Cancellation

If Emotiscope hears a note for more than about 10 seconds without the note changing, it grows a small "blind spot" where that note is, which goes away with about 10 more seconds without exposure to it.

Most notes in music don't last 10 seconds, that's an eternity! This means that constant hums like traffic outside, A/C units, or microwaves get automatically filtered away even while music is playing, no calibration needed.

***It's not just you, that microphone calibration icon is gone with the wind.*** (In 1.2, "The Presets Update", a wand icon will be taking its place!)

#### Instant Mirror Mode

You can now touch both sides of Emotiscope at once to toggle Mirror Mode without the app!

#### Landscape Mode

Yup, that!

#### Two Types of Light Modes

Light Modes have been split into two categories:

- **ACTIVE** - modes that react to music
- **INACTIVE** - modes that *do not* react to music

Currently, only "Neutral" mode is available as an Inactive light mode, but future updates will expand upon the possibilities for non-musical, decorative light modes. Emotiscope is open-source, and I'm open to including user-submitted modes in future updates!

#### Improved Tempo Range

Emotiscope now responds to 96 different tempi, up from 64 in 1.0! Slower and faster songs now perform better in tempo-sensitive light modes.

#### On-Board UI was made smaller

Now, when the UI is shown on the LEDs, it only takes up 25% of screen vs. 50% in 1.0.

#### Metronome Mode has been made smoother

Metronome Mode now uses a normal sine function instead of a clipped sine. ***English: it's a much smoother animation!***

(It's also been made wider when not in Mirror Mode!)

#### Hype Mode has been improved

Hype Mode has been improved by splitting odd and even tempi into two groups, and giving each group their own instance of Hype Mode.

Splitting them up helps to improve reactivity in scenarios where two neighboring tempi bins were detected 180 degrees out of phase, which led to destructive interference.

TLDR; Sometimes there's one dot, some times there's now two!

#### Now Compatible with Firefox and Safari

Thanks to @jasoncoon for first reporting it, Emotiscope 1.1 and the related web app infrastructure have been patched to allow non-Chromium browsers to join in! Please let me know if there's still any issues, I don't own any real iOS devices to test with, only online emulators.

#### Added PC Support

It works in regular PC browsers now! It's not my favorite looking thing, but it's there in an early form for those who want it!

----

## Smaller Improvements

These are more technical in nature, things like UI improvements and bug fixes.

- The "Blue Filter" slider was renamed to "Warmth", but its functionality is unchanged
- Screensaver no longer plays when Inactive Modes are used
- Gamma correction is now the *last* step of the visual pipeline, everything is Linear RGB until just before quantization
- Fixed HSV desaturation algorithm
- Added floating point lookup tables for common operations ( float fade = float(i) / NUM_LEDS )
- Cleaned up console.log() calls in web-app
- Faster substring calculation for websockets commands
- Decreased time that Emotiscope shows the sleep mode animation
- Needle UI closes itself automatically after inactivity if no close command arrives
- Reduced web-app cache length from one year to 15 minutes (oops!)
- Fixed UI blurring on Webkit browsers
- Added "Start Self Test" to the Top Menu. This shows an RGBW test pattern on the LEDs 
- The self test can also be triggered by pressing the BOOT button on Emotiscope's PCB
- "Light-show Modes" are now just called "Light Modes"
- VU floor (Analog Mode) is automatically calibrated similarly to the GDFT to remove background noise