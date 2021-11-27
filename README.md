# wavebeeper (WIP)
Play audio files through your beep speaker
---

## Usage
Convert and play a WAV file directly
```
convert.exe < audio.wav | play.exe
```
Or save the converted audio so you can play it multiple times or do something else with it
```
convert.exe < audio.wav > audio.sqr
play.exe < audio.sqr
```
## Explanation
`wav2sqr.exe` makes what I call Square files, with the `.sqr` extension. This is named after the fact that they encode a square wave for the beep speaker to play, a straight `1` or `0` per sample in the original audio file based on whether the sample is a part of a peak or a trough. `wav2sqr.exe` essentially quantizes WAV files to 1-bit, to match the fact that a beep speaker can only be fully on or fully off with no volume range in between.

`play.exe` is designed to take in a SQR file and play back the square wave it represents. When it encounters a 1, it excites the beep speaker by telling the operating system to play a pitch through it.[^1] When `play.exe` encounters a 0, it relaxes the beep speaker.[^2] Doing this at the original audio file's sample rate results in the beep speaker oscillating in time with the square wave just like a conventional speaker's diaphragm oscillates in time with regular sound data.

[^1]: It plays the lowest possible pitch so the speaker doesn't have a chance to go back to rest before the next sample. You can't actually set the beeper's diaphragm state directly—[that's abstracted away by dedicated hardware on your motherboard.](https://web.archive.org/web/20161030204856/https://courses.engr.illinois.edu/ece390/books/labmanual/io-devices-speaker.html)
[^2]: By turning it off.

`sqr2wav.exe` converts an SQR file back into an (unsigned 8-bit mono) WAV file with the same sample rate as the WAV file it came from.

## Square File Format (SQR)
| Offset | Length (bytes) | Contents |
| ------ | -------------- | -------- |
| `0x00` | 4              | The string "SQR\x00" |
| `0x04` | 4              | Sample rate (number of samples per second); unsigned 32-bit integer |
| `0x08` | 4              | Data size (in bytes); unsigned 32-bit integer |
| `0x0C` | [0x08]         | Audio data; each byte contains 8 samples, with the first in the most significant bit and last in the least significant bit |
