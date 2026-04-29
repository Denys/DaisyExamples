
/*
 * FMVoice.h
 * Brent Edstrom, 2020
 * 
 */

#include <Audio.h>
#include "FMOperator.h"
 class FMVoice
 {
    
    enum{OpA = 0, OpB, OpC, OpD};
    enum{numOperators = 4};

    //Arrays of pointers for oscillators and mixers
    AudioSynthWaveformModulated *pOperator[numOperators];
    AudioMixer4 *pOpMixer[numOperators];
    AudioMixer4 *pVoiceMixer;
    AudioEffectEnvelope *pEnvelope;

    public:
    bool note_on;
    int lastNoteOn;  
    
    //Array of operators
    FMOperator operators[numOperators];
    int fundamentalFrequency;

    FMVoice(): note_on(false), lastNoteOn(0)
    {
      
    }
    
    void setFundamentalFrequency(int freq)
    {
        fundamentalFrequency = freq;
        updateTuning();
    }

    void stop()
    {
        for(int i = 0; i < numOperators; i++)
        {
            pOperator[i]->amplitude(0.0);
        }
    }
    
    void attachSines(AudioSynthWaveformModulated *pA,
                     AudioSynthWaveformModulated *pB,
                     AudioSynthWaveformModulated *pC,
                     AudioSynthWaveformModulated *pD)
    {
        pOperator[0] = pA;
        pOperator[1] = pB;
        pOperator[2] = pC;
        pOperator[3] = pD;
    }

    void attachMixers(AudioMixer4 *pmA,
                      AudioMixer4 *pmB,
                      AudioMixer4 *pmC,
                      AudioMixer4 *pmD)
    {
        pOpMixer[0] = pmA;
        pOpMixer[1] = pmB;
        pOpMixer[2] = pmC;
        pOpMixer[3] = pmD;
    }

    void attachVoiceMixer(AudioMixer4 *voiceMixer)
    {
        pVoiceMixer = voiceMixer;
    }

    void attachEnvelope(AudioEffectEnvelope *pEnv)
    {
        pEnvelope = pEnv;
    }

    void updateOperatorOutputLevel()
    {
       for(int i = 0; i < numOperators; i++)
       {
          //Update sine objects with values from FMOperator
          pOperator[i]->amplitude(operators[i].outputLevel);
       }
    }

    void updateTuning()
    {
      for(int i = 0; i < numOperators; i++)
       {
          //Update sine objects with tuning values
          pOperator[i]->frequency((operators[i].coarseTuning + 
                                operators[i].fineTuning) * fundamentalFrequency);
       }
    }

    void updateFeedbackLevel()
    {
      for(int i = 0; i < numOperators; i++)
       {
          pOpMixer[i]->gain(i, operators[i].feedback);
       }
    }

    void turnOffInputs()
    {
        for(int i = 0; i < numOperators; i++)
        {
            for(int chan = 0; chan < 4; chan++)
            {
                pOpMixer[i]->gain(chan, 0.0);
            }
        }

        pVoiceMixer->gain(OpA, 0.0);
        pVoiceMixer->gain(OpB, 0.0);
        pVoiceMixer->gain(OpC, 0.0);
        pVoiceMixer->gain(OpD, 0.0);
        
    }

    void setADSR(int a, int d, float s, int r)
    {
        pEnvelope->attack(a);
        pEnvelope->decay(d);
        pEnvelope->sustain(s);
        pEnvelope->release(r);
    }

    void setAttack(int a)
    {
        pEnvelope->attack(a);
    }

    void setDecay(int d)
    {
        pEnvelope->decay(d);
    }

    void setSustain(float s)
    {
        pEnvelope->sustain(s);
    }

    void setRelease(int r)
    {
        pEnvelope->release(r);
    }

    void setHold(int h)
    {
        pEnvelope->hold(h);
    }

    void midiNoteOn(int note, int velocity)
    {
        //Convert MIDI note to frequency
        float freq = 440.0* pow (2.0, (note-69) / 12.0);
        lastNoteOn = note;

        setFundamentalFrequency(freq);
        //updateOperatorOutputLevel();

        for(int i = 0; i < numOperators; i++)
        {
            pOperator[i]->begin(WAVEFORM_SINE);
            pOperator[i]->amplitude(operators[i].outputLevel);
        }

        //Convert midi velocity to amplitude from 0-1.0
        float a = ((float)velocity / 127.0);

        //Update the envelope
        pEnvelope->sustain(a);
        pEnvelope->noteOn();
        note_on = true;
      }

     void midiNoteOff(int note)
     {
        if(note == lastNoteOn && note_on == true)
        {
            pEnvelope->noteOff();
            note_on = false;
        }
     }
      
    void updateAlgorithm(int alg)
    {
        float level = 0.25;
        
        turnOffInputs();
        updateFeedbackLevel();
        
        if(alg == 0)
        {
            pOpMixer[OpA]->gain(OpB, level);
            pOpMixer[OpB]->gain(OpC, level);
            pOpMixer[OpC]->gain(OpD, level);

            pVoiceMixer->gain(OpA, 1.0);
        }

        if(alg == 1)
        {
            pOpMixer[OpA]->gain(OpB, level);
            pOpMixer[OpB]->gain(OpC, level);
            pOpMixer[OpB]->gain(OpD, level);

            pVoiceMixer->gain(OpA, 1.0);
        }

        if(alg == 2)
        {
            pOpMixer[OpA]->gain(OpB, level);
            pOpMixer[OpA]->gain(OpD, level);
            pOpMixer[OpB]->gain(OpC, level);

            pVoiceMixer->gain(OpA, 1.0);
        }

        if(alg == 3)
        {
            pOpMixer[OpA]->gain(OpB, level);
            pOpMixer[OpA]->gain(OpC, level);
            pOpMixer[OpB]->gain(OpD, level);
            pOpMixer[OpC]->gain(OpD, level);

            pVoiceMixer->gain(OpA, 1.0);
        }

        if(alg == 4)
        {
            pOpMixer[OpA]->gain(OpC, level);
            pOpMixer[OpB]->gain(OpC, level);
            pOpMixer[OpC]->gain(OpD, level);

            pVoiceMixer->gain(OpA, 0.5);
            pVoiceMixer->gain(OpB, 0.5);
        }

        if(alg == 5)
        {
            pOpMixer[OpB]->gain(OpC, level);
            pOpMixer[OpC]->gain(OpD, level);

            pVoiceMixer->gain(OpA, 0.5);
            pVoiceMixer->gain(OpB, 0.5);
        }

        if(alg == 6)
        {
            pOpMixer[OpA]->gain(OpB, level);
            pOpMixer[OpA]->gain(OpC, level);
            pOpMixer[OpA]->gain(OpD, level);

           pVoiceMixer->gain(OpA, 1.0);
        }

        if(alg == 7)
        {
            pOpMixer[OpA]->gain(OpB, level);
            pOpMixer[OpC]->gain(OpD, level);

            pVoiceMixer->gain(OpA, 0.5);
            pVoiceMixer->gain(OpC, 0.5);
        }

        if(alg == 8)
        {
            pOpMixer[OpA]->gain(OpD, level);
            pOpMixer[OpB]->gain(OpD, level);
            pOpMixer[OpC]->gain(OpD, level);

            pVoiceMixer->gain(OpA, level);
            pVoiceMixer->gain(OpB, level);
            pVoiceMixer->gain(OpC, level);
           
        }

        if(alg == 9)
        {
            pOpMixer[OpC]->gain(OpD, level);

            pVoiceMixer->gain(OpA, level);
            pVoiceMixer->gain(OpB, level);
            pVoiceMixer->gain(OpC, level);
        }

        if(alg == 10)
        {
           pVoiceMixer->gain(OpA, level);
           pVoiceMixer->gain(OpB, level);
           pVoiceMixer->gain(OpC, level);
           pVoiceMixer->gain(OpD, level);
        }
    }
    

 };
