# Good First Issues

Welcome to the CGame engine project! We're excited that you're interested in contributing. This document lists a few issues that are great for new contributors to tackle. They are well-defined, self-contained, and a great way to get familiar with the project.

## 1. Add a New Procedural Mesh

**Goal**: Add a new procedurally generated mesh to the asset pipeline.

**Description**: The CGame engine has a powerful asset pipeline that can procedurally generate meshes. Your task is to add a new mesh to this pipeline. You can be as creative as you like with the mesh design, but a simple geometric shape (e.g., a torus, a pyramid, or a dodecahedron) is a great place to start.

**Steps**:

1.  **Fork the repository** and create a new branch for your feature.
2.  **Read the asset pipeline documentation** to understand how to add a new mesh: [docs/ASSET_PIPELINE.md](../docs/ASSET_PIPELINE.md)
3.  **Add a new mesh generator function** to `tools/mesh_generator/primitives.py`. This function should return the vertices, faces, and UV coordinates for your new mesh.
4.  **Register your new mesh** in the `MESH_GENERATORS` and `MESH_METADATA` dictionaries in `tools/clean_asset_pipeline.py`.
5.  **Generate the new mesh** by running the asset pipeline script: `python3 tools/clean_asset_pipeline.py --mesh your_mesh_name`
6.  **Test your new mesh** by running the engine and ensuring that it renders correctly.
7.  **Submit a pull request** with your changes.

## 2. Add a New Unit Test

**Goal**: Add a new unit test to the project's test suite.

**Description**: The CGame engine has a growing test suite to ensure code quality and prevent regressions. Your task is to add a new unit test for one of the core engine systems. A good place to start would be to add a new test for the math library (`src/core/math.c`).

**Steps**:

1.  **Fork the repository** and create a new branch for your feature.
2.  **Review the existing tests** in the `tests/` directory to understand the testing framework.
3.  **Add a new test case** to `tests/test_core_math.c`. This test should verify the correctness of one of an existing math function (e.g., `vec3_add`, `mat4_translate`, etc.) or a new function you add.
4.  **Run the test suite** by running `make test` and ensure that all tests pass.
5.  **Submit a pull request** with your changes.

## 3. Improve the Documentation

**Goal**: Improve the project's documentation.

**Description**: We're always looking to improve our documentation. If you find a section that is unclear, incomplete, or could be improved in any way, please feel free to submit a pull request with your changes.

**Steps**:

1.  **Fork the repository** and create a new branch for your changes.
2.  **Make your changes** to the documentation files in the `docs/` directory.
3.  **Submit a pull request** with your changes.

---

If you have any questions or need help with any of these issues, please don't hesitate to open an issue on GitHub. We're here to help!
