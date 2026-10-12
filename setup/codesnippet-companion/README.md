# CodeSnippet Companion (VS Code / Antigravity IDE)

An IDE companion extension designed specifically for the `codesnippet` LaTeX package.

## Features

- **Copy Semantic Symbol Path (`Ctrl+Alt+S` / `Cmd+Alt+S`)**:
  - Right-click anywhere in C++ code to get `Matrix::Matrix`, `Matrix::rows`, `Matrix::operator()`, `Matrix`, or `main`.
  - Right-click in Python code to get `MyClass.method`, `MyClass`, or `function_name`.
  - If a multi-line range is selected, automatically copies the line range `25-36`.
  - Formatted exactly as expected by `extract.py` and `codesnippet.sty`.

- **Copy `\codesnippet` LaTeX Macro (`Ctrl+Alt+C` / `Cmd+Alt+C`)**:
  - Automatically calculates the relative file path and produces:
    ```latex
    \codesnippet{src/Matrix.cpp}{Matrix::multiply_blas}
    ```
  - When inside project subfolders (e.g. `Project01/`), resolves cleanly relative to the project root so it fits `root = ..` setups out of the box.

- **Copy Detailed `\codesnippet` Macro with Title & Label**:
  - Generates full snippet definition with `title` and `label` for instant cross-referencing:
    ```latex
    \codesnippet[
        title = {Matrix::multiply_blas},
        label = snip:matrix_multiply_blas
    ]{src/Matrix.cpp}{Matrix::multiply_blas}
    ```

- **Copy Real Location**:
  - Copies `src/Matrix.cpp:138-163`.
