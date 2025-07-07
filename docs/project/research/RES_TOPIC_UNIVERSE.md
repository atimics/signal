# Procedural Universe Generation Research

**STATUS: FUTURE VISION DOCUMENT**  
**CURRENT IMPLEMENTATION: See `RES_CANYON_RACING_UNIVERSE.md` for aligned current state**

This document explores procedural generation concepts for the aspirational **Ghost Signal** universe as described in the lore documents. The current game implementation is a canyon racing experience - this document represents a potential future evolution of the project.

---

## Context: Current vs. Future Vision

**Current Game (Implemented):**
- High-speed canyon racing with checkpoints and time trials
- Precision flying controls optimized for racing
- Progressive difficulty through medal-based track unlocking
- Environmental challenges (wind, fog, narrow passages)

**Future Vision (This Document):**
- The Ghost Signal: atmospheric survival exploration  
- Symbiotic ship requiring "fuel" from mysterious broadcasts
- Vast derelict ships (Aethelian Fleet) as exploration environments
- A-Drive surface-skimming mechanics for traversal

---

## 1. High‐Level ArchiteBelow is a **procedural derelict design philosophy** aligned with **The Ghost Signal** game vision—creating the atmospheric Aethelian Fleet Graveyard where the player's symbiotic ship must navigate to survive.

---

# 1. Vision & Goals (Aligned with Ghost Signal Lore)

* **The Graveyard as Setting:** Procedurally generate the colossal Aethelian Ark-Ships that form the primary environment—massive derelicts floating in a haunted debris field.
* **A-Drive Focused Traversal:** Derelicts are designed for the **Attraction Drive** mechanic—vast hulls and corridor systems perfect for high-speed surface-skimming navigation.
* **Atmospheric Exploration:** Each derelict contains **Resonance Locks** hiding Echoes (lore fragments and schematics), creating natural exploration goals within the tense survival framework.
* **The Five Factions:** Procedural generation must support encounters with Drifters, Chrome-Barons, Echo-Scribes, Wardens, and evidence of the fallen Aethelians—each with distinct architectural signatures.─────────────────┐      ┌───────────────────┐      ┌───────────────────┐
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
Below is a **procedural derelict design philosophy** aligned with **The Ghost Signal** game vision—creating the atmospheric Aethelian Fleet Graveyard where the player's symbiotic ship must navigate to survive.

---

# 1. Vision & Goals (Aligned with Ghost Signal Lore)

* **The Graveyard as Setting:** Procedurally generate the colossal Aethelian Ark-Ships that form the primary environment—massive derelicts floating in a haunted debris field.
* **A-Drive Focused Traversal:** Derelicts are designed for the **Attraction Drive** mechanic—vast hulls and corridor systems perfect for high-speed surface-skimming navigation.
* **Atmospheric Exploration:** Each derelict contains **Resonance Locks** hiding Echoes (lore fragments and schematics), creating natural exploration goals within the tense survival framework.
* **The Five Factions:** Procedural generation must support encounters with Drifters, Chrome-Barons, Echo-Scribes, Wardens, and evidence of the fallen Aethelians—each with distinct architectural signatures.

---

# 2. Core Principles (Ghost Signal Aligned)

1. **Symbiotic Ship Survival**

   * The player's failing ship needs the **Ghost Signal** to repair its alien core—this drives all exploration.
   * Derelicts must contain **Resonance Locks** that yield Echoes when cracked.

2. **Aethelian Aesthetic**

   * **Organic-Mechanical Fusion:** Curved corridors, crystalline growths, bio-mechanical interfaces.
   * **Corruption Patterns:** Original pristine Aethelian design corrupted by the Ghost Signal collapse.

3. **A-Drive Traversal Design**

   * **Macro:** Vast external hulls perfect for surface-skimming at high speed.
   * **Meso:** Internal corridor networks with enough scale for A-Drive navigation.
   * **Micro:** Resonance Lock chambers and Echo repositories requiring careful approach.

4. **Faction Signatures**

   * **Warden zones:** Pristine military hardware sections with active defenses.
   * **Chrome-Baron salvage:** Crude industrial modifications and scavenged sections.
   * **Echo-Scribe nodes:** Organic AI growths with glowing glyphs and data conduits.

---

# 3. Aethelian Ark-Ship Generation

### 3.1. Ship Class Templates

* **Philosophy-Ships:** Massive cathedral-like structures with curved organic forms, perfect for A-Drive surfing
* **Research Vessels:** Modular, crystalline growths housing laboratories and data cores
* **Habitat Arks:** Residential cylinders with parkland sections now overgrown with corrupted bio-tech
* **Military Escorts:** Warden-controlled ships with geometric, pristine military architecture

### 3.2. Corruption Patterns

The Ghost Signal collapse ~600 years ago left distinctive marks:

* **Signal Crystals:** Jagged growths emanating from failed digital immortality nodes
* **Bio-mechanical Fusion:** Original Aethelian organic tech run wild and corrupted
* **Void Rifts:** Sections where the hull has been torn open, exposing the structure to space
* **Resonance Wells:** Deep chambers where the Ghost Signal pools strongest

### 3.3. A-Drive Surface Design

Each derelict must support high-speed surface skimming:

```c
// Hull curvature optimized for A-Drive physics
hull_surface = smooth_spline_with_rails(ark_template);
navigation_channels = carve_racing_lines(hull_surface, complexity);
anchor_points = place_magnetic_tethers(navigation_channels);
```

---

# 4. The Resonance Cascade Integration

### 4.1. Echo Placement Strategy

Based on the **Resonance Cascade** core loop:

1. **Plan (Resonance Map)**: Player identifies derelict signatures from safe distance
2. **Travel (Fusion Torch)**: Approach the massive Ark-Ship structure  
3. **Infiltrate (A-Drive)**: Use surface-skimming to navigate hull and corridors
4. **Exploit (Resonance Cracking)**: Find and crack Resonance Locks for Echoes
5. **Upgrade (Schematic Fragmentation)**: Return to safe zone to craft ship improvements

### 4.2. Resonance Lock Distribution

* **Hull Locks:** Accessible via A-Drive surfing, contain navigation and structural Echoes
* **Core Locks:** Deep within ship superstructure, hold critical lore and advanced schematics
* **Faction Locks:** Modified by Chrome-Barons, Wardens, or Echo-Scribes—dangerous but valuable

---

# 5. Faction Environmental Storytelling

### 5.1. Warden Presence
* **Visual:** Pristine, angular military geometry overlaid on organic Aethelian base
* **Audio:** Electronic chirping, scanning sounds, patrol alert tones
* **Mechanics:** Active defense systems, patrol routes to avoid

### 5.2. Chrome-Baron Salvage
* **Visual:** Crude industrial modifications, welded scrap, jury-rigged systems
* **Audio:** Industrial grinding, sparking electronics, rough mechanical sounds
* **Mechanics:** Scavenged resources, booby traps, territorial markers

### 5.3. Echo-Scribe Nodes
* **Visual:** Organic AI growths, flowing glyphs, data-stream visualizations
* **Audio:** Digital whispers, harmonic resonances, data transmission sounds
* **Mechanics:** Information brokers, encrypted data caches, AI communion points

---

# 6. Technical Implementation

### 6.1. Derelict Scene Descriptor
```jsonc
{
  "type": "aethelian_ark",
  "ship_class": "philosophy_vessel",
  "corruption_level": 0.7,
  "faction_presence": ["warden_patrol", "chrome_baron_salvage"],
  "resonance_locks": {
    "hull_locks": 12,
    "core_locks": 3,
    "faction_locks": 2
  },
  "scale": [2000, 800, 2000],
  "ghost_signal_intensity": 0.85
}
```

### 6.2. A-Drive Navigation Mesh
* **Surface Rails:** Procedural splines following hull curvature for optimal A-Drive performance
* **Anchor Points:** Magnetic attachment zones for precise maneuvering  
* **Transition Gates:** Entry/exit points between external surfing and internal corridor navigation

### 6.3. Ghost Signal Visualization
* **Signal Intensity Fields:** Visual representation of Ghost Signal strength for navigation
* **Resonance Hotspots:** Glowing areas indicating nearby Resonance Locks
* **Corruption Shaders:** Dynamic corruption effects on surfaces affected by the signal

---

# 7. Performance Considerations for The Graveyard

### 7.1. Massive Scale Management
* **Streaming:** Load derelict sections as player approaches via A-Drive navigation
* **LOD System:** Simplify distant Ark-Ships while maintaining visual grandeur
* **Culling:** Aggressive frustum culling for interior spaces and hull details

### 7.2. Sokol Integration
* **Instanced Debris:** Use instancing for small debris fields around each derelict
* **Modular Rendering:** Separate pipelines for hull surfing vs. interior exploration
* **Ghost Signal Effects:** Efficient particle systems for atmospheric corruption

---

# 8. Alignment with Player Experience

This procedural system directly supports the **Ghost Signal** core experience:

* **Survival Tension:** The failing ship creates urgency in every exploration decision
* **A-Drive Mastery:** Derelict geometry rewards skillful surface-skimming navigation  
* **Mystery & Discovery:** Each Echo found deepens the lore and provides concrete ship upgrades
* **Atmospheric Horror:** The corrupted Aethelian aesthetic reinforces the haunted, dangerous setting

The procedural generation ensures that while the core experience remains consistent, each journey into the Graveyard offers new derelicts to explore and new dangers to face—supporting the game's focus on **exploration, survival, and the gradual unraveling of the Aethelian civilization's tragic fate**.
