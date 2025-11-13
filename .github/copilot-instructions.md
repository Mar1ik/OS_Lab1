# Copilot / Agent Instructions for OS_Lab1

This file captures the essential, discoverable knowledge an AI coding agent needs to be immediately productive in this repository.

## Big picture
- Purpose: an educational OS lab containing three main components: a minimal shell (`shell.c`), a CPU-load MD5 generator (`cpu-calc-md5_*`), and an EMA-style integer file generator/patcher (`ema-replace-int_*`).
- Two build approaches are present and diverge: a CMake configuration that builds a single combined executable, and a `Makefile` that expects a different directory layout (subdirectories `shell/`, `cpu-calc-md5/`, `ema-replace-int/`). See `CMakeLists.txt` and `Makefile`.

## Key files to read first
- `CMakeLists.txt` — current canonical build used by the dev environment (builds `untitled10`).
- `Makefile` — contains useful `LDFLAGS` and a `test` target, but it references subdirectories that are not present in this workspace (likely older layout).
- `shell.c` — simple shell implementation using `clone(2)` with a manually allocated stack and sequential `;`-separated command handling.
- `cpu-calc-md5_cpu-calc-md5_Version1.c` and `cpu-calc-md5_cpu-calc-md5-mt_Version1.c` — CPU workload generators that use OpenSSL MD5 (`-lcrypto`) and random fragment generation.
- `ema-replace-int_ema-replace-int_Version1.c` and `ema-replace-int_ema-gen-data_Version1.c` — file I/O workloads that open files with `O_RDWR` and operate on integer buffers.

## Build & run (concrete commands)
- Prefer the CMake flow that's consistent with the current files:
  - `cmake -S . -B cmake-build-debug`
  - `cmake --build cmake-build-debug`
  - Run the combined binary: `./cmake-build-debug/untitled10`
- If you need to build artifacts referenced in the `Makefile`, note the path mismatch: `make` may fail because `shell/`, `cpu-calc-md5/`, and `ema-replace-int/` folders are not present.
- Single-file quick-build examples (useful for iterative edits):
  - `gcc -std=c11 -Wall -Wextra -D_GNU_SOURCE shell.c -o shell`
  - `gcc -std=c11 -Wall -Wextra -D_GNU_SOURCE cpu-calc-md5_cpu-calc-md5_Version1.c -o cpu-calc-md5 -lcrypto`
  - `gcc -std=c11 -Wall -Wextra -D_GNU_SOURCE ema-replace-int_ema-replace-int_Version1.c -o ema-replace-int`

## Tooling & external deps
- Linking needs OpenSSL crypto: install system dev package (`libssl-dev` on Debian/Ubuntu) to satisfy `-lcrypto`.
- Some components expect pthreads (`-lpthread`) for multithreaded variants.
- The container has a `cmake-build-debug/` folder created by CLion — use it or create a new `build/` folder.

## Project-specific conventions & gotchas
- Filenames include generated/version suffixes like `_Version1` and concatenated module names (e.g. `ema-replace-int_ema-gen-data_Version1.c`). Preserve these when refactoring unless intentionally normalizing the tree.
- Comments are in Russian in places — be attentive to intent and semantics when changing behavior.
- `Makefile` and `CMakeLists.txt` reflect different layouts: do not assume `make` is authoritative. Prefer CMake for reproducible builds in this workspace.
- Low-level APIs: the code uses `clone(2)` with a manually allocated stack in `shell.c`, raw `open/read/write/lseek` for large-file I/O in EMA tools, and OpenSSL APIs for MD5. Changes must respect those low-level semantics.

## Safety and correctness checks
- Watch for manual memory management (malloc/free) and stack allocation for `clone` — validate with AddressSanitizer / valgrind when changing allocation lifetimes.
- File I/O operates on integer buffers and fixed `BUFFER_SIZE` — ensure correct alignment and use of `ssize_t`/`off_t` when modifying loops.

## How to make non-invasive changes
- For small refactors, compile the single source with the quick-build commands above and run smoke examples.
- If you reorganize files into the `shell/`, `cpu-calc-md5/`, `ema-replace-int/` layout to satisfy the `Makefile`, update `CMakeLists.txt` (or vice versa) and keep `LDFLAGS` (`-lcrypto -lpthread`) consistent.

## Examples of patterns to follow
- Timing instrumentation: functions `get_time_us()` are used for microsecond timing — reuse this helper rather than adding new timing APIs.
- Progress output: workload tools print periodic progress (iterations/throughput). Preserve stdout formats if tests or scripts parse them.

If any of these sections are unclear or you want the agent to prefer a different flow (e.g., convert the Makefile to match current files), tell me which approach you prefer and I will update this file accordingly.
