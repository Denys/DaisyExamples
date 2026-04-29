**EURORACK EFFECTS UNIT<br>**
_Tested on 5-29-2024_

**Current draw:**
  | Rail  | Current |
  | ----- | ------- |
  | +12V  | 118mA   |
  | -12V  | 3ma     |
  | 5V    | N/A     |
<br>

**Torture Tests:**
- 20Vpp 1kHz sine applied to input
- 20Vpp 1kHz sine applied to output
<br>

**I/O Tests (using 1Khz sine wave):**<br>
- Column 2: 10k feedback resistor
- Colume 3: 4.7k feedback resistor

| Input (Vpp) | Output (Vpp)  | Output (Vpp)
| ----------- | --------------| --------------|
| 1.08        |  1.09         | 528mVpp       |
| 1.5         |  1.68         | 796mVpp       |
| 2.0         |  2.18         | 1.04Vpp       |
| 2.5         |  2.18         | 1.28          |
| 3.0         |  3.32         | 1.58          |
| 3.5         |  3.50*        | 1.86          |
| 4.0         |  3.50  "      | 2.06          |
| 10.0        |  3.54  "      | 3.5"          |
| 15.0        |  3.56  "      | 3.54          |
| 20.0        |               | 3.56          |

* Signals >=3.5V are clipped but can be attenuated by a potentiometer prior to processing.
