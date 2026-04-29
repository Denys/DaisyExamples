/*
 * FMOperator class
 * Brent Edstrom, 2020
 */

#include <Audio.h>

 struct FMOperator
 {
    public: 
    int coarseTuning; //Octave
    float fineTuning; //0.0-1.0
    float outputLevel;
    float feedback;
    int outputChannel;

    FMOperator()
    {
      coarseTuning = 1;
      fineTuning = 0.0;
      outputLevel = 0.25;
      feedback = 0.0;
      outputChannel = 0;
    }  

 };
