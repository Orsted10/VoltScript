# Contributing to ClawScript

Thank you for your interest in contributing to ClawScript! This document provides guidelines for contributors.

## Development Setup

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.16 or higher
- Git

### Building
```bash
git clone https://github.com/your-org/clawscript.git
cd clawscript
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Running Tests
```bash
ctest --output-on-failure
```

## Contribution Guidelines

### Code Style
- Follow C++20 best practices
- Use clang-format for code formatting
- Include appropriate headers and namespace usage
- Write clear, descriptive variable and function names

### Testing
- Add unit tests for new features
- Ensure all tests pass before submitting
- Test on multiple platforms if possible
- Include edge cases in test coverage

### Documentation
- Update README.md for user-facing changes
- Add inline comments for complex logic
- Update function documentation in headers
- Include examples for new APIs

### Commit Messages
- Use clear, descriptive commit messages
- Prefix with type: `feat:`, `fix:`, `docs:`, `test:`, etc.
- Keep messages concise but informative
- Reference issue numbers when applicable

## Pull Request Process

1. Fork the repository
2. Create feature branch: `git checkout -b feature-name`
3. Make changes with proper testing
4. Ensure code follows style guidelines
5. Submit pull request with description
6. Address review feedback promptly

## Code Review

All contributions require code review. Reviewers will check for:
- Code quality and style
- Test coverage
- Performance implications
- Documentation completeness
- Breaking changes

## Bug Reports

When reporting bugs, please include:
- Minimal reproduction case
- Expected vs actual behavior
- Environment details (OS, compiler, version)
- Error messages and stack traces
- Steps to reproduce

## Feature Requests

Feature requests should:
- Clearly describe the proposed feature
- Explain the use case and benefits
- Consider implementation complexity
- Discuss potential alternatives

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Community

- Be respectful and constructive
- Help others learn and contribute
- Follow the code of conduct
- Focus on what is best for the project

Thank you for contributing to ClawScript!
