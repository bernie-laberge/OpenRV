# OpenRV — AI Assistant Guidance

## About OpenRV

OpenRV is an image and sequence viewer for VFX and animation, developed under the
Academy Software Foundation (ASWF). It is a multi-platform, multi-language application
targeting **macOS, Windows, and Rocky Linux 8/9**.

## Technology Stack

### Core Application

- **C++17 / C++20**: Core application and plugins
- **Qt6**: UI framework
- **CMake 3.24+**: Build system
- **OpenGL 2.1 Legacy Profile**: Graphics rendering (NOT Core Profile)
- **FFmpeg**: Video codec support
- **OpenColorIO, OpenImageIO, OpenEXR**: Color management and image I/O

### Scripting & Automation

- **Python 3.11**: Build automation and plugins
- **Mu**: Custom scripting language for RV plugins (functional, Scheme-like with type annotations)

### Reference Platform

- **VFX Reference Platform**: CY2025
- Respect the `RV_TARGET_DARWIN`, `RV_TARGET_WINDOWS`, `RV_TARGET_LINUX` CMake variables for
  platform-specific code.

## Core Principles

1. **Be Conservative**: This is production software used by VFX studios. Prioritize stability and compatibility.
2. **Cross-Platform Awareness**: Always consider macOS, Windows, and Linux. Note or test platform-specific code.
3. **OpenGL 2.1 Legacy Profile**: Do NOT use Core Profile features (VAOs, DSA, Core-only shader
   requirements). Use the legacy fixed-function pipeline and GLSL 1.20 shaders.
4. **Ask Before Breaking Changes**: Discuss significant architectural changes before making them.
5. **Respect Legacy**: Some patterns may look outdated but exist for compatibility reasons.
6. **Surgical Code (KISS / YAGNI)**: Prefer the simplest solution that meets the requirement. Do not
   add functionality until it is necessary.
7. **Right Altitude for Logic**: Place behavior where it belongs architecturally, not where it is
   convenient. Test-only helpers must contain no production logic, and capabilities that should apply
   broadly must not be wired into a single special-case path.
8. **Security First**: Apply security best practices, especially around external input and process handling.
9. **Document Changes**: Update relevant README/docs when changing functionality.

### Upstream / Downstream

- OpenRV is the **open-source base**; commercial RV consumes it as a submodule.
- Keep OpenRV changes self-contained and independent of any commercial customization.
- Do NOT introduce commercial-only assumptions (Jenkins, AWS, ShotGrid, Live Review) into OpenRV code.

### Source Control (Git)

- Use `git log`, `git blame`, `git diff`, `git show` to understand *why* code changed and how the
  codebase evolved before making or recommending changes.
- **On Windows, run `git` (and all build commands) from the MSYS2-MinGW64 shell** — see
  "Build & Shell Environment" below. `git` is not on the default `cmd.exe`/PowerShell `PATH`; it is
  provided by MSYS2.

### Build & Shell Environment

- **macOS / Linux**: use a normal shell (bash/zsh). Build helpers live in `rvcmds.sh`
  (`rvcfg`, `rvbuild`, `rvinst`, …).
- **Windows**: all git and build operations **MUST** run from the **MSYS2-MinGW64** shell
  (`mingw64.exe`). The plain MSYS2 (`msys2.exe`) and MinGW32 (`mingw32.exe`) shells, as well as
  `cmd.exe`/PowerShell/WSL, are **not** supported for these tasks. MSYS2 is what supplies `git`,
  `cmake`, `flex`, `bison`, `nasm`, `zip`/`unzip`, etc. on `PATH`. Note: Open RV is still a
  **Microsoft Visual Studio 2022** build (CMake "Visual Studio 17 2022" generator) — it is **not** a
  MinGW build; MSYS2 is used only as the convenient shell/toolset host.
- Full Windows setup, including MSYS2 installation and required pacman packages, is documented in
  [`docs/build_system/config_windows.md`](docs/build_system/config_windows.md).

## Code Style & Formatting

All formatting is enforced via pre-commit hooks — generated code must pass these checks.

### C++ (C++17 minimum, prefer modern C++)

- **MUST** follow the repo-root `.clang-format`. Key rules:
  - Column limit: **140**
  - Indent width: **4 spaces** (never tabs)
  - Brace style: **Allman** (braces on their own line)
  - Pointer alignment: **Left** (`int* ptr`, not `int *ptr`)
  - Include sorting: **Never** (preserve manual ordering)
- Naming: Classes `PascalCase`; functions/methods `camelCase`; member variables `m_camelCase`;
  constants `UPPER_CASE` or `kPascalCase`.
- Headers: `#pragma once`. Group includes as system → third-party → project (do not auto-sort).
- Prefer `std::unique_ptr` / `std::shared_ptr` over raw pointers; follow RAII and const-correctness.
- Use Qt conventions (signals/slots, `Q_OBJECT`) for Qt-based code.
- Run `clang-format -i <file>` before committing.

### Python (Python 3.11)

- OpenRV uses **Ruff** for linting/formatting (line length **120**, per `ruff.toml`).
  (Note: the commercial parent repo uses Black; OpenRV does not.)
- Type hints where beneficial; Google-style docstrings; prefer f-strings.
- Group imports stdlib → third-party → local (PEP 8). Use context managers for file ops.

### Mu (RV scripting language)

- Functional language for mode files, menu customizations, and plugins; files end in `.mu`.
- Use explicit type annotations; follow existing OpenRV mode-file patterns; document public functions.
- **Prefer Python over Mu for new plugin development**; use Mu only when Python cannot do the job.

### CMake (require 3.24+)

- Auto-formatted via `cmake-format` (see `cmake-format.json`): line width **160**, indent **2 spaces**,
  UPPERCASE commands/keywords.
- Use modern CMake (targets, not variables); set properties via `target_*`; use
  `PRIVATE`/`PUBLIC`/`INTERFACE` appropriately; leverage modules in `cmake/`.

## Pre-Commit Hooks

```bash
pip install pre-commit       # one-time
pre-commit install           # install git hooks
pre-commit run --all-files   # run all hooks

# Specific hooks / files
pre-commit run clang-format --all-files
pre-commit run --files src/lib/MyLib/MyClass.cpp
```

Configured hooks (`.pre-commit-config.yaml`): `clang-format`, `ruff` (Python), `cmake-format`.

## Resources

- OpenRV Docs: <https://aswf-openrv.readthedocs.io/>
- VFX Reference Platform: <https://vfxplatform.com/>
- Build Instructions: see `README.md` and platform-specific docs
