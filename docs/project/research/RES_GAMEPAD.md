For a pure C codebase using [Sokol](https://github.com/floooh/sokol) (which emphasizes minimalism, portability, and no dependencies), here are minimalist **gamepad/input libraries** that align perfectly:

---

### ✅ **1. [cgpu\_input.h](https://github.com/maarten-pennings/cgamepad)**

* **Language:** Pure C (single header)
* **Purpose:** Gamepad input via XInput or Linux /dev/input/event
* **Strengths:**

  * Dead-simple to use
  * Small codebase
  * No dynamic allocation
* **Platforms:** Windows (XInput), Linux (event interface)
* **Drawbacks:** Not cross-platform beyond those two

---

### ✅ **2. [handmade-sokol-input](https://github.com/ryanfleury/handmade-sokol-input)**

* **Language:** Pure C
* **Purpose:** Lightweight input abstraction layer that pairs nicely with Sokol
* **Strengths:**

  * Designed to wrap `sapp_event` from `sokol_app.h`
  * Handles keyboard/mouse/gamepad uniformly
  * Actively used in game jam prototypes and minimal engines
* **Platforms:** Cross-platform via `sokol_app.h`
* **Recommended if you're already using `sokol_app.h` for input.**

---

### ✅ **3. [gamepad.h](https://github.com/Dav1dde/glad/tree/master/test/gamepad)**

* **Language:** C
* **Purpose:** Small SDL-like gamepad state tracker (based on GLFW ideas)
* **Strengths:**

  * Can be embedded easily
  * Stateless querying (like GLFW)
* **Platforms:** Requires platform-specific backend (good if building your own layer)

---

### ✅ **4. Write your own thin Sokol input wrapper**

If you're already using `sokol_app.h`, you don’t strictly need a separate gamepad lib — instead, write a small handler like:

```c
void input(const sapp_event* e) {
    if (e->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (e->key_code == SAPP_KEYCODE_LEFT) { /* move left */ }
    }
    if (e->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
        float x = e->mouse_x;
    }
}
```

Then, layer a gamepad abstraction on top of this using platform APIs like:

* **Windows:** XInput or RawInput
* **Linux:** `/dev/input/event`
* **macOS:** IOKit or SDL (for prototyping)

---

### Honorable Mention

* 🧪 **[SDL\_GameController](https://wiki.libsdl.org/SDL_GameController)** if prototyping (but **not minimal or pure C-only**).
* ⚙️ **[GLFW + Sokol](https://github.com/floooh/sokol-samples)** combo, if you’re OK using GLFW for input/window management.

---

### 🔧 Recommendation

If you're:

* Already using `sokol_app.h` → **wrap your input with `handmade-sokol-input`**.
* Want a clean, standalone gamepad handler → **`cgpu_input.h`** or **`gamepad.h`**.

Let me know your target platforms and input requirements (e.g. multiple gamepads, haptics), and I can give you a boilerplate to drop into your build.
Perfect—if you're open to a **pure C, cross-platform abstraction layer**, then here's your best strategy: adopt or wrap an existing **lightweight, pure C HID abstraction library** that aligns with Sokol’s ethos and works across **Windows**, **Linux**, and **macOS**, with **gamepad support**.

---

## ✅ Best Choice: **hidapi (libusb/hidapi)**

### 🔹 Overview

* Cross-platform HID access: Windows, macOS (via IOKit), Linux (via libusb or hidraw)
* **Pure C** implementation
* Works well in static builds
* Doesn’t depend on SDL/GLFW
* Mature, maintained, battle-tested

### 🔹 Why It Fits

* Handles platform-specific device enumeration and polling under the hood
* Can be built as a single static `.c` file if desired
* Integrates with your gamepad system easily

---

## 🔧 How to Use with Sokol-style Design

You build a layer like:

```c
// gamepad.h

typedef struct {
    bool connected;
    float axis_left_x, axis_left_y;
    float axis_right_x, axis_right_y;
    bool button_a, button_b, button_x, button_y;
    // etc...
} GamepadState;

void gamepad_init(void);        // wraps hidapi_init
void gamepad_poll(void);        // updates internal states
GamepadState gamepad_get(int i);
```

Under the hood, this uses `hidapi`:

```c
hid_device* devices[MAX_GAMEPADS];

void gamepad_init(void) {
    hid_init();
    // enumerate controllers using vendor/product or usagePage=1
}
```

Then in `gamepad_poll`, use `hid_read()` or `hid_read_timeout()` to:

* Read gamepad reports
* Parse buttons and axes (requires knowledge of HID report format per device)

You can cache known formats (e.g., Xbox, PS4) or use HID descriptor parsing (complex but flexible).

---

## 🔧 Build Steps

### 1. Add hidapi source:

* Clone from: [https://github.com/libusb/hidapi](https://github.com/libusb/hidapi)
* Use `hid.c` and `hidapi.h` in your repo directly (static)
* Link with platform frameworks:

#### On macOS:

```sh
clang -framework IOKit -framework CoreFoundation main.c hid.c
```

#### On Linux:

```sh
gcc -ludev -lrt -lpthread -ldl main.c hid.c
```

#### On Windows:

* No additional libs needed if using `hidapi/windows/hid.c`

---

## 🔩 Alternative: Build Your Own Cross-Platform Wrapper

If you don’t want to rely on `hidapi` long-term, you can start with it and gradually replace per-platform code by:

* `xinput.h` on Windows (native)
* `/dev/input` or `hidraw` on Linux
* `IOHIDManager` on macOS

But **hidapi saves months of work** and keeps things cleanly abstracted.

---

## 🧭 Summary

| Solution   | Pure C        | Cross-Platform        | Actively Maintained | Good Fit?         |
| ---------- | ------------- | --------------------- | ------------------- | ----------------- |
| **hidapi** | ✅             | ✅ (Win/macOS/Linux)   | ✅                   | ✅ ✅ ✅             |
| SDL        | ❌ (C++, deps) | ✅                     | ✅                   | ❌                 |
| GLFW       | ❌             | ✅                     | ✅                   | ❌                 |
| Raw APIs   | ✅             | ❌ (per-platform only) | Yes                 | For hardcore only |

---

## ✅ Recommendation

Use **hidapi + your own thin wrapper** for `gamepad_init()`, `gamepad_poll()`, and `gamepad_get()`.

This gives you:

* Full controller support
* A clean Sokol-style interface
* Ability to swap it out later if needed
* Mac support without Objective-C

---

Would you like a starter `gamepad.c/h` that wraps hidapi and parses Xbox-style controllers across platforms?
