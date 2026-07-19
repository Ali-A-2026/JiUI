# Security Policy

## Reporting a Vulnerability

If you discover a security vulnerability in JiUI, please report it responsibly:

- **Email:** Open an issue on GitHub at [https://github.com/Ali-A-2026/JiUI/security](https://github.com/Ali-A-2026/JiUI/security)
- **Do not** publicly disclose the vulnerability until it has been addressed

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 0.1.x   | ✅ Active development |

## Security Considerations

- JiUI renders UI via OpenGL and handles user input through platform-specific backends
- The markup parser processes input files — do not load untrusted JiUI markup
- Text input handling supports basic ASCII; no Unicode normalization is performed
- The framework does not make network requests or access the filesystem beyond what the application explicitly does

## License

JiUI is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
