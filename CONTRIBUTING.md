# Contributing

Thanks for your interest! Bug reports, tests with other lamps or boards, and
pull requests are welcome.

## Reporting bugs

Please use the bug report template and include your ESPHome version, board,
lamp model and the relevant log lines.

**Never post your mesh key, Wi-Fi credentials or API key.** Remove them from
configs and logs before posting. The fastcon debug log shows plaintext
payloads only, which are safe to share.

## Pull requests

- Keep changes small and focused; describe what you tested and on which
  hardware.
- Make sure the example still validates:

  ```bash
  cp secrets.example.yaml example/secrets.yaml
  sed 's#^  - source: github://.*#  - source: { type: local, path: ../components }#' \
    example/brmesh-gateway.yaml > example/ci.yaml
  esphome config example/ci.yaml
  ```

- Document protocol findings in `docs/PROTOCOL.md`, and user-facing changes in
  `CHANGELOG.md` under "Unreleased".
- Changes that are useful for everybody using esphome-fastcon are best also
  proposed upstream at
  [scross01/esphome-fastcon](https://github.com/scross01/esphome-fastcon).

By contributing you agree that your contribution is licensed under the
[MIT License](LICENSE).
