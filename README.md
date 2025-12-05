# bignum-cmp

[![C/ASM CI](https://github.com/kirill-bayborodov/bignum-cmp/actions/workflows/ci.yml/badge.svg)](https://github.com/kirill-bayborodov/bignum-cmp/actions/workflows/ci.yml)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/kirill-bayborodov/bignum-cmp?label=release)](https://github.com/kirill-bayborodov/bignum-cmp/releases/latest)


`bignum-cmp` is a high-performance, standalone module for performing a logical compare an arbitrary-precision integers (`bignum_t`).
A highly optimized x86-64 assembly implementation of a big numbers compare operation, designed for performance-critical applications. 

## Distribution

Part of the `bignum-lib` project: https://github.com/kirill-bayborodov/bignum-lib  
Also available as a standalone distribution.

## Features

-   **High Performance:** Hand-crafted x86-64 yasm assembly — an ultra-optimized, multithreading-ready engine delivering peak execution speed.
-   **Dependency-Free Core:** The core logic has no external runtime dependencies.
-   **Tests and Benchmarks:** Provides a comprehensive test suite and performance microbenchmarks.
-   **Automated Builds:** A comprehensive `Makefile` for easy compilation, testing, and benchmarking.
-   **Continuous Integration:** All changes are automatically built and tested via GitHub Actions.
-   **Static Analysis:** Code quality is enforced using `cppcheck` for all C-based test files.

## Dependencies

-   **Build-time:** `make`, `gcc`, `yasm`, `cppcheck`.
-   **Component:** This project requires `bignum-common` as a git submodule located at `libs/bignum-common`.

To clone the repository with its submodule, use:
```bash
git clone --recurse-submodules https://github.com/kirill-bayborodov/bignum-cmp.git
```
## API

The library provides a single function, declared in `include/bignum_cmp.h`.

```c
bignum_cmp_status_t bignum_cmp(const bignum_t *a, const bignum_t *b);
```
-   **`a`**: A pointer to the `bignum_t` structure to be compared (left operand).
-   **`b`**: A pointer to the `bignum_t` structure to be compared (right operand).
-   **Returns**: A `bignum_cmp_status_t` enum (`BIGNUM_CMP_GREATER`, `BIGNUM_CMP_EQ`, `BIGNUM_CMP_LESS`, `BIGNUM_CMP_ERROR_NULL`).

## How to Build, Test, Install and Use

The project uses a `Makefile` to manage all tasks.

### Build the code
Builds the assembly object file.
```bash
make build CONFIG=release
```

### Run Unit Tests
Compiles and runs fast, essential correctness tests.
```bash
make test CONFIG=release
```

### Run Static Analysis
Checks all C source files (`tests/`, `benchmarks/` and `dist/`) for potential bugs and style issues.
```bash
make lint
```

### Run Performance Benchmarks
Compiles and runs performance tests using `perf`. The txt report is saved to `benchmarks/reports/check_*.txt`.
```bash
make bench CONFIG=debug
```

### Build the distributive
Builds the installation pack of files (with objects .o file) in dist direstory.
```bash
make install CONFIG=release
```

### Build the distributive
Builds the distributive pack of files (with single-header and static library .a file).
```bash
make dist CONFIG=release
```

## Clean Up

To remove all generated files (object files, executables, reports ):
```bash
make clean
```

## How to Use

This project produces an object file (`bignum_cmp.o`) which you can link with your own application.

**1. Clone the repository with submodules:**
```bash
git clone --recurse-submodules https://github.com/kirill-bayborodov/bignum-cmp.git
cd bignum-cmp
```

**2. Build the object file:**
```bash
make build
```
The output will be located at `build/bignum_cmp.o`.

**3. Link with your application:**
When compiling your project, include the object file and specify the include paths for the headers.
```bash
gcc your_app.c build/bignum_cmp.o -I./include -I./libs/bignum-common/include -o your_app -no-pie
```	

## Contributing

Contributions are welcome! Please follow these steps:
1.  Fork the repository.
2.  Create a new branch (`git checkout -b feature/AmazingFeature`).
3.  Make your changes.
4.  Commit your changes (`git commit -m 'Add some AmazingFeature'`).
5.  Push to the branch (`git push origin feature/AmazingFeature`).
6.  Open a Pull Request.

When creating Issues or Pull Requests, please use the provided templates to ensure all necessary information is included.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
