# Third-party notices

## esphome-fastcon

The component in `components/fastcon/` is a modified version of
**esphome-fastcon**. The original was written by Dennis George and later
forked as follows:

| Repository | Role |
|---|---|
| [dennispg/esphome-fastcon](https://github.com/dennispg/esphome-fastcon) | original project |
| [brett-todd/esphome-fastcon](https://github.com/brett-todd/esphome-fastcon) | fork |
| [scross01/esphome-fastcon](https://github.com/scross01/esphome-fastcon) | fork, branch `dev` |

This repository is based on `scross01/esphome-fastcon` at commit
[`824e9777f3a8bd9abc2b1d9e2c65a4d9c0dc7ff8`](https://github.com/scross01/esphome-fastcon/commit/824e9777f3a8bd9abc2b1d9e2c65a4d9c0dc7ff8)
(branch `dev`). It also includes upstream commit
[`67c2e51`](https://github.com/scross01/esphome-fastcon/commit/67c2e51), which
fixes a misleading-indentation compiler warning.

Changes made in this repository are listed in [CHANGELOG.md](CHANGELOG.md).

License of the original work:

```
MIT License

Copyright (c) 2025 Dennis George

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Acknowledgements (no code used)

- **zentik-fr/esphome-brmesh** (MIT) – described the effect frame layout
  before it was confirmed independently for this project. No code from that
  repository is included.
- **Mooody** – [reverse engineering of the Fastcon BLE protocol](https://mooody.me/posts/2023-04/reverse-the-fastcon-ble-protocol/)
  ([archived copy](https://web.archive.org/web/20240917191822/https://mooody.me/posts/2023-04/reverse-the-fastcon-ble-protocol/)),
  the foundation of esphome-fastcon.
- **ArcadeMachinist/brMeshMQTT** – credited by the original esphome-fastcon
  project as an implementation reference.
- [Home Assistant community thread on BRMesh lights](https://community.home-assistant.io/t/brmesh-app-bluetooth-lights/473486)
