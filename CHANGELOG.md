# Changelog

## v1.0.0 - 2026-06-20

Newest GitHub package build for Qwens Tts Buddy.

### Added

- Matched the public GitHub working release version as `v1.0.0`.
- Added visible app version tracking inside the Windows title bar.
- Added the app version to the left sidebar and Settings page.
- Added a root `VERSION` file for quick GitHub/package checks.
- Added this GitHub changelog/update list.

### Improved

- Kept the Generate page text and instruction boxes wrapped for readable pasted scripts.
- Kept generation cancel support: Stop becomes Cancel while Qwen is generating and unlocks regeneration afterward.
- Kept editable Generate page Mode and Language controls.
- Kept the profile-first Generate flow with separate Voice Emotion and Audio Processing controls.
- Updated the README build instructions to point to `QwensTtsBuddy.sln`.

### Previous v12 Stability Fixes Included

- Native app renamed to Qwens Tts Buddy.
- Profile-first generation with hidden manual speaker selector.
- Safe output handling with versioned files when output audio is locked.
- Automatic audio processing during generation.
- Voice Library history action for creating processed versions.
- Model manager status view for installed, missing, and partial models.
- Fine Tune imports WAV, MP3, OGG, and FLAC and converts non-WAV inputs before training data is prepared.
- Playback volume warning is limited to one clear note when the Windows playback driver does not support per-file volume control.
