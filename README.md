# ESPHome BRMesh Gateway

[![Validate](https://github.com/Gehenna12/esphome-brmesh-gateway/actions/workflows/validate.yml/badge.svg)](https://github.com/Gehenna12/esphome-brmesh-gateway/actions/workflows/validate.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

*Deutsche Kurzfassung: [README.de.md](README.de.md)*

Control **BRMesh / Fastcon BLE lights** (lamps set up with the *BRMesh* app)
from **Home Assistant** with an ESP32 running **ESPHome** – including the
lamps' **native color effects**.

The gateway sends the same Bluetooth LE advertisements as the app. No cloud,
no phone, no MQTT broker: the lights show up in Home Assistant through the
regular ESPHome integration.

This project is based on [esphome-fastcon](https://github.com/scross01/esphome-fastcon)
and extends it with native effects (see [Credits](#credits)).

## Features

- On/off, brightness, RGB color and white for each light
- **Native effects run on the lamp itself**: one BLE command starts a smooth
  fade or flash, with no stream of color updates from the gateway
- All **21 built-in effects of the BRMesh app**, plus your own effects as a
  simple color list in YAML, with no C++ changes needed
- **Effect speed** (1–100) as a number entity in Home Assistant
- Works with ESPHome **2026.10.0** and newer (includes a fix for the changed
  light behaviour in that release)
- Mesh key is redacted in `esphome config` output and logs

## Tested hardware

| Part | Model |
|---|---|
| Board | ESP32-S3 Mini development board – [Amazon](https://www.amazon.de/dp/B0F1CR5RHV) |
| Lights | DUSKTEC LED recessed spotlights RGB, 7 W, 230 V (BRMesh app) – [Amazon](https://www.amazon.de/dp/B09FL59QJ2) |

Tested with 10 of these spotlights on one gateway. Other BRMesh/Fastcon
lights should work as well, because the protocol is the same, but they have
not been tested. Feedback is welcome.

Other ESP32 boards **with Bluetooth LE** (ESP32, ESP32-S3, ESP32-C3, …)
should work too; only the board above has been tested. The ESP32-S2 has no
Bluetooth and does **not** work.

> **Note:** Make sure your lamps really use the **BRMesh** app. The similarly
> named *brLight* app uses a different protocol.

## Requirements

- ESPHome **2026.10.0** or newer (dashboard/Device Builder or CLI)
- Home Assistant with the ESPHome integration
- Lights already set up in the **BRMesh** app
- An **Android** phone (plus a computer with `adb`) to read the mesh key once

## Quick start

1. Set up all lights in the BRMesh app (Android) as usual.
2. [Find the mesh key](#finding-the-mesh-key) and the
   [light IDs](#finding-the-light-ids).
3. Copy [`example/brmesh-gateway.yaml`](example/brmesh-gateway.yaml) into your
   ESPHome configuration folder.
4. Copy [`secrets.example.yaml`](secrets.example.yaml) to `secrets.yaml` (or
   add the keys to your existing one) and fill in your values.
5. Adjust `substitutions:`, the `light:` entries (`light_id`, `name`) and,
   if needed, the `board`.
6. Install the firmware (the first time via USB, later over the air).
7. Home Assistant discovers the device; add it through the ESPHome
   integration.

The example loads the component directly from this repository, pinned to a
release:

```yaml
external_components:
  - source: github://Gehenna12/esphome-brmesh-gateway@v1.0.0
    components: [fastcon]
```

## Configuration

### Controller

```yaml
fastcon:
  id: fastcon_controller
  mesh_key: !secret brmesh_mesh_key   # required, 8 hex characters
  adv_interval_min: 0x20               # optional, default 0x20
  adv_interval_max: 0x20               # optional, default 0x40
  adv_duration: 150                    # optional, ms, default 50
  adv_gap: 10                          # optional, ms, default 10
  max_queue_size: 100                  # optional, default 100
```

The values in the example (`0x20`/`0x20`/`150`) worked reliably with the
tested spotlights; the defaults come from the original project.

### Lights

```yaml
light:
  - platform: fastcon
    id: living_room_spot_1
    name: "Living Room Spot 1"
    light_id: 1                     # required, 1-255
    supports_cwww: false            # optional, lamp has cold + warm white
    color_interlock: true           # optional, never RGB and white together
    default_effect_speed: 1         # optional, 1-100, see below
    default_transition_length: 0s   # recommended
    effects:
      - color_list:
          name: "Christmas"
          type: fade                # fade or flash
          colors: [2, 4]            # 1-9 color codes, 1-15
```

### Effects

Each effect is a **type** (`fade` or `flash`) plus a **list of color codes**.
The lamp runs the effect itself.

| Code | Color | Code | Color |
|---:|---|---:|---|
| 1 | dark blue | 9 | light pink |
| 2 | red | 10 | light yellow |
| 3 | pink / purple | 11 | stronger pink |
| 4 | green | 12 | stronger light yellow |
| 5 | light blue | 13 | cold white |
| 6 | yellow | 14 | strong yellow |
| 7 | white (RGB) | 15 | cold white (weaker) |
| 8 | warm white | | |

Codes 1–7 are the colors offered by the app; 8–15 are not selectable in the
app and have only been tested as single-color fades. The example contains all
21 built-in app effects. Details: [docs/PROTOCOL.md](docs/PROTOCOL.md).

### Effect speed

The example creates a number entity **Effect Speed** (1 = slowest,
100 = fastest) shared by all lights. Changing it sends nothing by itself –
it applies to the **next effect command**. To change the speed of a running
effect, set the number and then send the same effect again. This works on
ESPHome 2026.10.0+ because the component re-sends a re-selected effect.

Until the number is used for the first time, each light uses its own
`default_effect_speed` (default 1).

## Finding the mesh key

All lights set up in one BRMesh app share a 4-byte **mesh key**. Without it,
the lamps ignore all commands. The key can be read from the app's debug log
on Android (method from the original esphome-fastcon project):

1. Enable USB debugging on the phone and connect it to a computer with `adb`.
2. Run:

   ```bash
   adb logcat | { grep -m 1 -o 'jyq_helper: .* payload:.\{24\},[[:space:]]*key:[[:space:]]*.\{8\}' | awk '{print $NF}'; kill -2 $(pgrep -P $$ adb); }
   ```

3. Open the BRMesh app and switch a light on and off. The command prints the
   mesh key (8 hex characters).

Put it into `secrets.yaml` as `brmesh_mesh_key`. **Treat the key like a
password**: anyone who has it can control your lights. Never post it in
issues or logs.

## Finding the light IDs

Each lamp has an ID (1–255) assigned by the app. The IDs usually follow the
order in which the lamps were added, but gaps are common (e.g. after a lamp
was removed and added again), so check them:

```bash
adb logcat -c
adb logcat -b all -v threadtime | grep --line-buffered -i 'jyq_helper'
```

Switch a **single** light in the app. The second byte of the logged payload is
the light ID, for example:

```text
payload:220B80000000000000000000   ->  0x0B = light ID 11
```

Alternatively, add lights with IDs 1, 2, 3, … to the configuration and see
which lamp reacts.

**Adding new lamps:** pair them in the BRMesh app first (they then get the
same mesh key), find their ID as above and add a `light:` entry.

## Home Assistant

Each light appears as a normal light entity with brightness, color and an
effect list. Examples (entity IDs depend on your names):

```yaml
# Start an effect
action: light.turn_on
target:
  entity_id: light.brmesh_gateway_living_room_spot_1
data:
  effect: "Christmas"

# Faster effect: set the speed, then send the effect again
action: number.set_value
target:
  entity_id: number.brmesh_gateway_effect_speed
data:
  value: 40

# Stop the effect: select a color (or the effect "None")
action: light.turn_on
target:
  entity_id: light.brmesh_gateway_living_room_spot_1
data:
  rgb_color: [255, 120, 0]
```

## Troubleshooting

| Symptom | Things to check |
|---|---|
| A light does not react | correct `light_id`; correct mesh key; distance/walls between ESP32 and lamp; close the BRMesh app while testing |
| No light reacts | mesh key; board has Bluetooth LE (not ESP32-S2); BLE stack starts (log) |
| Commands are slow or get lost | do not use logger level `VERBOSE`; keep `default_transition_length: 0s`; avoid fast color animations from Home Assistant |
| `Error setting raw advertisement data (err=259)` once at boot | harmless: a restored light state is sent before BLE is ready. Only a problem if it also appears later |
| `esp32_ble took a long time for an operation` at boot | harmless when BLE works afterwards |
| Effect speed change has no effect | the speed applies to the next effect command – send the effect again |

To see what is sent, set `fastcon.controller` and `fastcon.light` to `DEBUG`
in the `logger:` section; the plaintext payloads are logged (see
[docs/PROTOCOL.md](docs/PROTOCOL.md)).

## Known limitations

- **Send only, no feedback:** the lamps send no state back. Home Assistant
  shows what the gateway last sent. Changes made in the BRMesh app or by a
  power cut are not visible.
- **Range:** every lamp must be within BLE range of the gateway. Do not rely
  on other lamps repeating the signal.
- **Brightness during an effect:** effect commands carry no brightness, so the
  brightness cannot be changed while an effect is running.
- Group and broadcast commands of newer upstream versions are not included
  (one command per light).
- Only the hardware listed above has been tested.

## Credits

- [dennispg/esphome-fastcon](https://github.com/dennispg/esphome-fastcon) by
  Dennis George – the original ESPHome Fastcon component, continued in
  [brett-todd/esphome-fastcon](https://github.com/brett-todd/esphome-fastcon)
  and [scross01/esphome-fastcon](https://github.com/scross01/esphome-fastcon),
  on which this repository is based.
- [Mooody](https://mooody.me/posts/2023-04/reverse-the-fastcon-ble-protocol/)
  ([archived copy](https://web.archive.org/web/20240917191822/https://mooody.me/posts/2023-04/reverse-the-fastcon-ble-protocol/)) –
  reverse engineering of the Fastcon BLE protocol.
- [zentik-fr/esphome-brmesh](https://github.com/zentik-fr/esphome-brmesh) –
  earlier description of the effect frames.
- [ArcadeMachinist/brMeshMQTT](https://github.com/ArcadeMachinist/brMeshMQTT)
  and the [Home Assistant community](https://community.home-assistant.io/t/brmesh-app-bluetooth-lights/473486).

Details and license texts: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## License

[MIT](LICENSE) – the same license as the original esphome-fastcon.

## Disclaimer

This is an independent hobby project. It is **not affiliated with, endorsed
by or connected to** the makers of the BRMesh app, Broadlink, DUSKTEC or any
other manufacturer. All product names and trademarks belong to their owners.
The Amazon links are plain links, not affiliate links.

Use at your own risk. The protocol was reverse engineered and may change with
app or firmware updates.

**Mains voltage:** the tested lights run on 230 V. Installing or wiring mains
powered lights must only be done by a qualified electrician. This project
only concerns the low-voltage ESP32 and the wireless control.
