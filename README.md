# wavebeeper
Play audio files through your beep speaker

![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)

---

## Usage
Convert and play a WAV file directly
```
./wav2sqr < audio.wav | ./play
```
Or save the converted audio so you can play it multiple times or do something else with it
```
./wav2sqr < audio.wav > audio.sqr
./play < audio.sqr
```
## Explanation
`wav2sqr` makes what I call Square files, with the `.sqr` extension. This is named after the fact that they encode a square wave for the beep speaker to play, a straight `1` or `0` per sample in the original audio file based on whether the sample is a part of a peak or a trough. `wav2sqr` essentially quantizes WAV files[^1] to 1-bit, to match the fact that a beep speaker can only be fully on or fully off with no volume range in between.

[^1]: Currently only 8-bit PCM WAV files. Support for the far more common 16-bit PCM and 32-bit float formats in the works.

`play` is designed to take in a SQR file and play back the square wave it represents. When it encounters a 1, it moves the beep speaker's diaphragm out, and when it encounters a 0, it moves the beep speaker's diaphragm in. Doing this at the original audio file's sample rate results in the beep speaker oscillating in time with the square wave just like a conventional speaker's diaphragm oscillates in time with regular sound data.

`sqr2wav` converts an SQR file back into an (unsigned 8-bit mono) WAV file with the same sample rate as the WAV file it came from.

## Square File Format (SQR)
| Offset | Length (bytes) | Contents |
| ------ | -------------- | -------- |
| `0x00` | 4              | The string "SQR\x00" |
| `0x04` | 4              | Sample rate (number of samples per second); unsigned 32-bit integer |
| `0x08` | 4              | Data size (in bytes); unsigned 32-bit integer |
| `0x0C` | [0x08]         | Audio data; each byte contains 8 samples, with the first in the most significant bit and last in the least significant bit |
