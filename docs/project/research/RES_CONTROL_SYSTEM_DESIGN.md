# Research: Next-Generation Control System Design

**Document ID**: RES_CONTROL_SYSTEM_DESIGN  
**Date**: July 6, 2025  
**Author**: Gemini

## 1. Introduction

This document outlines the research and design goals for the next-generation control system for the SIGNAL engine. The recent implementation of a new event-driven input system (`src/services/input_service.h`) has laid the foundation for a more flexible, data-driven, and user-friendly control architecture. This document builds on that foundation to propose a clear path forward for Sprint 25 and beyond.

## 2. Analysis of Recent Changes

A review of the recent commit history reveals a clear trajectory:

-   **Decoupling**: The introduction of the `InputService` and `InputHAL` successfully decouples low-level hardware events from high-level game actions.
-   **Centralization**: The `control_system` (`src/system/control.c`) is now the single point of authority for translating player input into entity actions (e.g., thruster commands).
-   **Modernization**: Legacy input systems have been removed, and the codebase now consistently uses the new event-driven model.

The groundwork is solid, but the final link—a flexible mapping between raw input and game actions—is not yet fully implemented.

## 3. High-Level Design Goals

For the next sprint, the control system should evolve to meet the following design goals:

1.  **Complete Decoupling**: The mapping of physical keys and buttons to game actions (e.g., `W` key to `THRUST_FORWARD`) should be entirely data-driven, not hardcoded.
2.  **Context-Sensitivity**: The control scheme should change automatically based on the game state (e.g., "In-Flight", "Main Menu", "Map Screen").
3.  **Extensibility**: Adding new control schemes (e.g., for different ship classes, or for on-foot controls) should not require changing the core control system code.
4.  **User Customization**: The architecture should eventually support user-configurable keybindings by simply modifying the data files that define the control maps.

## 4. Proposed Architecture: Input Action Maps

To achieve these goals, I propose the implementation of an **Input Action Map** system.

### 4.1. Core Concepts

-   **Action Map**: A data structure that defines a set of bindings between raw hardware inputs (e.g., `SAPP_KEYCODE_W`, `SAPP_GAMEPAD_BUTTON_A`) and abstract `InputActionID`s (e.g., `INPUT_ACTION_THRUST_FORWARD`).
-   **Control Context**: A named state (e.g., "flight", "menu") that determines which Action Map is currently active. The `InputService` will be responsible for managing a stack of these contexts.

### 4.2. Data-Driven Configuration

Action Maps will be defined in a simple data format (e.g., JSON or YAML) and loaded at startup. A hypothetical `action_maps.json` might look like this:

```json
{
  "contexts": [
    {
      "name": "flight_controls",
      "bindings": [
        { "input": "KEY_W", "action": "THRUST_FORWARD" },
        { "input": "KEY_S", "action": "THRUST_BACK" },
        { "input": "GAMEPAD_RIGHT_TRIGGER", "action": "THRUST_FORWARD", "scale": 1.0 },
        // ... etc.
      ]
    },
    {
      "name": "menu_controls",
      "bindings": [
        { "input": "KEY_UP", "action": "UI_UP" },
        { "input": "KEY_DOWN", "action": "UI_DOWN" },
        { "input": "GAMEPAD_DPAD_UP", "action": "UI_UP" },
        // ... etc.
      ]
    }
  ]
}
```

### 4.3. System Integration

1.  **`InputService`**:
    -   Loads all Action Maps from the data file at initialization.
    -   Maintains a stack of active `ControlContexts`.
    -   When processing raw hardware events, it consults the Action Map for the currently active context to determine which `InputActionID` to emit.

2.  **`Controllable` Component**:
    -   This component will be simplified. It will no longer need to store complex axis configurations. Instead, it will simply flag an entity as "player-controllable."

3.  **`control_system`**:
    -   Its role remains largely the same: it queries the `InputState` (which is populated by the `InputService`) and applies the appropriate forces to the `ThrusterSystem` and other components. It remains blissfully unaware of the underlying hardware.

## 5. Implementation Roadmap for Sprint 25

The following is a detailed plan for the next sprint team to implement this vision.

1.  **Create `docs/manual/CONTROLS.md`**:
    -   **Action**: Create a new documentation file to serve as the single source of truth for all player controls.
    -   **Initial Content**: Document the currently planned flight controls (WASD, QE, Arrows).

2.  **Extend `InputActionID` Enum**:
    -   **File**: `src/services/input_service.h`
    -   **Action**: Add the new flight-related actions as detailed in the sprint plan.

3.  **Implement Data-Driven Action Maps**:
    -   **Action**: Create a new module (`src/services/input_action_maps.c/h`) responsible for loading and parsing the `action_maps.json` file.
    -   **Action**: Modify `InputService` to use this new module. The `input_service_setup_default_bindings()` function will be replaced with a call to load the action maps from the data file.

4.  **Refactor `input_state.c`**:
    -   **Action**: Update the `update_input_state()` function to correctly map the new `InputActionID`s to the `g_input_state` struct, as detailed in the sprint plan.

5.  **Update UI Hints**:
    -   **File**: `src/ui_adaptive_controls.c`
    -   **Action**: Add new `ControlHint` definitions for the flight controls (e.g., `UI_HINT_PITCH`, `UI_HINT_YAW`, `UI_HINT_ROLL`, `UI_HINT_THRUST`). These will be used to display on-screen help to the player.

6.  **Testing**:
    -   **Manual**: Thoroughly test the `flight_test_scene` to ensure all new controls work as expected.
    -   **Unit**: Implement the unit tests proposed in the sprint plan for `test_input_advanced.c` and `test_control.c`.

## 6. Conclusion

By completing the steps outlined in this document, the SIGNAL engine will have a best-in-class control system that is flexible, data-driven, and ready for future expansion. This will significantly improve the development workflow and the end-user experience.
