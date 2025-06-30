# Research Brief: High-Performance Binary Asset Format (.cobj)

**ID**: `R15_Binary_Asset_Format`
**Author**: Gemini (Lead Scientist and Researcher)
**Status**: Draft
**Related Sprint**: 12.0 (Mesh Rendering Stabilization)

## 1. Overview

This document presents a research-driven proposal to evolve the `.cobj` asset format from its current text-based representation into a true, pre-compiled binary format. The current approach, which uses a text-based `.obj` file renamed to `.cobj`, requires expensive and slow parsing at runtime. A binary format will allow the engine to load mesh data directly into GPU-ready structures with minimal processing, resulting in significantly faster load times and a more efficient rendering pipeline.

## 2. Analysis of the Current System

The current asset loading process, centered around `parse_obj_file` in `src/asset_loader/asset_loader_mesh.c`, has several critical inefficiencies:

*   **High Parsing Overhead**: The function relies on `fgets` to read the file line-by-line and `sscanf` to parse text into floating-point numbers. This is extremely slow compared to reading binary data.
*   **Multi-Pass Reading**: The file is read *three times*: once to count elements, once to allocate memory, and a final time to parse and populate the data. This is a major I/O bottleneck.
*   **Redundant Data**: The text representation of numbers (e.g., `"0.707107"`) is much larger than its 4-byte binary `float` equivalent, leading to larger file sizes.
*   **No Pre-computation**: Important data required for rendering, such as tangents and bounding boxes, cannot be stored and must be calculated at runtime (or are currently omitted), further slowing down the loading process.

## 3. Proposed Binary `.cobj` Format Specification

The new `.cobj` format will be a tightly packed binary file that can be read directly into C structs. This eliminates runtime parsing entirely.

### 3.1. File Structure

The file will consist of a header followed immediately by the raw vertex and index data.

```
[ COBJ Header (struct COBJHeader) ]
[ Vertex Data (Vertex[])          ]
[ Index Data  (uint32_t[])        ]
```

### 3.2. Header Definition

The header contains all the metadata needed to load the mesh.

```c
// This struct defines the binary layout of the header
typedef struct {
    // File Identifier
    char     magic[4];              // "CGMF" (CGame Mesh File)
    uint32_t version;               // e.g., 1

    // Mesh Counts
    uint32_t vertex_count;
    uint32_t index_count;

    // Bounding Volume (for culling)
    Vector3  aabb_min;
    Vector3  aabb_max;

    // Future-proofing / Padding
    uint32_t reserved[8];

} COBJHeader;
```

*   **`magic`**: A 4-byte "magic number" to quickly identify the file as a CGame Mesh. This prevents the engine from trying to load incorrect file types.
*   **`version`**: A version number for the format itself. If we change the format in the future, the engine can identify older versions and handle them appropriately.
*   **`vertex_count`, `index_count`**: The number of vertices and indices in the mesh. This allows us to allocate the exact amount of memory required with a single `malloc` call.
*   **`aabb_min`, `aabb_max`**: The axis-aligned bounding box of the mesh, pre-calculated by the asset compiler. This is essential for implementing view frustum culling.

### 3.3. Data Layout

*   **Vertex Data**: Immediately following the header, the file will contain `vertex_count` instances of the `Vertex` struct, written as a raw binary blob.
*   **Index Data**: Immediately following the vertex data, the file will contain `index_count` instances of a `uint32_t` index.

## 4. Actionable Implementation Plan

This transition requires a two-phase implementation: modifying the Python compiler and then updating the C engine loader.

### Phase 1: Python Asset Compiler (`asset_compiler.py`)

The asset compiler will be responsible for performing all the heavy lifting offline.

1.  **Read Source Mesh**: Parse the source `.obj` file as is currently done.
2.  **Generate Tangents**: For each vertex, calculate the tangent and bitangent vectors. The **MikkTSpace** algorithm is the industry standard and has Python implementations available. This is a crucial step for normal mapping in a PBR pipeline.
3.  **Optimize Mesh (Optional but Recommended)**: Before saving, process the mesh to improve runtime GPU performance.
    *   **Vertex Cache Optimization**: Re-order the index buffer to take advantage of the GPU's post-transform vertex cache. Libraries like `py-mesh-triangle-optimizer` can be used.
4.  **Calculate Bounding Box**: Iterate through all vertex positions to find the minimum and maximum extents (the AABB).
5.  **Pack and Write Binary Data**:
    *   Use Python's `struct` module to pack the `COBJHeader` into a binary format.
    *   Write the header to the output `.cobj` file.
    *   Iterate through the vertices and write the vertex data (position, normal, tangent, tex_coord) as a contiguous block of binary data.
    *   Write the (optimized) index data as a final contiguous block.

### Phase 2: C Engine Loader (`src/asset_loader/asset_loader_mesh.c`)

The C loader becomes dramatically simpler and faster. The `parse_obj_file` function will be replaced with a new `load_cobj_binary` function.

1.  **Open File**: Open the `.cobj` file in binary read mode (`"rb"`).
2.  **Read and Validate Header**:
    *   Read the size of the `COBJHeader` into a `COBJHeader` struct using a single `fread`.
    *   Check if `header.magic` is equal to `"CGMF"`. If not, it's an invalid or old file.
    *   Check `header.version` to ensure compatibility.
3.  **Allocate Memory**:
    *   Allocate `header.vertex_count * sizeof(Vertex)` for the vertices.
    *   Allocate `header.index_count * sizeof(uint32_t)` for the indices.
4.  **Read Data Blobs**:
    *   Use a single `fread` to read all vertex data directly into the allocated vertex buffer.
    *   Use a second `fread` to read all index data directly into the allocated index buffer.
5.  **Close File**: The mesh is now loaded.

## 5. Benefits of the Binary Format

*   **Load Speed**: Eliminating text parsing and multi-pass file reading will make mesh loading **10-100x faster**. The process will be limited only by disk I/O speed.
*   **File Size**: Binary floating-point numbers and integers are significantly smaller than their text representations, reducing the overall size of the game's assets.
*   **Reduced Complexity**: The C loading code becomes simpler, more robust, and easier to maintain. All the complex parsing logic is moved to the offline Python toolchain.
*   **Runtime Performance**: Pre-calculating tangents and bounding boxes means less work for the engine to do when an object is loaded or rendered for the first time.

This transition is a critical step in professionalizing the CGame engine's architecture and is a prerequisite for building a high-performance rendering system.
