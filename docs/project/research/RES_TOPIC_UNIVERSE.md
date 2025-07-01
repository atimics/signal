Here’s a **deep‐dive design** for fully procedural planet and asteroid mesh generation—**integrated** into your **Sokol/C** engine—with **performance** front and center.

---

## 1. High‐Level Architecture

```
┌──────────────────┐      ┌───────────────────┐      ┌───────────────────┐
│  Scene Config    │─┐    │  Generation Module│─┐    │   Render Module   │
│ (JSON/C structs) │ │    │   (C code)        │ │    │  (Sokol pipelines)│
└──────────────────┘ │    └───────────────────┘ │    └───────────────────┘
     feeds into      │         builds             │      draws with
                      └───────────────────────────┘
```

* **Scene Config** declares planet parameters (seed, radius, LOD levels) and asteroid belt specs.
* **Generation Module** (e.g. `planet_mesh_gen.c` / `asteroid_gen.c`) runs at scene load:

  * Builds one **sphere mesh** with procedural displacement for each planet LOD.
  * Builds one **asteroid “prototype” mesh** by deforming an icosphere.
  * Creates **instance data buffers** (model matrices, scales, colors).
* **Render Module** uses Sokol’s:

  * **Instance‐buffer draws** (`sg_draw(…, instance_count)`).
  * **Frustum culling** and **distance‐based LOD switching** on CPU.
  * **Fullscreen quad & shader** for atmospheric glow if needed.

---

## 2. Procedural Planet Mesh

### 2.1. Base Geometry: Cube→Sphere (Minimal Vertex Count)

* Start with a **unit cube** (6 faces, 8 verts).
* Subdivide each face N times (e.g. N=16 for medium LOD, N=64 for high LOD).
* **Normalize** each vertex to lie on the unit sphere:

  ```c
  for each vert v:
      v = normalize(v);
  ```
* **Pros:** uniform distribution, easier UVs.

### 2.2. Noise‐Based Terrain & Normals

* Apply **fractional Brownian motion** (fBM) noise to displace each vertex radially:

  ```c
  float h = fBM3D(v * terrain_scale, octaves);  
  v = normalize(v) * (1.0 + h * max_displacement);
  ```
* Recompute normals by:

  * Either analytical: perturb normal = normalize(position\_after – position\_before)
  * Or via cross‐product of triangle edges.

### 2.3. LOD Generation

* **LOD0**: Cube‐sphere subdiv N0 (low)
* **LOD1**: subdiv N1 (medium)
* **LOD2**: subdiv N2 (high)
* At runtime, choose mesh based on camera‐to‐planet distance:

  ```c
  if (dist < near) use LOD2;
  else if (dist < mid) use LOD1;
  else use LOD0;
  ```

---

## 3. Procedural Asteroid Mesh

### 3.1. Prototype Mesh Creation

* Start from an **icosahedron** (20 triangles, 12 verts).
* **One or two subdivisions** for \~80–320 triangles.
* **Randomize** each vertex along its normal:

  ```c
  float r = 1.0 + (rand(seed) * 2.0 - 1.0) * deformation_scale;
  v = normalize(v) * r;
  ```
* **Bake once** per belt (or per a few “types”)—reusing mesh for thousands of instances.

### 3.2. Instance Data

* For each belt asteroid:

  * **Model matrix**: translate into a ring/cloud, scale `(rand(0.5, 2.0))`, and random rotation.
  * **Per‐instance color tint**: slight variation for visual interest.

---

## 4. Performance Strategies

### 4.1. One‐Time Generation

* Run mesh generation **only at scene load** (or offline), not per frame.
* **Cache** generated `sg_buffer` and reuse between runs.

### 4.2. Instancing & Batching

* Use **one** `sg_pipeline` and **one draw call** per mesh‐LOD:

  ```c
  sg_apply_pipeline(pip_planet);
  sg_apply_bindings({ .vertex_buffers = { sphere_buf, planet_inst_buf } });
  sg_draw(0, sphere_num_verts, planet_count);
  ```
* Similarly for asteroids:

  ```c
  sg_apply_pipeline(pip_asteroid);
  sg_apply_bindings({ .vertex_buffers = { ast_buf, ast_inst_buf } });
  sg_draw(0, ast_num_verts, belt_count);
  ```

### 4.3. CPU Frustum Culling & LOD

* Before updating instance buffers, **filter** by distance & view‐frustum.
* Only upload visible instances (e.g., cull 80% of far‐off asteroids).

### 4.4. GPU‐Side Displacement (Advanced)

* For planets, consider a **vertex‐shader noise** approach:

  * Use low‐poly sphere and displace in VS with 3D noise texture.
  * Eliminates multiple LOD meshes; noise cost is low on GPU.
* Example VS snippet:

  ```glsl
  vec3 pos = normalize(a_position);
  float h = fbm(pos * u_scale);
  pos *= (1.0 + h * u_height);
  ```

---

## 5. Integration in Sokol C

### 5.1. Data Structures

```c
typedef struct {
  sg_buffer mesh_buf;
  int     num_verts;
  sg_buffer inst_buf;
  int     inst_count;
} MeshSet;

MeshSet planets[NUM_LODS];
MeshSet asteroid_set;
```

### 5.2. Scene Init

```c
load_config(&cfg);
generate_planet_meshes(planets, cfg.planet_params);
generate_asteroid_prototype(&asteroid_set, cfg.belt_params);
allocate_instance_buffers();
```

### 5.3. Frame Loop

```c
update_orbits(dt, cfg.planet_params, planet_instances);
update_belt(dt, cfg.belt_params, asteroid_instances);
frustum_cull(&planet_instances, cam, visible_planets);
frustum_cull(&asteroid_instances, cam, visible_asteroids);
sg_update_buffer(planets[current_lod].inst_buf, visible_planets, ...);
sg_update_buffer(asteroid_set.inst_buf, visible_asteroids, ...);
render_planets_and_asteroids();
```

---

## 6. Memory & Performance Budget

| Component          | Mesh Size    | Instances | Draw Calls | Notes                        |
| ------------------ | ------------ | --------- | ---------- | ---------------------------- |
| Planet LOD0        | \~1 K verts  | 1         | 1          | small CPU & GPU cost         |
| Planet LOD2        | \~65 K verts | 1         | 1          | only for very close camera   |
| Asteroid Prototype | \~500 verts  | \~2000    | 1          | instanced, low per‐vert cost |

* **Total buffers**: 3 sphere meshes + 1 asteroid mesh
* **Draw calls per frame**: ≤ 4 (sun, planets, asteroids, gameplay)
* **Per‐frame CPU**: frustum cull + buffer updates (\~1–2 ms on desktop)

---

### ✅ Summary

By **procedurally generating**:

* **Planets** via cube-to-sphere + noise (or GPU VS noise), with LOD switching
* **Asteroids** via randomized icospheres, instanced in a belt

…you achieve **maximal variety**, **minimal asset footprint**, and **top performance** on modern hardware—all driven by **scene-level data** and realized with **Sokol’s lightweight C API**.
Below is a **fully procedural derelict design philosophy**—covering everything from **huge, canyon-racing megastructures** to **maze-like FPS dungeons**, all built as part of a data-driven, infinite universe pipeline in a Sokol/C engine.

---

# 1. Vision & Goals

* **Derelicts as Worlds:** Every wreck is a self-contained “level” you can race through, dogfight around, or explore on foot.
* **Multi-Mode Gameplay:** Same geometry supports:

  * **High-speed surface racing** (canyon runs, narrow chases).
  * **FPS-style exploration** (maze puzzles, loot rooms).
* **Infinite Variety:** Artists/configs define **module libraries** and **generation rules**—the engine seeds each scene for endless unique derelicts.
* **Seamless Universe:** Derelicts slot into a larger system alongside stars, planets, and asteroid belts, sharing the same procedural pipeline.

---

# 2. Core Principles

1. **Data-Driven & Seeded**

   * All parameters (size, shape, layout style) come from a small JSON/C struct with a seed.
   * Changing the seed or ruleset yields completely new wrecks.

2. **Modular, Hierarchical Generation**

   * **Macro:** Determine wreck envelope (bounding shell, overall shape).
   * **Meso:** Lay out main “spine” corridors and branching veins.
   * **Micro:** Snap together predefined modules (tunnels, chambers, open arenas).

3. **Dual-Path Support**

   * **Racing Lines:** Identify 1–3 fast, flowing paths for vehicles.
   * **Dungeon Mesh:** Intersect with a graph of branching corridors and rooms for on-foot play.

4. **Streaming & LOD**

   * Derelicts can be vast—stream in chunks based on player position.
   * Use instanced meshes and GPU frustum culling to keep performance high.

---

# 3. Macro-Scale: Wreck Envelope & Profile

* **Shape Templates:** Box, cylinder, ring, composite.
* **Size Parameters:**

  * `radius_x/y/z` (scale can be non-uniform for “flattened” or “elongated” wrecks).
  * `complexity` (number of main corridors, branch density).
* **Carve-Outs & Ruptures:** Random planes or spheres cut holes to expose “canyon” walls.

**Algorithm:**

```pseudocode
envelope = choose_template(seed)
envelope.apply_noise_displacement(amplitude, frequency)
envelope.carve_out(seed2, plane_count, sphere_count)
```

This gives you a giant, broken shell with dramatic openings and overhangs.

---

# 4. Meso-Scale: Spine & Vein Layout

1. **Spine Graph**

   * A central spline tracing a “race loop”: a closed path hugging the envelope interior.
   * Control points seeded for difficulty (tighter turns, vertical drops).

2. **Branch Generation**

   * At intervals along the spine, spawn branching corridors that dead-end or rejoin later.
   * Use **L-systems** or **branching noise** to vary length and angle.

3. **Dungeon Graph**

   * Overlay a **maze graph** (e.g. recursive backtracker) inside unused volume.
   * Ensure connectivity: at least one path from spawn to boss room.

4. **Intersection Rules**

   * Allow spine & maze to interconnect at designated “junction modules” so you can switch modes.

---

# 5. Micro-Scale: Module Library

* **Corridor Modules:**

  * Straight, curved, sloped, vertical shafts.
* **Chamber Modules:**

  * Open arenas (for racing overtakes), dead-end rooms (FPS loot), puzzle rooms.
* **Transition Modules:**

  * Broken floors (gaps you must jump/drive through), grated catwalks, tunneled breaches.
* **Surface Detail Props:**

  * Debris, cables, pipes, catwalk rails—placed procedurally to sell scale.

**Module Specs:**

* Snap points on a uniform grid (e.g., 5m increments).
* Metadata tags: `race_path`, `fps_only`, `hazard_zone`, `secret_entrance`.

---

# 6. Dual Gameplay Paths

### A. Racing Paths

* **Extract** the fastest route by sampling the spine and branches: maximize length while minimizing sharp angles.
* **Tag** modules on that path with `race_path=true`.
* **Spawn** speed-boost pickups or shortcuts on these edges.

### B. Dungeon Crawls

* **Use** the maze graph to place key rooms (security locks, lore Echoes).
* **Procedural puzzles:** lockable doors requiring “Resonance” keys found along racing path (encourages dual-mode play).

---

# 7. Integration into Universe Pipeline

* **Scene Descriptor:**

  ```jsonc
  {
    "type":"derelict",
    "envelope_template":"cylinder",
    "seed": 12345,
    "size":[200,100,200],
    "corridor_density":0.3,
    "maze_complexity":0.2
  }
  ```
* **Scene Loader:**

  1. Parse descriptor.
  2. Call `generate_envelope()`.
  3. Build spine & maze graphs.
  4. Place modules & props.
  5. Create instance buffers & collision meshes.
* **Streaming:** Divide volume into chunks (e.g. 50m³). Generate and upload chunks on demand as the player moves.

---

# 8. Procedural Techniques & Algorithms

| Layer       | Technique                                  |
| ----------- | ------------------------------------------ |
| Envelope    | 3D noise (Perlin/fBM) + CSG carve          |
| Spine       | Catmull-Rom spline seeded + random offsets |
| Maze        | Recursive backtracker or BSP partition     |
| Module Snap | Grid snap + collision check                |
| Props       | Poisson disk sampling in corridor volumes  |

---

# 9. Performance Considerations

* **Pre-Generation:**

  * Generate meshes once at scene load (or offline).
* **Instancing:**

  * Use Sokol’s instanced draws for modules/props to minimize draw calls.
* **Frustum & Distance Culling:**

  * Cull distant chunks & modules.
* **LOD Switching:**

  * Simplify envelope geometry for far distances.
* **Memory Pools:**

  * Reuse vertex/index buffers across scenes.

---

# 10. Extensibility & Narrative Hooks

* **Tag modules** with lore events (e.g. `echo_spawn`, `warden_ambush`).
* **Dynamic events:** spawn NPC patrols on racing paths to threaten FPS players.
* **Secret branches** unlock with mechanical achievements, blending both modes.

---

**By combining**
• **Macro envelope shaping**,
• **Meso spine/maze graph algorithms**,
• **Micro modular snap-together prefabs**,
all driven by **data and seeds**, you create an **infinite array of derelict megastructures**—from super-highway canyon races to claustrophobic FPS dungeons—seamlessly integrated into your Sokol/C universe.
