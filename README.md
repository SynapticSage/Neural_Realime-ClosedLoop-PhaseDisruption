# Real-time closed loop phase detection

Here, I re-implemented a closed-loop phase detection algorithm to work inside the faster SpikeGadgets environment. This software detects 4 possible phases in your band-pass of interest. If the power rises above a threshold on a user-defined number of tetrodes, and user-defined phase criteria, it can trigger an optical fiber. Used for causally exploring the role of phase-coding on animal behavior.

A mirror of this code with accompanying trodes software can be found here, in the PhaseDisrupt feature branch:
https://bitbucket.org/mkarlsso/trodes/src/PhaseDisrupt/
