#include "cgame.h"
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

// Main game loop
int main(int argc, char* argv[]) {
    struct Game game;

    if (!init_game(&game)) {
        return 1;
    }

    printf("=== X-WING STARFIGHTER SIMULATOR ===\n");
    printf("Controls:\n");
    printf("  W/S: Throttle up/down\n");
    printf("  A/D: Strafe left/right\n");
    printf("  Space/C: Move up/down\n");
    printf("  Q/E: Roll left/right\n");
    printf("  R/F: Pitch up/down\n");
    printf("  T/G: Yaw left/right\n");
    printf("  Shift: Afterburner\n");
    printf("  Alt: Fire lasers\n");
    printf("  M: Toggle mouse capture\n");
    printf("  Mouse: Look around (when captured)\n");
    printf("  Close window to exit\n");
    printf("\nPress M to capture mouse for flight controls!\n");

    // Main game loop
    while (game.running) {
        handle_events(&game);
        update_game(&game);
        render_game(&game);
        
        // Cap the frame rate to approximately 60 FPS
        SDL_Delay(16);
    }

    cleanup_game(&game);
    return 0;
}
