# Particle Simulator

Real-time simulation of 1000 charged particles (protons and electrons) interacting via Coulomb's law. Same-charge particles repel, opposite-charge particles attract.

## Features

- Inverse-square force calculation between charged particles
- Spatial hash grid for near-linear collision detection (avoids brute-force O(n^2))
- Elastic wall collisions with configurable energy loss
- Interactive force control at runtime

## Build

Requires SDL2 and CMake 3.10+.

```sh
mkdir build && cd build
cmake ..
make
```

## Controls

| Key   | Action                     |
|-------|----------------------------|
| Space | Increase repulsion force   |
| Esc   | Quit                       |

## How it works

Particles are assigned to cells in a spatial hash grid based on their x-position. Force calculations only check neighboring cells, reducing the per-frame cost from O(n^2) to approximately O(n). Forces follow an inverse-square law with a softening factor to prevent singularities at close range. Newton's third law is applied so each pair is only computed once.
