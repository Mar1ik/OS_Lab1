# OS_Lab1

Educational OS lab with three small components:

- `shell/` - a tiny shell that demonstrates using `clone(2)` and manual stacks (`shell.c`).
- `cpu-calc-md5/` - CPU workload generators that produce random text and compute MD5 (`cpu-calc-md5.c`) and a simple multithreaded variant (`cpu-calc-md5-mt.c`). Requires OpenSSL (`-lcrypto`) and pthreads for the MT variant.
- `ema-replace-int/` - tools to generate integer-filled files (`ema-gen-data.c`) and search/replace integer values in-place (`ema-replace-int.c`).

Quick build (requires `gcc` and `libssl-dev`):

```bash
make
```

Run tests:

```bash
make test
```

Notes:
- The repository was reorganized: sources live under logical subdirectories now. Build targets are in the top-level `Makefile`.
- If you prefer CMake or CLion, remove the `Makefile` and restore `CMakeLists.txt` (previously present in the repo).
