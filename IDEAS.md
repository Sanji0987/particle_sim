# Ideas

## Gameplay / Visuals
- Mouse interaction — click/drag to attract or repel particles, or spawn new ones
- Particle trails — fade-out trails behind particles showing their path
- Color by velocity — map particle speed to a color gradient (slow=blue, fast=red)
- Gravity toggle — add a downward gravity force you can turn on/off
- Particle count slider — let the user increase/decrease particles at runtime

## Physics
- 2D spatial hashing — grid only hashes on X position; adding Y would cut unnecessary force checks
- Barnes-Hut tree — quadtree-based approximation for O(n log n) force calculation, scale to 10k+ particles
- Velocity damping / friction — particles never lose energy (energy_loss_factor is 1.0), adding drag would create stable formations
- Particle mass — give protons and electrons different masses so they respond differently to forces

## Technical
- Delta time — use actual frame delta instead of fixed SDL_Delay(16) so physics stays consistent
- Multithreading — split force calculations across threads with pthreads or OpenMP
- GPU compute — move force calculations to a compute shader (OpenCL or Vulkan compute)
- FPS counter — render frame time/FPS on screen with SDL_ttf
