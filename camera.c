#include "cgame.h"

// Screen dimensions (should match main.c)
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

// 3D to 2D projection with rotation
Point2D project_3d_to_2d(Vector3D point, struct Camera* camera) {
    // Translate relative to camera
    Vector3D relative = {
        point.x - camera->position.x,
        point.y - camera->position.y,
        point.z - camera->position.z
    };
    
    // Apply camera rotation (inverse rotation for world-to-camera transform)
    relative = rotate_z(relative, -camera->rotation.z);
    relative = rotate_y(relative, -camera->rotation.y);
    relative = rotate_x(relative, -camera->rotation.x);
    
    // Perspective projection
    float distance = relative.z;
    if (distance <= 0.1f) distance = 0.1f; // Prevent division by zero
    
    float scale = camera->fov / distance;
    
    Point2D result;
    result.x = (int)(relative.x * scale + SCREEN_WIDTH / 2);
    result.y = (int)(-relative.y * scale + SCREEN_HEIGHT / 2); // Flip Y for screen coordinates
    
    return result;
}

// Initialize camera
void init_camera(struct Camera* camera) {
    camera->position = (Vector3D){0, 0, -50};
    camera->rotation = (Vector3D){0, 0, 0};
    camera->fov = 500.0f;
    camera->near = 0.1f;
    camera->far = 1000.0f;
}

// Update camera to follow spaceship
void update_camera(struct Camera* camera, struct Spaceship* player) {
    // Update camera to follow player with rotation
    camera->position.x = player->position.x;
    camera->position.y = player->position.y;
    camera->position.z = player->position.z - 60;
    
    // Camera inherits player rotation for cockpit view
    camera->rotation = player->rotation;
}
