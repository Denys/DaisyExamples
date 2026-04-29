**MIDI TO CONTROL VOLTAGE CONVERTER<br>**
_Tested on 5/29/2024_

**Current draw:**
  | Rail  | Current |
  | ----- | ------- |
  | +12V  | 30mA    |
  | -12V  | 13ma    |
  | 5V    | N/A     |
<br>

**Torture Tests:<br>**
20Vpp 1Hz sine applied to the following for 60 seconds:  
- Gate A 
- Gate B
- Pitch 1
- Pitch 2
<br>

**Gate Output:<br>**
| Gate  | Off    | On     |
| ----- | ------ | ------ |
| A     | 2.5mV  | 5.12V  |
| B     | -2.0mV | 5.12V  |
<br>

**Voct Output:<br>**
| Octave  | Voct A    | Voct B     |
| ------- | --------- | ---------- |
| C0      | 2.1mV     | 9.7mV      |
| C1      | 0.98V     | 1.01V      |
| C2      | 1.97V     | 2.00V      |
| C3      | 2.96V     | 3.00V      |
| C4      | 3.95V     | 4.00V      |
| C5      | 4.94V     | 4.99V      |
| C6      | 5.93V     | 5.99V      |
| C7      | 6.91V     | 6.99V      |
| C8      | 7.90V     | 7.99V      |
| C9      | 8.88V     | 8.99V      |
| C10     | 9.73V     | 9.73V      |

