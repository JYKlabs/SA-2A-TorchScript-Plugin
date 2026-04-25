# SA-2A TorchScript Plugin

This repository contains a separate real-time JUCE plugin for hosting a TorchScript `.pt` model of an SA-2A-style optical compressor.

It is **not** the offline recording or dataset-capture tool used for the paper. Instead, it was created to run the learned compressor model in a live audio context and to provide a simple real-time interface for testing a PyTorch-exported model.

## Overview

The plugin loads a single TorchScript model file and exposes three main controls:

- **Input Gain**
- **Peak Reduction**
- **Output Gain**

The loaded model is applied in real time to the incoming audio signal. If no model is loaded, the plugin behaves as a gain-only pass-through processor.

The plugin is available in the following formats:

- VST3
- AU
- Standalone

## Features

- Real-time JUCE audio plugin
- Loads a single TorchScript `.pt` model
- Designed for SA-2A-style compressor emulation
- Three user controls:
  - Input Gain
  - Peak Reduction
  - Output Gain
- Model loading and model clearing from the plugin UI
- Stereo input/output support
- Automatic pass-through behavior when no model is loaded

## Plugin Controls

| Control | Range | Description |
|---|---:|---|
| Input Gain | 0.0 to 4.0 | Linear gain applied before model inference |
| Peak Reduction | 0.0 to 100.0 | Conditioning value passed to the model |
| Output Gain | 0.0 to 4.0 | Linear gain applied after model inference |
| Load Model (.pt) | — | Loads a TorchScript model file |
| Clear Model | — | Unloads the currently loaded model |

## Model Requirements

The plugin expects a TorchScript `.pt` file exported from PyTorch.

The model must provide a `forward()` method that accepts:

- an input audio tensor
- a peak-reduction tensor

The current implementation is built around a fixed internal frame size of **512 samples**.

For best results, the exported model should be compatible with frame-based real-time inference.

## How the Plugin Works

When audio is processed:

1. The input block is copied into an internal buffer.
2. The buffer is multiplied by the current **Input Gain**.
3. The current **Peak Reduction** value is passed to the model as a conditioning tensor.
4. The model is executed without gradients.
5. The output is multiplied by **Output Gain**.
6. The processed samples are written back to the host buffer.

If no model has been loaded, the plugin simply applies the input and output gain stages and returns the signal without learned processing.

## Usage

### 1. Build the plugin
Build the project with JUCE, CMake, and LibTorch available.

### 2. Load a model
Open the plugin interface and click:

**Load Model (.pt)**

Then select a TorchScript `.pt` file.

### 3. Adjust the controls
Set:

- **Input Gain** for input scaling
- **Peak Reduction** for the desired compressor condition
- **Output Gain** for post-processing level

### 4. Clear the model
Click:

**Clear Model**

to remove the loaded TorchScript module and return to gain-only behavior.

## Stereo Operation

The plugin is configured for stereo input and output.

Each channel is processed independently, while using the same loaded model and the same Peak Reduction control value.

## Build Requirements

- JUCE
- CMake
- LibTorch
- A C++17 compiler

## Build Notes

The project is configured with:

- JUCE plugin formats: VST3, AU, Standalone
- C++17
- TorchScript model loading

If you use a custom LibTorch installation, make sure the Torch CMake package is discoverable during configuration.

## Limitations

- Only one model file can be loaded at a time.
- Host state persistence is not implemented.
- The plugin is centered on a 512-sample processing window.
- Stereo I/O is required.
- The model must be exported as TorchScript and compatible with the plugin’s two-input forward call.

## Relation to the Research

This plugin is a separate real-time TorchScript host developed alongside the research on neural modeling of an analog SA-2A-style compressor.

It was created for live evaluation and model deployment, and is distinct from the dataset-generation and training framework used in the study.

## Related Research

This plugin is related to ongoing research on TCN-based modeling of an analog optical compressor, but it is distributed here as a standalone real-time implementation independent of the research dataset and training framework.
