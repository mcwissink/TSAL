#include "Mixer.h"
#include "Oscillator.h"
#include "MidiParser.h"
#include "MidiNotes.h"
#include <omp.h>
#include <vector>

// Temporary fix
#include <chrono>
#include <thread>

void thread_sleep(unsigned milliseconds) {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
// Temporary fix

int main(int argc, char* argv[]) {
  tsal::Mixer mixer;
  std::vector<tsal::Oscillator> voices(3);  

  tsal::MidiNote chord[] = {tsal::C4, tsal::E4, tsal::G4};
  for (unsigned i = 0; i < voices.size(); i++) {
    voices[i].setGain(0.1);
    voices[i].setNote(chord[i]);
    mixer.add(voices[i]);
  }

  omp_set_num_threads(3);

  #pragma omp parallel
  {
    int id = omp_get_thread_num();

    #pragma omp for
    for (unsigned i = 0; i < 100; i++) {
      voices[id].setFrequency(voices[id].getFrequency() + 10);
      thread_sleep(100);
    }

  }
  return 0;
}