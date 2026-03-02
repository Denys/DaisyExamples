/**
 * @file Pomodoro_Pod.cpp
 * @brief Pomodoro Timer for Daisy Pod
 * @author Antigravity (Enhanced by Gemini)
 *
 * Workflow:
 * - STANDBY (White): Rotate encoder to select 25m (Cyan) or 40m (Blue). Press to confirm.
 * - READY (Yellow): Press SW1 to Start. Defaults to Standby after 5m idle.
 * - WORKING (Red): SW1 to Pause (Blink Red). Timer End -> Break.
 * - BREAK (Green): 25m Work -> 5m Break. 40m Work -> 10m Break. Timer End -> Ready.
 * - RESET (Global): SW2 from any state -> Standby (White).
 *
 * Feedback:
 * - LED1: State Color
 * - LED2: Progress Indicator (during Work) / Blink Green (start of Break)
 *   - 100-80% Red
 *   - 80-60% Magenta
 *   - 60-40% Yellow
 *   - 40-20% Blue
 *   - 20-10% Cyan
 *   - 10-5% White
 *   - <5% Green
 *   - Break Start: Blink Green (1 min)
 */

#include "daisy_pod.h"

using namespace daisy;

// Hardware
DaisyPod hw;

// Constants
const uint32_t TIME_SHORT_WORK  = 25 * 60 * 1000; // 25 min
const uint32_t TIME_LONG_WORK   = 40 * 60 * 1000; // 40 min
const uint32_t TIME_SHORT_BREAK = 5 * 60 * 1000;  // 5 min
const uint32_t TIME_LONG_BREAK  = 10 * 60 * 1000; // 10 min
const uint32_t TIMEOUT_READY    = 5 * 60 * 1000;  // 5 min idle

// Simple Color Struct
struct MyColor
{
    float r, g, b;
    MyColor(float _r, float _g, float _b) : r(_r), g(_g), b(_b) {}
    MyColor() : r(0), g(0), b(0) {}
};

const MyColor C_WHITE(1.0f, 1.0f, 1.0f);
const MyColor C_CYAN(0.0f, 1.0f, 1.0f);
const MyColor C_BLUE(0.0f, 0.0f, 1.0f);
const MyColor C_YELLOW(1.0f, 1.0f, 0.0f);
const MyColor C_RED(1.0f, 0.0f, 0.0f);
const MyColor C_GREEN(0.0f, 1.0f, 0.0f);
const MyColor C_MAGENTA(1.0f, 0.0f, 1.0f);
const MyColor C_OFF(0.0f, 0.0f, 0.0f);

// Helper to set LED with MyColor
void SetLed(RgbLed& led, const MyColor& c)
{
    led.Set(c.r, c.g, c.b);
}

// Gradient Logic
MyColor GetProgressColor(float percent)
{
    if(percent > 0.8f)
        return C_RED; // 100-80%
    if(percent > 0.6f)
        return C_MAGENTA; // 80-60%
    if(percent > 0.4f)
        return C_YELLOW; // 60-40%
    if(percent > 0.2f)
        return C_BLUE; // 40-20%
    if(percent > 0.1f)
        return C_CYAN; // 20-10%
    if(percent > 0.05f)
        return C_WHITE; // 10-5%
    return C_GREEN;     // < 5%
}

// States
enum State
{
    ST_STANDBY,
    ST_READY,
    ST_WORKING,
    ST_PAUSED,
    ST_BREAK
};

State currentState = ST_STANDBY;

// Timing Variables
uint32_t stateEnteredTime;
uint32_t timerStartTime;
uint32_t pausedTime;
uint32_t accumulatedPauseTime;
uint32_t currentWorkDuration;
uint32_t currentBreakDuration;

// Selection Logic
bool isLongDurationSelected = false; // false=25m, true=40m

// Feedback
uint32_t feedbackTriggerTime = 0;
bool     feedbackActive      = false;

// Helpers
void TriggerFeedback()
{
    feedbackTriggerTime = System::GetNow();
    feedbackActive      = true;
}

void EnterState(State newState)
{
    currentState         = newState;
    stateEnteredTime     = System::GetNow();
    accumulatedPauseTime = 0;

    TriggerFeedback();
}

void ResetToStandby()
{
    EnterState(ST_STANDBY);
    isLongDurationSelected = false; // Default to short
}

int main(void)
{
    hw.Init();

    // Initial State
    ResetToStandby();

    while(1)
    {
        hw.ProcessAllControls();

        // Global Reset (SW2)
        if(hw.button2.RisingEdge())
        {
            ResetToStandby();
        }

        uint32_t now = System::GetNow();

        // --- State Machine ---
        switch(currentState)
        {
            case ST_STANDBY:
            {
                // Encoder Rotate: Toggle Selection
                if(hw.encoder.Increment() != 0)
                {
                    isLongDurationSelected = !isLongDurationSelected;
                }

                // Encoder Press: Confirm -> READY
                if(hw.encoder.RisingEdge())
                {
                    if(isLongDurationSelected)
                    {
                        currentWorkDuration  = TIME_LONG_WORK;
                        currentBreakDuration = TIME_LONG_BREAK;
                    }
                    else
                    {
                        currentWorkDuration  = TIME_SHORT_WORK;
                        currentBreakDuration = TIME_SHORT_BREAK;
                    }
                    EnterState(ST_READY);
                }

                // LED1 Logic
                if(isLongDurationSelected)
                {
                    SetLed(hw.led1, C_BLUE); // 40m
                }
                else
                {
                    SetLed(hw.led1, C_CYAN); // 25m
                }
                break;
            }

            case ST_READY:
            {
                // Timeout check
                if(now - stateEnteredTime > TIMEOUT_READY)
                {
                    ResetToStandby();
                }

                // SW1: Start -> WORKING
                if(hw.button1.RisingEdge())
                {
                    timerStartTime = now;
                    EnterState(ST_WORKING);
                }

                SetLed(hw.led1, C_YELLOW);
                break;
            }

            case ST_WORKING:
            {
                // Calculate elapsed time
                uint32_t elapsed
                    = (now - timerStartTime) - accumulatedPauseTime;

                // Timer End
                if(elapsed >= currentWorkDuration)
                {
                    EnterState(ST_BREAK);
                    timerStartTime = now; // Reuse for break timer
                }

                // SW1: Pause
                if(hw.button1.RisingEdge())
                {
                    pausedTime = now;
                    EnterState(ST_PAUSED);
                }

                SetLed(hw.led1, C_RED);

                // LED2: Progress
                float percent
                    = 1.0f - ((float)elapsed / (float)currentWorkDuration);
                if(percent < 0.0f)
                    percent = 0.0f;
                SetLed(hw.led2, GetProgressColor(percent));
                break;
            }

            case ST_PAUSED:
            {
                // SW1: Unpause
                if(hw.button1.RisingEdge())
                {
                    accumulatedPauseTime += (now - pausedTime);
                    currentState = ST_WORKING; // Direct transition back
                    TriggerFeedback();
                }

                // Visual: Blink Red
                if((now / 500) % 2 == 0)
                {
                    SetLed(hw.led1, C_RED);
                }
                else
                {
                    SetLed(hw.led1, C_OFF);
                }

                SetLed(hw.led2, C_OFF);
                break;
            }

            case ST_BREAK:
            {
                // Calculate elapsed time (reusing timerStartTime)
                uint32_t elapsed = now - timerStartTime;

                // Timer End -> READY
                if(elapsed >= currentBreakDuration)
                {
                    EnterState(ST_READY);
                }

                SetLed(hw.led1, C_GREEN);

                // LED2: Blink Green for first 1 minute
                if(elapsed < 60000)
                {
                    if((now / 500) % 2 == 0)
                    {
                        SetLed(hw.led2, C_GREEN);
                    }
                    else
                    {
                        SetLed(hw.led2, C_OFF);
                    }
                }
                else
                {
                    SetLed(hw.led2, C_OFF);
                }
                break;
            }
        }

        // --- LED2 Feedback Override (Short Click Blinks) ---
        // Disable click feedback during Working/Break so it doesn't interrupt progress bar
        if(currentState != ST_WORKING && currentState != ST_BREAK)
        {
            if(feedbackActive)
            {
                if(now - feedbackTriggerTime < 100)
                {
                    SetLed(hw.led2, C_WHITE);
                }
                else
                {
                    feedbackActive = false;
                    SetLed(hw.led2, C_OFF);
                }
            }
            else
            {
                SetLed(hw.led2, C_OFF);
            }
        }

        hw.UpdateLeds();
    }
}
