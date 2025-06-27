#include "cgame.h"
#include <stdlib.h>
#include <string.h>

struct Mesh starship_mesh;

void init_meshes() {
    starship_mesh.num_vertices = 18;
    starship_mesh.vertices = malloc(sizeof(Vector3D) * starship_mesh.num_vertices);
    starship_mesh.vertices[0] = (Vector3D){0, 0, 2};
    starship_mesh.vertices[1] = (Vector3D){0.5, 0.2, -1};
    starship_mesh.vertices[2] = (Vector3D){-0.5, 0.2, -1};
    starship_mesh.vertices[3] = (Vector3D){0, -0.5, -1};
    starship_mesh.vertices[4] = (Vector3D){1, 0, -1};
    starship_mesh.vertices[5] = (Vector3D){-1, 0, -1};
    starship_mesh.vertices[6] = (Vector3D){0, 1, -1};
    starship_mesh.vertices[7] = (Vector3D){0, 0, -1.5};
    starship_mesh.vertices[8] = (Vector3D){2, 1, -1.5};
    starship_mesh.vertices[9] = (Vector3D){-2, 1, -1.5};
    starship_mesh.vertices[10] = (Vector3D){0, -1, -1.5};
    starship_mesh.vertices[11] = (Vector3D){2, -1, -1.5};
    starship_mesh.vertices[12] = (Vector3D){-2, -1, -1.5};
    starship_mesh.vertices[13] = (Vector3D){1.5, 1.5, -1.5};
    starship_mesh.vertices[14] = (Vector3D){-1.5, 1.5, -1.5};
    starship_mesh.vertices[15] = (Vector3D){1.5, -1.5, -1.5};
    starship_mesh.vertices[16] = (Vector3D){-1.5, -1.5, -1.5};
    starship_mesh.vertices[17] = (Vector3D){0, 0, -3};

    starship_mesh.num_edges = 36;
    starship_mesh.edges = malloc(sizeof(int) * starship_mesh.num_edges * 2);
    int edges[][2] = {
        {0, 1}, {0, 2}, {0, 3}, {1, 2}, {2, 3}, {3, 1},
        {1, 4}, {2, 5}, {3, 6}, {4, 5}, {5, 6}, {6, 4},
        {4, 8}, {5, 9}, {6, 7}, {8, 9}, {9, 7}, {7, 8},
        {8, 13}, {9, 14}, {10, 15}, {11, 16}, {12, 17},
        {13, 14}, {14, 12}, {12, 13}, {15, 16}, {16, 17}, {17, 15},
        {13, 15}, {14, 16}, {8, 11}, {9, 12}, {10, 7}, {11, 10}
    };
    memcpy(starship_mesh.edges, edges, sizeof(edges));
}
