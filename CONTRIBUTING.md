# Contributing to AetherSDR

Thanks for your interest in AetherSDR! We're building a Linux-native SmartSDR
client for FlexRadio, and community contributions are welcome. This document
explains how to get involved and what we expect from contributors.

## How to Contribute

### Reporting Bugs

- Open a GitHub issue with a clear title and description.
- Include your Linux distro, Qt version, radio model, and firmware version.
- Attach relevant logs, screenshots, or Wireshark captures if possible.
- Check existing issues first to avoid duplicates.

### Suggesting Features

- Open a GitHub issue tagged `enhancement`.
- Describe the SmartSDR feature you'd like replicated, or the new capability
  you're proposing.
- Reference the SmartSDR UI or FlexLib behavior where applicable — screenshots
  of the Windows client are very helpful.

### Submitting Code

1. **Fork the repo** and create a feature branch from `main`.
2. **Keep commits small and focused** — one logical change per commit.
3. **Follow the coding conventions** outlined below.
4. **Test your changes** against a real FlexRadio if possible, or describe
   how you tested without hardware.
5. **Open a pull request** against `main` with a clear description of what
   changed and why.

## Rules

### Branch Protection

- **No direct pushes to `main`.** All changes go through pull requests.
- PRs require at least one approving review before merge.
- All PRs must pass CI checks before merging.
- Use descriptive branch names: `feature/band-stacking`, `fix/waterfall-crash`,
  `refactor/meter-model`.

### Code Quality

- Follow the C++20 / Qt6 conventions documented in
  [CLAUDE.md](CLAUDE.md) — that file is the authoritative style guide.
- **RAII everywhere.** No naked `new`/`delete`. Use smart pointers or Qt
  parent-child ownership.
- **No `using namespace std;`** in headers. Qualify types explicitly.
- Keep classes small and single-responsibility.
- Use `QSignalBlocker` to prevent feedback loops when syncing UI from model
  state — never rely on flag hacks.
- Prefer Qt signals/slots over raw callbacks or lambdas for cross-object
  communication.
- No compiler warnings. Build with `-Wall -Wextra` and fix what it finds.

### Commit Messages

- Use imperative mood: "Add band stacking" not "Added band stacking".
- First line under 72 characters. Add a blank line then a longer description
  if needed.
- Reference GitHub issues where applicable: `Fixes #42`.

### What We Will Not Accept

- **Wine/Crossover workarounds.** The goal is a fully native Linux client.
- **Vendored dependencies.** Use system packages or CMake's `find_package`.
- **Code without clear provenance.** Do not copy-paste code from SmartSDR,
  FlexLib, or any proprietary source. Clean-room implementations based on
  observed protocol behavior and public documentation are fine.
- **Changes that break slice 0 RX flow.** The core receive path must always
  work. If your change touches `RadioModel`, `SliceModel`, `PanadapterStream`,
  or `AudioEngine`, test thoroughly.
- **Large reformatting PRs.** Style-only changes create merge conflicts and
  obscure real work. Fix style in files you're already modifying.

### Protocol Work

- All protocol commands must be documented with the firmware version they
  target (currently v1.4.0.0).
- If you discover undocumented protocol behavior, include a comment explaining
  how you verified it (Wireshark capture, SmartSDR observation, etc.).
- When in doubt about protocol behavior, open an issue and ask — don't guess.

## Areas Where Help Is Needed

Check the [README](README.md) "What's NOT Yet Implemented" section for the
full list. High-impact areas include:

- **Multi-slice support** — slice tabs, overlaid markers, independent audio
- **Band stacking / band map** — store and recall frequency+mode per band
- **CW keyer** — built-in keyer with memories and contest macros
- **DAX / CAT interface** — integration with logging and contest software
- **Spot / DX cluster** — telnet or web-based cluster overlay on the spectrum
- **Network audio (Opus)** — compressed audio for remote operation
- **TNF management** — tracking notch filter UI
- **Testing infrastructure** — unit tests for protocol parsing and model logic

If you're looking for a good first contribution, issues tagged `good first issue`
are a great starting point.

## Development Setup

### Dependencies (Arch Linux)

```bash
sudo pacman -S qt6-base qt6-multimedia cmake ninja pkgconf
```

### Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j$(nproc)
./build/AetherSDR
```

### Hardware

A FlexRadio FLEX-6000 or FLEX-8600 series radio running firmware v1.4.0.0 is
the reference target. If you don't have hardware, you can still contribute to
UI, protocol parsing, and model logic — just note in your PR that you tested
without a radio.

## Code of Conduct

Be respectful, constructive, and patient. Ham radio has a long tradition of
helping each other learn — bring that spirit here. We're all building
something together.

## License

By contributing to AetherSDR, you agree that your contributions will be
licensed under the [GNU General Public License v3.0](LICENSE).
