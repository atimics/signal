#ifndef MESH_H
#define MESH_H

#include "math3d.h"

struct Mesh {
    Vector3D* vertices;
    int num_vertices;
    int* edges;
    int num_edges;
};

extern struct Mesh starship_mesh;


void init_meshes();

#endif
