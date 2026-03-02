# Pomodoro Pod

A Pomodoro timer for the Daisy Pod with a dedicated visual workflow.

## Controls

| Hardware | Action | Function |
|----------|--------|----------|
| **Encoder** | Rotate | Select Duration (25m / 40m) in Standby |
| **Encoder** | Press | **Approve selection** (Go to Ready) |
| **Button 1** | Press | **Start** (from Ready) / **Pause** / **Resume** |
| **Button 2** | Press | **Reset** (from any state) to Standby |

## State Indicators (LED 1)

| State | Color | Description |
|-------|-------|-------------|
| **STANDBY** | **White** | Waiting for selection. |
| **SELECT** | **Cyan** | 25 min Work + 5 min Break selected. |
| **SELECT** | **Blue** | 40 min Work + 10 min Break selected. |
| **READY** | **Yellow** | Seletion confirmed. Waiting for Start. |
| **WORKING** | **Red** | Work timer active. |
| **PAUSED** | **Blink Red** | Timer paused. |
| **BREAK** | **Green** | Break timer active. |

**LED 2** indicates remaining work time (Progress):
- **100-80%**: Red
- **80-60%**: Magenta
- **60-40%**: Yellow
- **40-20%**: Blue
- **20-10%**: Cyan
- **10-5%**: White
- **< 5%**: Green
- **Break Start**: Blinks Green (1 min)

## Compiling & Flashing

```bash
# Compile
make

# Flash to Daisy Pod
make program-dfu
```
