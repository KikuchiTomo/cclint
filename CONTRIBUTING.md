# Contributing to cclint

## Building from source

### Requirements

- Rust stable (1.75 or later)
- libclang (required at both build and run time)
- A C toolchain

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y build-essential libclang-dev llvm-dev pkg-config
```

### macOS

```bash
xcode-select --install
# or
brew install llvm
```

`make` auto-detects `LIBCLANG_PATH`. Override it via the environment variable
when needed.

## Development commands

```
make build               Release build
make run                 Run on tests/cpp_fixtures with example rules
make test                All tests
make test-rust           cargo test only
make test-fixtures       Violation detection (e2e)
make test-fixtures-clean No-false-positive check
make fmt                 cargo fmt
make lint                cargo clippy
make dist                Produce distribution tarball under dist/
make install             Install to /usr/local/bin
make clean
```

## Layout

```
crates/
  cclint-cli/         Binary (clap)
  cclint-config/      .cclint.toml loader
  cclint-ast/         libclang wrapper + OwnedNode
  cclint-script/      Lua bindings
  cclint-diagnostic/  Diagnostic types + formatting
examples/rules/       Sample Lua rules
tests/cpp_fixtures/         Violation fixtures (must trigger)
tests/cpp_fixtures_clean/   Clean fixtures (must not trigger)
scripts/
  dist.sh             Distribution tarball builder
  install.sh          curl-based installer
.github/workflows/
  ci.yml              ubuntu/macos test matrix
  release.yml         Release build on tag push
```

## Commits

Follow Conventional Commits.

```
feat: ...
fix: ...
docs: ...
refactor: ...
test: ...
chore: ...
```

Use `feat!:` or a `BREAKING CHANGE:` footer for breaking changes.

## Releases

`git tag v0.x.y && git push --tags` triggers `release.yml`, which produces
per-platform tarballs and publishes a GitHub Release.
