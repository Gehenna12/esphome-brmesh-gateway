# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.0] - 2026-10-22

First public release. Based on
[scross01/esphome-fastcon](https://github.com/scross01/esphome-fastcon) at
commit `824e977` (branch `dev`).

### Added

- Native BRMesh effects: new effect frame (frame type 8) sent by
  `FastconController::effect_control()`; the effect runs on the lamp itself.
- `color_list` effect for the `fastcon` light platform: an effect is a type
  (`fade`/`flash`) plus 1–9 color codes (1–15), defined in YAML.
- Effect speed: `FastconController::set_effect_speed()` for a shared template
  number, and the per-light option `default_effect_speed` (1–100). Fade speed
  is encoded directly, flash speed inversely.
- Re-send of an unchanged effect on ESPHome 2026.10.0+: since
  esphome/esphome#19916 ESPHome no longer calls `write_state()` when a light
  receives the effect that is already running; the component now re-sends it.
- Example configuration with all 21 built-in effects of the BRMesh app.
- Protocol notes for effects, color codes and speed (`docs/PROTOCOL.md`).
- CI workflow that validates the example configuration.

### Changed

- `mesh_key` is marked as sensitive and redacted in `esphome config` output.
- Requires ESPHome 2026.10.0 or newer.

### Fixed

- Misleading-indentation compiler warning in `to8()` (upstream commit
  `67c2e51`).

[Unreleased]: https://github.com/Gehenna12/esphome-brmesh-gateway/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/Gehenna12/esphome-brmesh-gateway/releases/tag/v1.0.0
