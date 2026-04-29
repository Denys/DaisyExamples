/* SynthVoice.h
 * Brent Edstrom, 2020
 */


 struct SynthVoice
 {
      AudioSynthWaveform *pWaveform;
      AudioEffectEnvelope *pEnvelope;
      byte last_note;
      long timestamp;

      //Default constructor
      SynthVoice()
      {
          pWaveform = NULL;
          pEnvelope = NULL;
          last_note = timestamp = 0;
      }
      
      void attachAudioObjects(AudioSynthWaveform *pWave, AudioEffectEnvelope *pEnv)
      {
          pWaveform = pWave;
          pEnvelope = pEnv;
      }

      bool isActive()
      {
          return pEnvelope->isActive();
      }
      
      void noteOn(byte note, byte velocity) 
      {
          if(velocity > 0)
          {
             //Convert MIDI note to frequency
             float freq = 440.0* pow (2.0, (note-69) / 12.0);

             //Convert MIDI velocity (0-127) to amplitude (0-1)
             float amp = (float) velocity / 127.0;

             //Output tone
             pWaveform->begin(amp, freq, WAVEFORM_SAWTOOTH);

             pEnvelope->noteOn();
             last_note = note;
             timestamp = millis();
          }else{
             noteOff(note);
          }
          
      }
      
      void noteOff(byte note) 
      {
          if(note == last_note && pEnvelope->isActive())
          {
            pEnvelope->noteOff();
            timestamp = 0;
            last_note = 0;
          }
      }

      
 };
 
