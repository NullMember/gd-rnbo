# gd-rnbo: Use Cycling74's RNBO exports as stream or audio effect in Godot
RNBO is MaxMSP's new audio engine that allows you to export your Max patches as C++ code. This repository contains a GDExtension that allows you to use these exports in Godot as a stream or audio effect.

# Building

## Prerequisites
- Godot 4.0 or later
- C++ compiler (e.g. GCC, Clang, MSVC)
- SConstruct

## GDExtension

1. Clone this repository with `git clone --recurse-submodules`
2. Export RNBO C++ Source Code in `rnbo-src` folder
    - Ensure `Copy C++ library code` is checked in the export settings if you're exporting it for the first time.
3. Comment all exception throws in rnbo source code (search throw in all sources)
    - Godot does not support exceptions, so you need to comment them out. This is common in game engines for performance reasons.
    - This is required for the first time you export the RNBO patch. For consequtive exports disable the `Copy C++ library code` option in the export settings.
4. Build the GDExtension with SConstruct
5. Copy demo/addons folder to your Godot project

# Usage

## RNBO Effect

RNBO Effect is a GDExtension that allows you to use RNBO patches as audio effects in Godot. It is a subclass of AudioEffect and can be used in the same way as any other audio effect in Godot.  
In RNBO you can get audio input using `in~ 0` and `in~ 1` (left and right channel respectively) and send audio output using `out~ 0` and `out~ 1`.

## RNBO Stream

You can use AudioStreamPlayer2D or AudioStreamPlayer3D to play RNBO stream. Add RNBOStream as a stream to the AudioStreamPlayer2D or AudioStreamPlayer3D node.  
If you set playing to true, RNBOStream will send 1 to `param play` in the RNBO patch.
If you set `param stop` to 1 in RNBO patch, the stream will stop playing in Godot. 

## Parameters in RNBO patch

All parameters in the RNBO patch will be automatically exposed in Godot editor (except play and stop parameters). You can access all parameters you created in the RNBO patch using the same way you access any AudioEffect or AudioStream parameters in Godot.
