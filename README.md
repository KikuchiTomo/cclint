# cclint (customizable c++ linter)

A C++ linter. The AST is obtained from libclang, and rules are written in Lua.

## Supported platforms

- Linux x86_64 / aarch64
- macOS aarch64 (Apple Silicon)

Pre-built binaries are not provided for Intel mac (x86_64). See `CONTRIBUTING.md`
to build from source.

## Install

```bash
curl -fsSL https://github.com/KikuchiTomo/cclint/raw/main/scripts/install.sh | bash
```

The binary lands in `$HOME/.local/bin/cclint`. Override the install prefix with
`CCLINT_PREFIX`.

```bash
CCLINT_PREFIX=/usr/local curl -fsSL https://github.com/KikuchiTomo/cclint/raw/main/scripts/install.sh | sudo bash
```

libclang is bundled. No separate install required.

## Usage

Place `.cclint.toml` at the project root.

```toml
cpp_standard = "c++17"
include_patterns = ["src/**/*.cpp", "src/**/*.h"]
exclude_patterns = ["build/**"]

[[rules]]
path = "rules/private_prefix.lua"
```

Run `cclint .`. Use `--format json` for machine-readable output.

## Documentation

- `docs/api.md` — Lua API reference
- `docs/rules.md` — Bundled sample rules
- `CONTRIBUTING.md` — Building from source

## License

GPL-3.0-or-later. See `LICENSE`.
