# Particle Simulator

A charged particle physics sim built from scratch in C99 with SDL2. Protons and electrons interact via Coulomb's law — same charges repel, opposites attract. Started as a brute-force O(n^2) mess and evolved into something actually usable.

## What I built

- **2D spatial hashing** for near-linear force calculation — particles are binned into a grid by both X and Y position, neighbor lookups check a 3x3 cell region. Uses prime-number hashing (`73856093`, `19349669`) to distribute cells evenly across the table.
- **Particle mass** — protons are 3x heavier than electrons, so electrons zip around while protons lumber. Force is divided by mass of the recipient (F=ma, so a=F/m).
- **Velocity damping** — particles lose a tiny bit of energy each frame (0.999x multiplier) so they eventually settle into stable formations instead of flying around forever.
- **Camera system** — the world is 16000x9000 (10x bigger than the window). Mouse wheel zooms toward cursor, WASD pans. Particles scale with zoom and off-screen ones are culled.
- **Cross-language shared memory** — press P to launch a Python tkinter control panel. The C program creates a POSIX shared memory segment (`shm_open`/`mmap`), and the Python script opens the same segment via `mmap` + `struct` to read/write the particle count in real time. No sockets, no files, just raw shared memory across two languages.
- **Zombie process handling** — `signal(SIGCHLD, SIG_IGN)` so forked Python processes don't leak as zombies.

## Build

Requires SDL2, CMake 3.10+, Python 3 (for the control panel).

```sh
mkdir build && cd build
cmake ..
make
./particle_sim
```

## Controls

| Key | Action |
|---|---|
| WASD | Pan camera |
| Scroll Wheel | Zoom in/out |
| Space | Increase repulsion force |
| P | Open particle count slider |
| H | Show keybindings |
| Escape | Quit |

## How it works

Particles live in a large world space and interact via inverse-square Coulomb forces. A spatial hash grid bins particles by 2D position so only nearby pairs are checked — roughly O(n) instead of O(n^2). Newton's third law is applied so each pair is computed once. Wall collisions bounce particles with configurable energy loss. The rendering pipeline transforms world coordinates through a camera (position + zoom) to screen space.
