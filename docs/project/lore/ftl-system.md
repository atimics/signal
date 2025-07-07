🚀 FTL Navigation Planning – Simplified Design Doc

✦ Core Concept

The player selects FTL routes between derelicts or star systems.
Routes form dynamically based on gravitational field intersections and change over time due to orbiting hazards or AI activity. The player sees only what their sensors reveal and must decide how much to risk—or invest in better clarity.

⸻

🎮 Gameplay Summary
• Player selects destination.
• System displays potential FTL routes (Sheaths), with visible stability and threat indicators based on sensor level.
• Player optionally slots Echoes, buys scans, or uses forecasts to improve clarity.
• Player commits to a route.
• Outcome may vary depending on actual stability vs. predicted data.

⸻

🧩 Core Mechanics

Element Description
FTL Nodes Start and end points for FTL travel (wrecks, derelicts, nav buoys).
FTL Lanes Temporary threads between nodes. Valid when local gravity vectors cancel.
Hazards Asteroids, pirates, or Warden scouts that may block a path.
Sensor Coverage Determines how accurate the player’s data is.
Echoes / Forecasts Slottable items that reveal hidden info about lanes or threats.
Stability Score Value (0–100%) indicating likelihood of successful travel.

⸻

🖥 UI Elements (FTL Planner)

UI Element Function
Map View 2D layout of FTL nodes and lanes (colored lines)
Lane Color Blue (stable), Yellow (uncertain), Red (unstable)
Flicker Intensity Higher = lower sensor confidence
Hover Panel Shows threat type, forecast window, stability, last scan time
Echo Slot Panel Drag & drop interface for unlocking better intel
Confirm Button Player locks in route and jumps

⸻

🧪 Example Simulation

Situation:
• Player at WRECK-A, wants to travel to DERELICT-B.
• Three possible FTL routes:
1. Route X: 85% stable, last scan 10 min ago, asteroid drift possible
2. Route Y: 50% stable, known pirate movement nearby
3. Route Z: Hidden — requires slotting a “Fleet Command Echo”

Action:
• Player drags in the Echo.
• Route Z appears: 95% stable for next 5 min, then collapses due to fragment sweep.
• Player selects Route Z, jumps immediately.

Outcome:
• Successful instant jump.
• AI voice: “Threaded. Resonance sheath held. Arrival achieved.”

⸻

🛠 Technical Notes (for Prototyping)
• Use 2D nodes connected by procedural links.
• Each link gets a stability_score, hazard_chance, and reveal_state.
• Run time-based updates every N ticks to simulate:
• Moving hazards (linear drift or orbital arcs)
• Route decay (invalidation if obstructed)
• Trigger events (e.g., pirate ambush) if actual hazard ≠ predicted stability.

⸻

✅ Scope Checklist for Vertical Slice
• 3–5 static nodes
• Dynamic hazards (basic movement)
• Route display with stability color
• Echo slot UI mock
• Commit & result logic (success/failure)
• Basic voice/text feedback

⸻

Would you like a JSON-based simulation prototype for lane generation and hazard resolution next? Or a wireframe image of the UI concept?

hmmm i want it more like you can plot a direct line but its risky for "collapse" reasons u may dump out at any point or short hops which are "safer" but you might hop into an ambush that could be a visual interface showing an increasingly twisted line to your destination (in 3d?)
Great direction — this introduces a powerful dynamic:

⚠️ Direct jumps are fast but unstable (can collapse mid-way).
✅ Short hops are stable but increase exposure to interdiction events (ambushes, Warden scans, etc).

You’re essentially designing a risk spectrum navigation tool with:
• A physically simulated 3D signal field
• Visual feedback through an FTL path visualizer
• Emergent strategy between survivability vs. speed

Here’s how to structure it:

⸻

🧭 DESIGN CONCEPT: “FTL Threadline Planner”

🎮 Player Experience
1. Click a destination wreck
2. The ship’s system simulates a possible direct FTL line
• It’s glitchy, distorted, maybe “buzzing” visually
3. The player can choose to:
• Accept the full jump, risking collapse
• Insert “hop points” along the route for stability
4. The more hops added:
• The signal line straightens and stabilizes
• But potential hazard regions are revealed (e.g. Chrome-Baron listening posts, Warden sweeps)

⸻

🧠 System Summary

Concept Mechanics
FTL Line A 3D Bezier-style or spline curve between two points
Collapse Field A computed instability score based on mass fields + resonance drift
Hop Points Anchors that reduce field instability but increase timeline risk
Interdiction Risk Each hop increases time → factions may react or spawn patrols
Visual Feedback Signal thread twists and distorts with instability

⸻

🧰 Game Mechanics

Base Jump (Direct Line)
• Instability Score: Simulated based on gravitational turbulence along the direct path
• Represented as:
• Line distortion
• “Flicker” pulse through the thread
• Percentage: e.g. “57% chance of sheath collapse”

Hop Insertion
• Player can insert up to 3–5 intermediate “micro-jump” points
• Each hop:
• Snaps to a nearby resonance-friendly zone (a null point in the field)
• Smooths the line visually (less curve tension)
• Reduces collapse risk
• Adds travel time: AI can react to signal buildup

Interdiction Events
• Simulated behind the scenes
• Triggered if:
• Your signature stays visible too long
• A hop point intersects a faction patrol path
• You fail to use an Echo or cloaking device

⸻

🎨 UI / VISUALIZATION (3D or Stylized 2.5D)

In-Cockpit Planner:
• Space backdrop with star parallax
• FTL nodes glow faintly
• Selected destination shows:
• A primary route thread (glowing, twisting)
• Instability areas: rendered as pulses or “noise” along the line
• Option to drag out hop points, which visibly straighten and calm the line

Example Visual Progression:

[WRECK A]-----------------------/----------[TARGET]

Direct jump:

Twisting spline with frequent “fizz” or resonance overloads
High instability
With hops:
[WRECK A]---o----o---o-----[TARGET]

Hops visible as nodes
Line gently bends and stabilizes
Instability pulses drop
⸻

🧪 Example Flow

Player wants to jump 600km to WRECK-12.
• Direct jump:
• Simulated instability: 65%
• Possible outcome: collapse at 420km → dumped near a rogue wreck
• Player adds 2 hop points:
• Instability drops to 12%
• But AI pings predict a Chrome-Baron patrol may reach the 2nd hop in 3 minutes

Player decision:
• Gamble for speed?
• Play it safe and risk a fight?

⸻

🔧 Prototype Implementation

To keep it lean in your prototype:
• Simulate only 1–3 key gravitational influences (wrecks, planetoids)
• Use splines or Bezier curves in 3D space to draw FTL lines
• Calculate instability as:

instability = sum(gravity_gradient_along_path) + noise(time) + signal_strength_modifiers

1
2
3
•	Visual feedback:
•	Line thickness/pulse = collapse risk
•	Hops = stable anchor points (smoothed spline segments)
⸻

✅ Why This Works
• Tight player choice loop: fast vs. safe
• Low dev cost: simulate “movement” without real-time space flight
• Rich visuals: easy to stylize as part of UI
• Narrative plug-in: Echoes and AI events are deeply tied to these routes