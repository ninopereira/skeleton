"""Generate mouth_script.hpp (jaw keyframes) from the voice clips.

Each clip gets its own script. The jaw opening follows the loudness of the
voice, sampled in fixed frames and quantized to a few levels so the servo
gets clean, distinct moves.

Clip names must start with their advert number (1, 2, ... in order, e.g.
001_WhoAreYou.mp3): script i is played with /ADVERT/000i.mp3.

Requires: python3 with numpy, and ffmpeg on the PATH.
Usage:    python3 tools/make_jaw_script.py [voice_dir]   (default: sd_card)
"""

import re
import subprocess
import sys
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DIR = ROOT / "sd_card"
HEADER = ROOT / "mouth_script.hpp"
# Limits of the Keyframe fields and the 8-bit frame index.
MAX_HOLD_MS = 65535
MAX_KEYFRAMES = 255

SAMPLE_RATE = 16000
# A hobby servo needs ~60-80 ms to swing the jaw; faster frames just blur.
FRAME_MS = 60
# Voice band used to measure loudness; drops rumble and hiss/echo tails.
BAND_LOW_HZ = 300
BAND_HIGH_HZ = 3000
# Syllables are peaks above the slow loudness trend; subtracting part of
# that trend makes the jaw close between syllables in continuous speech.
FAST_WINDOW_MS = 30
SLOW_WINDOW_MS = 300
TREND_WEIGHT = 0.35
# Articulation (0..1 of peak) at which the jaw starts to open / is fully
# open.
OPEN_START = 0.06
OPEN_FULL = 0.60
# Openness levels in percent; quantizing avoids constant servo jitter.
LEVEL_STEP_PCT = 25


def load_mono(path):
    raw = subprocess.run(
        ["ffmpeg", "-v", "error", "-i", str(path), "-f", "f32le", "-ac", "1",
         "-ar", str(SAMPLE_RATE), "-"],
        capture_output=True, check=True).stdout
    return np.frombuffer(raw, dtype=np.float32)


def band_limit(samples):
    """Keep only the voice band using an FFT mask."""
    spectrum = np.fft.rfft(samples)
    freqs = np.fft.rfftfreq(len(samples), 1.0 / SAMPLE_RATE)
    spectrum[(freqs < BAND_LOW_HZ) | (freqs > BAND_HIGH_HZ)] = 0.0
    return np.fft.irfft(spectrum, len(samples))


def moving_rms(samples, window_ms):
    width = max(SAMPLE_RATE * window_ms // 1000, 1)
    power = np.convolve(samples ** 2, np.ones(width) / width, "same")
    return np.sqrt(power)


def frame_openness(samples):
    """Openness percent for each FRAME_MS frame."""
    voice = band_limit(samples)
    fast = moving_rms(voice, FAST_WINDOW_MS)
    slow = moving_rms(voice, SLOW_WINDOW_MS)
    articulation = np.clip(fast - TREND_WEIGHT * slow, 0.0, None)
    articulation /= max(float(articulation.max()), 1e-9)

    frame_len = SAMPLE_RATE * FRAME_MS // 1000
    count = len(articulation) // frame_len
    level = articulation[:count * frame_len].reshape(count, frame_len)
    level = level.mean(axis=1)
    ratio = np.clip((level - OPEN_START) / (OPEN_FULL - OPEN_START), 0.0, 1.0)
    return (np.round(ratio * 100 / LEVEL_STEP_PCT) * LEVEL_STEP_PCT).astype(int)


def merge(openness, total_ms):
    """Collapse runs of equal openness into (open_pct, hold_ms) keyframes."""
    keyframes = []
    for pct in openness:
        if keyframes and keyframes[-1][0] == pct:
            keyframes[-1][1] += FRAME_MS
        else:
            keyframes.append([int(pct), FRAME_MS])
    # Make the script last exactly as long as the clip, ending shut.
    covered = len(openness) * FRAME_MS
    if keyframes[-1][0] != 0:
        keyframes.append([0, 0])
    keyframes[-1][1] += max(total_ms - covered, 0)
    return keyframes


def find_clips(voice_dir):
    """Clips sorted by their leading number, which must run 1, 2, 3, ..."""
    clips = []
    for path in voice_dir.glob("*.mp3"):
        match = re.match(r"(\d+)", path.name)
        if match is None:
            sys.exit(f"{path.name}: name must start with its advert number")
        clips.append((int(match.group(1)), path))
    clips.sort()
    numbers = [number for number, _ in clips]
    if numbers != list(range(1, len(clips) + 1)):
        sys.exit(f"clip numbers must be 1..{len(clips)}, got {numbers}")
    return [path for _, path in clips]


def make_script(path):
    samples = load_mono(path)
    total_ms = len(samples) * 1000 // SAMPLE_RATE
    keyframes = merge(frame_openness(samples), total_ms)
    if len(keyframes) > MAX_KEYFRAMES:
        sys.exit(f"{path.name}: {len(keyframes)} keyframes, max "
                 f"{MAX_KEYFRAMES}")
    if max(ms for _, ms in keyframes) > MAX_HOLD_MS:
        sys.exit(f"{path.name}: a hold exceeds {MAX_HOLD_MS} ms")
    return keyframes, total_ms


def format_script(number, path, keyframes, total_ms):
    rows = "\n".join(f"    {{{pct}U, {ms}U}}," for pct, ms in keyframes)
    return f"""/// Jaw script for {path.name} (/ADVERT/{number:04d}.mp3), {total_ms} ms.
constexpr Keyframe SPEECH_{number}[] PROGMEM = {{
{rows}
}};
"""


def write_header(scripts):
    arrays = "\n".join(format_script(*script) for script in scripts)
    entries = "\n".join(
        f"    {{SPEECH_{number}, "
        f"sizeof(SPEECH_{number}) / sizeof(SPEECH_{number}[0])}},"
        for number, _, _, _ in scripts)
    HEADER.write_text(f"""/// @file mouth_script.hpp
/// @brief Jaw keyframes synced to the voice clips.
///
/// GENERATED by tools/make_jaw_script.py -- do not edit by hand. Re-run the
/// script whenever the voice clips change.

#pragma once

#include <avr/pgmspace.h>
#include <stdint.h>

namespace skeleton
{{

/// @brief One step of a jaw script.
struct Keyframe
{{
  uint8_t open_pct;   ///< 0 = closed, 100 = fully open.
  uint16_t hold_ms;   ///< How long to stay here before the next frame.
}};

/// @brief A jaw script stored in flash; read frames with memcpy_P().
struct Speech
{{
  const Keyframe* frames;  ///< PROGMEM array of keyframes.
  uint8_t length;          ///< Number of keyframes.
}};

// Jaw openness over time, starting when the voice starts playing. Each
// script's hold times add up to its clip's length. The keyframes live in
// flash: together they would fill much of the Uno's 2 KB of RAM.
{arrays}
/// Script i belongs to advert file i + 1.
constexpr Speech SPEECHES[] = {{
{entries}
}};

constexpr uint8_t SPEECH_COUNT =
    static_cast<uint8_t>(sizeof(SPEECHES) / sizeof(SPEECHES[0]));

}}  // namespace skeleton
""")


def main():
    voice_dir = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_DIR
    clips = find_clips(voice_dir)
    if not clips:
        sys.exit(f"no .mp3 files in {voice_dir}")
    scripts = []
    for number, path in enumerate(clips, start=1):
        keyframes, total_ms = make_script(path)
        scripts.append((number, path, keyframes, total_ms))
        print(f"{path.name}: {len(keyframes)} keyframes, {total_ms} ms")
    write_header(scripts)
    print(f"{len(scripts)} scripts -> {HEADER}")


if __name__ == "__main__":
    main()
