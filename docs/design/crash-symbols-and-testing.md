# Design: Crash-reporting symbol management & tests

Status: **Done.** This document captured the follow-on work for the
crash-reporting capability and the exploration that informed it. All tasks are
implemented and verified on macOS, Linux, and Windows; the normative behavior
now lives in the source-of-truth spec [`crash-reporting.md`](../crash-reporting.md)
(§3, §7, §8). This file is retained as a design/decision record.

Implementation progress:

- [x] **Task 2b** — `symbolicate_crash.sh --symbols <dir>` override (verified on
      macOS).
- [x] **Task 1 + 2** — strip symbols from the customer (Release) package and the
      `symbols_archive` zip target (verified on macOS: 178 `.sym` entries
      stripped, archive symbolicates a real dump). **Verified on Windows** (VS
      2022 Release build, v3.1.0): the Release install contains zero `.pdb` files
      (and no `symbols/` tree) while `rvio` still runs from the prefix; the
      `symbols_archive` target emits `RV-3.1.0-windows-amd64-symbols.zip` under
      `stage/packages/` with 200 PDBs under `symbols/` (paths preserved, e.g.
      `symbols/bin/rv.pdb`, `rvio.pdb`, `TwkUtil.pdb`), matching the 200 PDBs in
      `stage/app`. (The strip is gated on `CMAKE_INSTALL_CONFIG_NAME ^Release$`,
      so a Debug install keeps its PDBs; only the Release path was exercised on
      this host.)
- [x] **Task 3** — automated tests. A doctest unit test (`src/test/CrashHandlerTest`,
      cross-platform, runs in CI) covering the CrashHandler singleton, the
      not-initialized contract, and the pre-init annotation buffering path; plus
      an end-to-end smoke test (`src/test/CrashDumpSmokeTest`) that launches RV
      with `crash()` and asserts a minidump is produced. It is enabled by default
      on every platform that builds the Crashpad handler (toggle with
      `RV_BUILD_CRASH_DUMP_SMOKE_TEST`); on macOS/Linux it also asserts the
      `platform` annotation via `minidump_dump`, while on Windows (no
      `minidump_dump`) it verifies only that a dump was produced. Verified on
      macOS. **Verified on Windows**: `CrashHandlerTest` builds and passes via
      `ctest` (3 cases / 9 assertions). No DLL-path fix was needed — RV stages the
      test exe into `stage/app/bin` alongside `TwkUtil.dll` and its dependency
      DLLs, which is the default Windows DLL search path, so the `LD_LIBRARY_PATH`
      that `ADD_TEST` sets (and Windows ignores) is harmless here.
- [x] Update `crash-reporting.md` §7/§8 — §7 now documents the strip + archive +
      `--symbols` symbolication flow, and §8 documents the doctest unit test and
      the smoke test (the stale "no unit-test harness" line is gone).

All planned work is implemented and verified on macOS, Linux, and Windows.

## Findings

- **Headless (`QT_QPA_PLATFORM=offscreen`) does not populate the Mu execution
  context.** Under offscreen Qt, a `crash()` dump has an empty `mu_function` /
  `mu_script_file`, whereas in a real display session (e.g. cocoa) `mu_function`
  is correctly set (verified: `boom` 2/2). `platform` and the Python context are
  present in both modes. Because production RV always runs with a real display,
  this is a headless-test limitation rather than a user-facing bug, so the
  headless smoke test asserts only the always-present `platform` annotation and
  reports `mu_function` for information. Validating the Mu context (C2)
  end-to-end requires a display-backed run. Investigating why the
  `MuCrashObserver` hook is inactive under offscreen is optional follow-up.

## Context

The crash-reporting capability (branch `add_crash_dump_capability`) is complete
and converged — audit items A–G plus D4 are all resolved against
`docs/crash-reporting.md`. Three follow-on tasks remain to make the *symbol*
side production-grade:

1. **Prune symbol files in the install step.** The symbols tree bloats over
   rebuilds and currently ships in full to customers, who never symbolicate.
2. **Save/archive symbol files** so a customer-provided `.dmp` can be
   symbolicated later (via `symbolicate_crash.sh`) to identify the crash source.
3. **Add simple automated tests** to validate and maintain the basic crash-dump
   functionality so it does not silently regress.

## Current state (from code exploration)

### Symbol generation & install pipeline

- Per-target `POST_BUILD`, `cmake/macros/rv_generate_symbols.cmake` runs
  `dsymutil` (macOS) → `dump_syms` → `organize_symbols.sh`, placing each `.sym`
  under `${CMAKE_BINARY_DIR}/stage/app/symbols/<MODULE_NAME>/<MODULE_ID>/`. It is
  invoked from `rv_stage.cmake:80` for binary target types — ~48 call sites.
- `organize_symbols.sh` uses `mkdir -p` + `cp` and **accumulates** a fresh
  `<MODULE_ID>` directory on every rebuild; old build-ids are never removed.
  A live debug tree measured **824 MB across 178 `.sym` files** for only ~48–70
  modules — i.e. most of the bulk is stale duplicate build-ids.
- Install is the `rvinst` alias (`rvcmds.sh:424` →
  `cmake --install ${RV_BUILD_DIR} --prefix ${RV_INST_DIR}`). The copy is a
  blind `FILE(GLOB_RECURSE ...)` of `RV_APP_ROOT` (= `stage/app`) in
  `cmake/install/install.cmake:12-17`, filtered per-file by `BEFORE_COPY`
  (`cmake/install/pre_install*.cmake`). **There is no symbol-specific install
  rule** — `symbols/` is copied wholesale because it sits directly under
  `stage/app/`.
- There is no CPack step; install is a flat file copy + rpath fixup. The symbols
  dir is a *peer* of `RV.app/` on macOS and of `bin/` on Linux. Windows generates
  no Breakpad symbols (native PDBs are used instead — by design).

### Symbolication

- `src/bin/apps/rv/symbolicate_crash.sh` resolves `SYMBOLS_DIR` *relative to its
  own location* (`../symbols` on Linux from `bin/`; `../../../symbols` on macOS
  inside the bundle) and runs `minidump_stackwalk <dmp> <symbols_dir>`, then
  enriches the report with GPU info + annotations via `minidump_dump`. The
  symbols path is **hardcoded relative** — it breaks if symbols are not shipped
  beside the script (as they would not be, if pruned from the customer install).

### Test harness (correction to the spec)

The spec §8 currently says "OpenRV has no unit-test harness for this subsystem."
That is **inaccurate**: a real harness exists and should be used.

- **doctest + CTest** are wired and run in CI on all three platforms after every
  build (`.github/actions/build-*/action.yml` → `ctest --test-dir _build`).
- Pattern to follow: `src/test/IPCore/ApplicationTest` and `AudioRendererTest`
  (doctest; the latter even links `Mu`). On macOS only `FastMemcpyTest` and
  `QFontTest` register; the IPCore tests are Linux-only
  (`src/test/CMakeLists.txt:9-14`).
- Mu tests: ~100 `.mu` scripts run via `mu-interp -main <script>` (exit code =
  pass/fail using `assert()`). Python tests run via `py-interp -m unittest`.
- `crash()` is defined in `src/lib/ip/IPMu/CommandsModule.cpp` and registered
  through `Rv::initCommands`, so it is **not** available in the plain
  `mu-interp`. `rvio` has it but exposes no `-eval`. `rv -eval 'crash();'` works
  (RV is a GUI app; the crash kills it). The dump directory is overridable via
  the env var `RV_CRASH_DUMPS_DIR` (`CrashHandler.cpp:31`) — useful to redirect
  dumps to a temp dir under test.

## Resolved decisions

1. **Prune scope — strip symbols from the customer-facing package on all three
   platforms.** Customers never symbolicate; symbols live only in the archive
   (decision 2). This covers the macOS/Linux Breakpad `.sym` tree
   (`stage/app/symbols/`) *and* the Windows PDB files (PDBs are generated
   specifically for crash-dump symbolication — `cxx_msvc_defaults.cmake:63`,
   `src/bin/apps/rv/CMakeLists.txt:130`).
2. **Archive form — a versioned per-platform zip,
   `RV-<version>-<platform>-symbols.zip`** (`RV_MAJOR/MINOR/REVISION` from
   `CMakeLists.txt:12-14`), built portably with `cmake -E tar ... --format=zip`
   (works on Windows too), emitted at a deterministic build path. The zip
   bundles the platform's symbols (`.sym` tree on macOS/Linux, PDBs on Windows)
   plus `minidump_stackwalk` + `symbolicate_crash.sh` so it is self-contained.
   - **OpenRV ↔ Jenkins boundary (upstream rule).** OpenRV ONLY strips the
     customer package and produces the zip artifact. The Jenkins build (whose
     Jenkinsfile lives in a *different* repo) picks up that zip and uploads it to
     Artifactory. **No Jenkins / Artifactory / AWS logic belongs in OpenRV** —
     this keeps the open-source base free of commercial-only assumptions
     (CLAUDE.md). OpenRV's contract is just "a predictably-named zip at a known
     path."
3. **`symbolicate_crash.sh --symbols <dir>` — yes.** Add an explicit override
   (the current relative path stays as the fallback) so an unpacked archive can
   be pointed at when symbolicating a customer dump on a dev machine. This is
   effectively required now, since symbols no longer ship beside the script.
4. **Test scope — both unit and smoke.** A doctest unit test for the
   CrashHandler annotation table (contract C3), pre-init buffering/flush (item
   G), and the unmapped-key warning (cross-platform, CI-friendly, mirrors
   `src/test/IPCore/ApplicationTest`), PLUS a lightweight subprocess smoke test
   that triggers `crash()` and asserts a dump with the expected annotations.

## Implementation approach

### Task 1 — strip customer symbols (all platforms)

- macOS/Linux: stop the blind glob from shipping `stage/app/symbols/`. Add a
  symbol-aware exclusion in `BEFORE_COPY` (`cmake/install/pre_install*.cmake`) or
  a pre-install prune so the installed package contains no `.sym` tree.
- Windows: enable the existing (currently commented-out) `.pdb` strip filter in
  `cmake/install/pre_install_windows.cmake:8` for Release installs.
- Order matters: the archive (task 2) must be produced from the staged symbols
  **before** they are stripped from the install.

### Task 2 — archive symbols as a versioned zip

- A new `cmake/macros/rv_archive_symbols.cmake` (or an install/package step)
  collects the platform's symbols into `RV-<version>-<platform>-symbols.zip` via
  `cmake -E tar cf <zip> --format=zip ...`, written to a known build/packages
  path (near `RV_PACKAGES_DIR`).
- Bundle `minidump_stackwalk` + `symbolicate_crash.sh` (macOS/Linux) so the zip
  is self-contained for offline symbolication.
- Dedup naturally falls out of archiving a fresh per-build symbol set rather than
  the accumulated tree (consider a clean step or selecting only current
  build-ids).

### Task 2b — `symbolicate_crash.sh --symbols <dir>`

- Parse an optional `--symbols <dir>`; default to the existing OS-relative path.
  Keep all other behavior identical.

### Task 3 — tests

- Unit: new `src/test/.../CrashHandlerTest/` (doctest), wired into
  `src/test/CMakeLists.txt`, asserting (a) every key in `g_annotationMappings[]`
  round-trips, (b) an unmapped key is rejected/warned, (c) pre-init
  `addAnnotation()` is buffered and flushed on `initialize()` (item G).
- Smoke: a CTest `ADD_TEST` script that launches the app with `crash()` (env
  `RV_CRASH_DUMPS_DIR` → temp dir), asserts a `.dmp` is produced and that
  `minidump_dump` shows the expected annotations. Gate by platform/headless
  availability as the existing IPCore tests do.

### Suggested sequencing

Implement and verify the macOS/Linux-testable parts here (strip + archive on
mac/Linux, `--symbols`, unit + smoke tests); verify the Windows PDB strip +
archive on a Windows host (as was done for audit item D4).

## Candidate files to touch (once decisions land)

- **Prune:** a dedup pass in `organize_symbols.sh` (build-time) and/or a
  pre-install filter in `cmake/install/install.cmake` /
  `cmake/install/pre_install_*.cmake`.
- **Archive:** a `cmake/install/` post-install script and/or a new
  `cmake/macros/rv_archive_symbols.cmake`; CI `.github/actions/build-*`.
- **Symbolicate:** `src/bin/apps/rv/symbolicate_crash.sh`.
- **Tests:** new `src/test/.../CrashHandlerTest/` (doctest) wired into
  `src/test/CMakeLists.txt`; a smoke test registered as a CTest `ADD_TEST`
  script.
- **Docs:** `docs/crash-reporting.md` §7 (symbol workflow) and §8 (testing —
  correct the "no harness" note), cross-referencing this design once settled.

## Verification outline (to expand per task)

- **Prune / archive:** run `rvinst`; confirm the installed symbols size dropped
  (or is absent); unpack the archive and confirm its layout; symbolicate a real
  `.dmp` against the unpacked archive end-to-end.
- **Tests:** `ctest --test-dir _build -R CrashHandler --extra-verbose` green on
  macOS and Linux; the smoke test produces and validates a dump (annotations +
  symbolicated native stack).
