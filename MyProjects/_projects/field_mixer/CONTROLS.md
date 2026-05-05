# field_mixer Controls

Hydra uses physical IN L. Edge uses physical IN R. The firmware creates one
common mix and writes the same protected signal to `OUT L`, `OUT R`, and the
headphone output. Headphone output carries the same common mix because the
Field headphone jack follows the same codec output pair as the line outputs.

## Main Bank

Use `K1-K8` with no switch held.

| Control | Function | Notes |
|---|---|---|
| K1 | Hydra level | Source level after trim/tone |
| K2 | Edge level | Source level after trim/tone and ducking |
| K3 | Hydra tone | Left = warmer, center = neutral, right = brighter |
| K4 | Edge tone | Left = warmer, center = neutral, right = brighter |
| K5 | Hydra delay send | Amount sent from Hydra to the mono delay |
| K6 | Edge delay send | Amount sent from Edge to the mono delay |
| K7 | Delay return | Delay level into the main mix |
| K8 | Master level | Final level before output shaping |

## Alt Bank

Hold `SW1` while moving `K1-K8`.

| Control | Function | Notes |
|---|---|---|
| K1 | Hydra input trim | Range is conservative for line-level sources |
| K2 | Edge input trim | Range is conservative for line-level sources |
| K3 | Hydra high-pass | Removes low-end buildup before the mix |
| K4 | Edge high-pass | Removes low-end buildup before the mix |
| K5 | Delay time | Approx. 60-850 ms |
| K6 | Delay feedback | Clamped below runaway feedback |
| K7 | Delay tone | Dark to bright delay return |
| K8 | Ducking amount | Edge ducks under Hydra when ducking is enabled |

## Utility Layer

Hold `SW2` while moving `K1-K8`.

| Control | Function | Notes |
|---|---|---|
| K1 | Hydra mute fade time | Smooths Hydra mute/solo changes |
| K2 | Edge mute fade time | Smooths Edge mute/solo changes |
| K3 | Delay throw amount | Added to source sends while throw is active |
| K4 | Delay freeze feedback | Used while freeze is active |
| K5 | Limiter threshold | Safety threshold before the ceiling |
| K6 | Output saturation | Drive amount when saturation is enabled |
| K7 | Input gate threshold | Suppresses no-input hum from floating line inputs; turn down if quiet tails are cut |
| K8 | Output safety ceiling | Final output ceiling |

## A/B Keys

| Key | Function | LED behavior |
|---|---|---|
| A1 | Hydra mute | On while muted |
| A2 | Edge mute | On while muted |
| A3 | Delay mute | On while muted |
| A4 | Master mute | On while muted |
| A5 | Hydra solo | Blinks while soloed |
| A6 | Edge solo | Blinks while soloed |
| A7 | Bypass all modifiers, keep mixer active | Blinks when any modifier is active |
| A8 | Panic / reset performance toggles | Always blinks as the safety key |
| B1 | Delay freeze | On while active |
| B2 | Delay throw | On while active |
| B3 | Edge ducking toggle | On while active |
| B4 | Soft saturation toggle | On while active |
| B5 | Scene 1: clean mix | One-hot scene LED |
| B6 | Scene 2: Hydra lead | One-hot scene LED |
| B7 | Scene 3: Edge lead | One-hot scene LED |
| B8 | Scene 4: delay performance | One-hot scene LED |

## Startup Defaults

| Parameter | Default |
|---|---|
| Hydra level | 75% |
| Edge level | 70% |
| Hydra tone | Neutral |
| Edge tone | Slightly warm |
| Hydra delay send | 10% |
| Edge delay send | 15% |
| Delay return | 20% |
| Delay feedback | 25% |
| Hydra high-pass | Approx. 133 Hz |
| Edge high-pass | Approx. 158 Hz |
| Input gate threshold | 45% |
| Master level | 80% |
| Ducking | Off |
| Saturation | Off |
| Limiter | On |

## Scenes

| Scene | Behavior |
|---|---|
| B5 Clean mix | Startup-style balanced mix with conservative delay |
| B6 Hydra lead | Hydra louder, Edge lower, modest Hydra delay send |
| B7 Edge lead | Edge louder, Hydra lower, modest Edge delay send |
| B8 Delay performance | Higher delay sends, return, and feedback |

## Manual Hardware Validation Checklist

Record date, build commit or diff state, and tester name before marking this
project hardware-validated.

- `make` succeeds from `MyProjects/_projects/field_mixer`.
- QAE validation succeeds with `py -3 ../../../DAISY_QAE/validate_daisy_code.py .`.
- Hydra connected to `IN L` changes level with `K1`, tone with `K3`, and delay send with `K5`.
- Edge connected to `IN R` changes level with `K2`, tone with `K4`, and delay send with `K6`.
- `K8` controls master level without zipper noise or unsafe jumps.
- `SW1` exposes the alt bank and uses pickup/catch before changing values.
- `SW2` exposes the utility layer and uses pickup/catch before changing values.
- `A1-A6` mute/solo behavior is audible and LED states match the table.
- `A8` clears mutes, solos, delay freeze, throw, ducking, and saturation.
- With no Hydra or Edge connected, the input gate and higher startup high-pass
  should suppress no-input hum from floating line inputs.
- `B1` delay freeze holds delay content without runaway output.
- `B2` delay throw increases the delay send amount.
- `B3` makes Hydra reduce Edge level when Hydra is present.
- `B4` enables controlled soft saturation without harsh clipping.
- `B5-B8` recall the documented scenes and update LEDs.
- OLED shows the active bank, source status, meters, and edit zoom after parameter changes.
- `OUT L`, `OUT R`, and headphones carry the same protected common mix.
