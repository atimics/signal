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
