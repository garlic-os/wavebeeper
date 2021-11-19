#include "Beeper.cpp"

/*
BOOKMARK
for bit in file:
  if bit == 1:
    beeper.beep(LOWEST_NOTE)
  else:
    beeper.stop()
  beeper.delay(1 SAMPLE_RATEth of a second) - however long it takes to do the above

And you were finding out how a WAV file is formatted so you can iterate over
each sample, put a threshold on it, and then output a 1 or a 0 to a file.

It it ends up being fast enough, maybe skip the file and beep directly?

Also make a program that reads this kind of converted file and outputs a square
wave file in WAV form.
*/



int main() {
  Beeper beeper;

  beeper.activate();

  // Middle C
  beeper.beep(261.63);
  Sleep(500);

  // Frequency sweep
  // int increment = 1;
  // for (int i = 0; i < 10000; i += increment) {
  //   beeper.beep(i);
  //   if (i > 2000) {
  //     increment = 10;
  //   }
  //   Sleep(1);
  // }
  beeper.deactivate();

  return 0;
}
