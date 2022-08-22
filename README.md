# Real-time closed loop phase detection

This library adapts a closed-loop phase detection algorithm (from open-ephys modules) to work inside the faster SpikeGadgets environment. This software detects 4 possible phases in your band-pass of interest (peak, trough, rising zero, falling zero). If band-pass power rises above a threshold on some user-defined set of tetrodes with the correct phase, it triggers a laser to modify brain activity. This was used to pilot exploring a role for theta and beta phase-coding on animal behavior.

A mirror of this code with accompanying trodes software can be found here, in the PhaseDisrupt feature branch:
https://bitbucket.org/mkarlsso/trodes/src/PhaseDisrupt/

My implementation rides atop Loren Frank Lab's FSData/FSGui ripple-closed loop detection. This tool is a direct descendent of their closed-loop module.
