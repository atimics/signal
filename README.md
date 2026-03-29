# Sokol Space Miner

A small C + Sokol prototype with a simple mining loop:

- Fly a ship with `W/S` and `A/D`
- Line up asteroids and hold `Space` to mine ore
- Return to the station ring and press `E` to sell cargo

## Build

Native desktop:

```sh
cmake -S . -B build
cmake --build build
./build/space_miner
```

Browser / WebAssembly with Emscripten:

```sh
emcmake cmake -S . -B build-web
cmake --build build-web
```

That produces `build-web/space_miner.html` plus the `.js` and `.wasm` files.

## Notes

- The game is set up to stay asset-free: geometry and HUD text are drawn directly with Sokol.
- Native builds use Metal on macOS, OpenGL on Linux, and OpenGL on Windows through Sokol.
- The browser target uses WebGL 2 via Emscripten.
