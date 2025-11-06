# CLA2A Style Stereo Leveler

This repository contains a JUCE-based stereo dynamics processor inspired by classic opto compressors. The plug-in provides three primary controls:

- **Gain** – Input gain stage in dB for pre-compression trim.
- **Comp** – CLA2A-style compression intensity mapped to threshold, ratio, and time constants.
- **De-esser** – Optional high-frequency attenuation that can be toggled on/off.

The project is configured for Visual Studio through JUCE's CMake workflow. Build targets include VST3 and a standalone application.

## Getting Started

1. **Clone JUCE** (if you do not already have it locally):
   ```bash
   git clone https://github.com/juce-framework/JUCE.git
   ```

2. **Configure the project with CMake**:
   ```bash
   cmake -B build -S . -DJUCE_DIR=/path/to/JUCE
   cmake --build build --config Release
   ```
   On Windows, you can open the generated Visual Studio solution in the `build` directory instead.

3. **Run or deploy** the generated VST3/standalone binaries from the build output folder.

## Processing Overview

- A smoothed envelope follower controls gain reduction to emulate opto-like behavior.
- Compression strength simultaneously adjusts threshold, ratio, and timing to deliver a leveling feel.
- The de-esser stage uses a simple high-band detector with adaptive attack/release and a soft ceiling.

Feel free to adjust parameter ranges or styling to match your workflow.
