# Qwens Tts Buddy

Qwens Tts Buddy is a native Windows desktop app for running Qwen3-TTS locally, offline, and with a studio-style workflow instead of a browser interface. It is built for creators, developers, and voice hobbyists who want to type text, choose a saved voice, shape the delivery, and export finished audio without sending anything to the cloud.

The app centers on three clear choices: a Voice Profile, a Voice Emotion, and optional Audio Processing. Voice Profiles manage built-in Qwen speakers, cloned voices, designed voices, and fine-tuned checkpoint voices. Voice Emotion controls acting style, such as natural, calm, angry, scared, whispering, excited, sad, urgent, funny, creepy, or unhinged. Audio Processing is separate and only changes the rendered sound, with options like clean narration, radio, telephone, echo chamber, reverb, deepening, brightening, robotic tone, chorus/flanger, low-pass, and high-pass filtering.

Qwens Tts Buddy includes playback controls, safe file saving, automatic versioned filenames, and export support for WAV, MP3, OGG, and FLAC. Long text can be split into chunks and joined smoothly with crossfade, while speed and generation variation remain separate so users understand what each setting does. The app stores a local voice library and generation history in SQLite, making it easy to reuse voices, inspect recent outputs, open output folders, or create processed versions of existing generations.

The model manager scans the local Qwen models folder and clearly shows which models are installed, missing, or partially present, plus what each model is used for. It supports stronger instruction models for emotion control, VoiceDesign models for character-style prompting, and Base models for cloning and fine-tuning.

For voice cloning and fine-tuning, the app provides guided validation instead of confusing Python crashes. Fine-tune projects can import WAV, MP3, OGG, and FLAC, then normalize non-WAV files into project WAVs before training. Everything is designed to stay offline-first, practical, organized, and shareable as a clean GitHub C++ package for local Qwen TTS work. Because it is a native Win32 application, it launches quickly, avoids web wrappers, and fits neatly beside an existing embedded Python/Qwen setup, making it a lightweight companion for everyday speech generation and voice experimentation at home.

Current version: `v1.0.0`

See [CHANGELOG.md](CHANGELOG.md) for the GitHub update list.

## Build

Open `QwensTtsBuddy.sln` in Visual Studio and build `Release | x64`, or run `BUILD_RELEASE.bat`.

## Run

The app defaults to the local setup created earlier:

`C:\Users\Owner\Desktop\qwen tts`

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

`C:\Users\Owner\Desktop\qwen tts\QwensTtsBuddy`

The script does not remove models, embedded Python, generated audio, or other Qwen files.

## Included Controls

- Studio-style native UI with dark/light theme toggle
- Generate, Voice Library, Voice Clone, Fine Tune, Models, and Settings pages
- SQLite-backed local voice library at `C:\Users\Owner\Desktop\qwen tts\qwen_studio_data\qwen_studio.sqlite`
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
