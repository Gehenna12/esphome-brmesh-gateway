# BRMesh / Fastcon protocol notes

What this component sends, with the parts that were added for native effects.
All values were confirmed by capturing the **plaintext** payloads of the
BRMesh Android app (`adb logcat`) and by testing on real lights. The
encryption, addressing and advertising layer come from the original
esphome-fastcon project and are not repeated here.

All payloads below are the 12-byte plaintext *before* encryption. They
contain no mesh key.

## Byte 0

Byte 0 is not an opcode but two nibbles:

```text
byte0 = frame_type | (number_of_meaningful_bytes_after_byte0 << 4)
```

The length only counts meaningful bytes, not the zero padding up to 12 bytes.

| frame_type | Meaning |
|---|---|
| `2` | single light: on/off, brightness, color |
| `8` | single light: effect |

## Single light: on/off, brightness, color (frame type 2)

```text
72 [light_id] [state_brightness] [blue] [red] [green] [warm] [cold] 00 00 00 00
```

- `state_brightness`: bit 7 = on, bits 0-6 = brightness 0-127.
- **Channel order is blue, red, green** – not RGB.
- `22 [light_id] 00 ...` turns the light off.

## Single light: effect (frame type 8)

```text
[8 | (n << 4)] [light_id] [speed] [first_color | type_flag] [more colors...] [00 padding]

n = 3 + number of additional color bytes
```

There are no effect IDs. Every effect – including the 21 built-in effects of
the app – is just a **type** plus a **list of color codes**:

| Type | Flag in the first color byte |
|---|---|
| fade | `0x40` |
| flash | `0xC0` |

Single-color effects carry an explicit `00` as second color byte (for flash
this means "color on, then off").

Examples (light 1, slowest speed):

```text
880101420301050406000000   Full Color Fade  [2, 3, 1, 5, 4, 6]
480101420400000000000000   Christmas        [2, 4]
4801C8C20000000000000000   Red Flash        [2]
```

### Color codes

| Code | Color |
|---:|---|
| 1 | dark blue |
| 2 | red |
| 3 | pink / purple |
| 4 | green |
| 5 | light blue |
| 6 | yellow |
| 7 | white (RGB white, slightly purple) |
| 8 | warm white |
| 9 | light pink |
| 10 | light yellow |
| 11 | stronger pink |
| 12 | stronger light yellow |
| 13 | cold white |
| 14 | strong yellow |
| 15 | cold white (weaker than 13) |

Codes 1-7 are the ones offered by the app. Codes 8-15 cannot be selected in
the app; they were found by experiment and only tested as single-color fades.
There is no orange, but colors are interpolated, so a fade red → yellow
passes through orange.

Up to 9 colors fit into the frame; up to 7 are confirmed.

### Speed

The UI speed is 1 (slowest) to 100 (fastest).

| Type | Encoding | Samples (UI → byte) |
|---|---|---|
| fade | `speed` | 1 → `01`, 50 → `32` |
| flash | `202 - 2 × speed` | 1 → `C8`, 50 → `66`, 97 → `08` |

### Behaviour on the lamp

- The effect runs on the lamp itself; the gateway sends a single command.
- An effect command turns a lamp on, even if it was off.
- A static color command stops a running effect. There is no separate
  "stop effect" command. Selecting effect "None" in Home Assistant therefore
  sends the current color.
- The lamps send no state back. The state shown in Home Assistant is what
  the gateway last sent.

## Built-in app effects

| Name | Type | Colors |
|---|---|---|
| Full Color Fade | fade | 2, 3, 1, 5, 4, 6 |
| Fresh | fade | 5, 1, 4 |
| Christmas | fade | 2, 4 |
| Winter | fade | 1, 7 |
| Halloween | fade | 4, 7 |
| Valentines Day | fade | 2, 7 |
| Romantic | fade | 3, 2 |
| Cozy | fade | 6, 2 |
| Full Color Flash | flash | 2, 3, 1, 5, 4, 6 |
| Three Color Flash | flash | 2, 4, 1 |
| Yellow and Green Flash | flash | 6, 4 |
| Purple and Green Flash | flash | 3, 4 |
| Purple and Blue Flash | flash | 3, 1 |
| Yellow and Purple Flash | flash | 6, 3 |
| Red and Green Flash | flash | 2, 4 |
| Red and Blue Flash | flash | 2, 1 |
| Green and Blue Flash | flash | 4, 1 |
| Red Flash | flash | 2 |
| Green Flash | flash | 4 |
| Blue Flash | flash | 1 |
| White Flash | flash | 7 |
