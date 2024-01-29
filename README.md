# Real-time closed loop phase detection

This library adapts a closed-loop phase detection algorithm (from open-ephys modules) to work inside the faster SpikeGadgets environment. 

This software detects 4 possible phases in your band-pass of interest (peak, trough, rising zero, falling zero). If band-pass power rises above a threshold on some user-defined set of tetrodes with the correct phase, it triggers a laser to modify brain activity. This was used to pilot exploring a role for theta and beta phase-coding on animal behavior.

A mirror of this code with accompanying trodes software can be found here, in the PhaseDisrupt feature branch:
https://bitbucket.org/mkarlsso/trodes/src/PhaseDisrupt/

My implementation rides atop Loren Frank Lab's FSData/FSGui ripple-closed loop detection. This tool is a direct descendent of their closed-loop module and adds the ability to trigger a laser using phase or amplitude of an arbitrary band-pass signal (user-configurable frequencies).

Below is an example of fast beta (20-30hz) phase disurption, with brain samples streaming at 1500hz. I can also acheive reliable theta wave  (6-12hz) disruption with this.
![Example of beta rhythm detection](https://github.com/SynapticSage/Realtime-neural-phaseDisruption/raw/master/beta_detection.png)

Key capabilities of this closed-loop code, old and new:

## Neural data acquisition

- Acquires continuous neural data streams and spike waveforms from multiple electrodes
- Configurable selection of electrodes and sampling rates
- UDP data streaming from acquisition module to processing code

## Real-time feature detection

- Bandpass filters data to isolate oscillations like theta, beta, ripple bands
- Detects amplitude, phases, and rhythmicity of neural signals 
- Algorithms for spatial position, ripple , theta phase targeting

## Closed loop stimulation

- Triggers electrical or optogenetic stimulation in real-time based on detected features
- Highly configurable stimulation parameters and patterns
- Precise targeting of stimulation timing based on phase or amplitude detection

## Graphical configuration interface

- GUI for selecting electrodes, configuring algorithms, designing stimulation patterns
- Saves/loads system configurations for repeatable experiments 

## Optimized for real-time performance

- C++ code and libraries optimized for real-time signal processing
- Sub-millisecond latency from data acquisition to stimulation output
- Runs reliably on commodity Linux workstations with minimal dropped data
