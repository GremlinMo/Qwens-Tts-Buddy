# Qwens Tts Buddy

Native Windows desktop UI for your offline Qwen3-TTS setup.

Current version: `v1.0.0`

See [CHANGELOG.md](CHANGELOG.md) for the GitHub update list.

## Build

Open `QwensTtsBuddy.sln` in Visual Studio and build `Release | x64`, or run `BUILD_RELEASE.bat`.

## Run

The app defaults to the local setup created earlier:

`C:\Users\flyin\OneDrive\Desktop\qwen tts`

It expects:

- `python312-embed\python.exe`
- `models\Qwen3-TTS-12Hz-0.6B-CustomVoice`

The app runs offline by default and writes audio to the selected output file.

## Install Into The Local Qwen Folder

Run this from PowerShell if you want the app and source staged inside the local Qwen folder:

```powershell
powershell -ExecutionPolicy Bypass -File .\INSTALL_TO_QWEN_TTS_FOLDER.ps1
```

It installs into:

`C:\Users\flyin\OneDrive\Desktop\qwen tts\QwensTtsBuddy`

The script does not remove models, embedded Python, generated audio, or other Qwen files.

## Included Controls

- Studio-style native UI with dark/light theme toggle
- Generate, Voice Library, Voice Clone, Fine Tune, Models, and Settings pages
- SQLite-backed local voice library at `C:\Users\flyin\OneDrive\Desktop\qwen tts\qwen_studio_data\qwen_studio.sqlite`
- Reusable voice profiles for built-in voices, cloned voices, VoiceDesign prompts, and fine-tuned checkpoints
- Generate page is profile-first: one selected `Saved Profile` controls speaker, language, model mode, default voice emotion, and default audio processing
- Voice sample manager with audio/transcript validation and duration/sample-rate capture
- Import/export voice profiles as local JSON
- Generation history and versions saved locally for recent outputs
- Playback controls through Windows multimedia APIs
- Stop becomes Cancel while generation is running, so a stuck or unwanted Qwen job can be stopped before regenerating
- Export formats: WAV, MP3, OGG, FLAC
- Safe output handling with temp writes and auto-versioned filenames when a file already exists or is locked
- Rendered Speed control from 0.70x to 1.50x; Temperature/Variation remains generation randomness, not speed
- Long-text generation with sentence-aware chunking and configurable crossfade
- Normalize toggle plus max chunk and crossfade controls
- Separate `Voice Emotion` and `Audio Processing` controls, with Voicebox-inspired tags such as `[laugh]`, `[chuckle]`, `[gasp]`, `[sigh]`, and `[whisper]`
- Voice emotions: Natural, Calm, Angry, Scared/paranoid, Whisper, Excited, Sad, Urgent, Funny with laugh ending, Creepy/tense, Crazy/unhinged
- Audio processing: None, Clean Narration, Radio, Telephone, Echo Chamber, Reverb Room, Deepen, Brighten, Robotic, Chorus/Flanger, Low-Pass, High-Pass
- Effects use a fast offline `numpy`/`scipy`/`soundfile` engine for pitch, gain, filters, compression, delay, reverb, chorus, wobble, tremolo, and distortion
- `Run Audio Diagnostics` checks every audio-processing preset from Settings; Voice Library history can create a separate processed version of an existing output
- Playback controls through Windows multimedia APIs; the volume slider disables itself with one clear note if the current Windows driver does not support app-level MCI volume
- Instruction presets for calm, angry, scared/paranoid, whisper, excited, sad, and urgent delivery
- Scared/paranoid and other emotional presets route to Strong Instructions 1.7B when installed; choose Ryan/Aiden profiles for stronger English acting
- Automatic routing from built-in 0.6B mode to 1.7B CustomVoice when instruction text is present and the stronger model is installed
- Model-aware generation modes:
  - Built-in voices: `Qwen3-TTS-12Hz-0.6B-CustomVoice`
  - Strong instructions: `Qwen3-TTS-12Hz-1.7B-CustomVoice`
  - Voice design: `Qwen3-TTS-12Hz-1.7B-VoiceDesign`
  - Voice clone/fine-tune: `Qwen3-TTS-12Hz-0.6B-Base`
- Model scanner and downloader shows status, purpose, and path for every key model
- Guided single-speaker fine-tuning dataset builder imports WAV, MP3, OGG, and FLAC and normalizes non-WAV files into project WAVs before writing JSONL
- Device, dtype, attention, offline mode, and sampling controls
- Validation before running voice clone or fine-tuning jobs, so missing models/files/transcripts produce clear app messages instead of Python stack traces

## Note

Qt 6 was not available on this machine, so this build uses a custom native C++ Windows UI that can be compiled immediately with the installed Visual Studio toolchain. The Python job interface is JSON-based so a Qt shell can be added later without changing the Qwen backend workflow.
