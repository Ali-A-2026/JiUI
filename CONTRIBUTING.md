# Contributing to JiUI

Thank you for your interest in contributing to JiUI! We welcome contributions from everyone.

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/JiUI.git`
3. Create a branch: `git checkout -b feature/my-feature`
4. Build: `cmake --preset dev && cmake --build build`
5. Test: `cd build && ctest --output-on-failure`

## Coding Standards

- **Language:** C11 (with C++ wrappers in C++17)
- **Style:** K&R with 4-space indentation
- **Naming:** `ji_module_function_name()` for public API, `snake_case` for internal
- **Headers:** Every public function in `include/jiui/ji_*.h`, implementation in `src/*/ji_*.c`
- **Memory:** Use `ji_alloc()`/`ji_calloc()`/`ji_free()` — never raw `malloc`/`free`
- **Errors:** Use `JI_ERROR_LOG()` for error reporting
- **API:** All public functions must have `JI_API` prefix and be in `ji_api.h`

## Pull Request Process

1. Ensure all 27+ tests pass: `cd build && ctest`
2. Add tests for new features
3. Update documentation in header files
4. Keep PRs focused — one feature per PR
5. Write clear commit messages

## Reporting Bugs

- Use GitHub Issues
- Include: OS, compiler, steps to reproduce, expected vs actual behavior

## License

By contributing, you agree that your contributions will be licensed under LGPL-3.0.
