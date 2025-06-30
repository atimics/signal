# Sprint 16 Research: Asset Pipeline Refactor

This document contains the research and best practices for refactoring the asset pipeline to be non-destructive and artist-friendly.

## Research Request

Please investigate the following areas to ensure best practices for this refactor:

### Asset Pipeline Design
1.  **Non-Destructive Workflows**: What are the core principles of a non-destructive asset pipeline?
    *   **Best Practice**: The pipeline should treat source assets as immutable. It should read from the source, process the data, and write to a separate build directory. It should never modify the source assets.
2.  **Artist-Friendly Tooling**: How can we make the pipeline more artist-friendly?
    *   **Best Practice**: The pipeline should prioritize artist-created data. If an artist has provided a texture, material, or UV map, the pipeline should use it. Procedural generation should only be a fallback for missing data.
    *   **Resource**: [Game Asset Pipelines (Gamasutra)](https://www.gamasutra.com/view/feature/131457/the_game_asset_pipeline.php)

### Python Best Practices
1.  **Path Handling**: What is the modern way to handle file paths in Python?
    *   **Best Practice**: Use the `pathlib` module for all file system operations. It provides an object-oriented interface for file paths and is generally safer and more readable than using string manipulation.
    *   **Resource**: [pathlib — Object-oriented filesystem paths](https://docs.python.org/3/library/pathlib.html)
2.  **Dependency Management**: How should we manage Python dependencies?
    *   **Best Practice**: Use a `requirements.txt` file to list all Python dependencies. This makes it easy for other developers to set up the required environment using `pip install -r requirements.txt`.

### 3D Graphics Programming
1.  **Mesh Data**: What is the standard way to represent mesh data?
    *   **Best Practice**: A mesh is typically represented by a list of vertices and a list of faces (indices). Each vertex can have multiple attributes, such as position, normal, and UV coordinates.
    *   **Resource**: [Wavefront .obj file (Wikipedia)](https://en.wikipedia.org/wiki/Wavefront_.obj_file)
2.  **UV Mapping**: How does UV mapping work?
    *   **Best Practice**: UV mapping is the process of assigning a 2D texture coordinate to each vertex of a 3D model. This allows a 2D texture to be "wrapped" around the 3D model.
    *   **Resource**: [UV mapping (Wikipedia)](https://en.wikipedia.org/wiki/UV_mapping)
