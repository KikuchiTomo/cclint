CARGO ?= cargo
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
TARGET_BIN := target/release/cclint

# libclang は `clang` crate の `runtime` フィーチャーで dlopen 経由で読む。
# 環境によっては LIBCLANG_PATH を指定しないと見つからない場合があるため、
# 自動検出して export する。
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  XCODE_LIB := $(shell xcrun --find clang >/dev/null 2>&1 && echo "$$(dirname $$(dirname $$(xcrun --find clang)))/lib")
  ifneq ($(wildcard $(XCODE_LIB)/libclang.dylib),)
    export LIBCLANG_PATH ?= $(XCODE_LIB)
  endif
endif

FIXTURE_DIR := tests/cpp_fixtures
EXAMPLE_RULES := examples/rules

.PHONY: all build run test test-rust test-fixtures install uninstall clean fmt lint check help

all: build

help:
	@echo "make build       - リリースビルド"
	@echo "make run         - tests/cpp_fixtures に対して example ルールを走らせる"
	@echo "make test        - cargo test + 違反検出 e2e テスト"
	@echo "make test-rust   - cargo test のみ"
	@echo "make test-fixtures - C++ fixture に対する検出テストのみ"
	@echo "make fmt         - cargo fmt"
	@echo "make lint        - cargo clippy"
	@echo "make install     - $(BINDIR) に cclint をインストール"
	@echo "make uninstall   - $(BINDIR)/cclint を削除"
	@echo "make clean       - target/ を削除"

build:
	$(CARGO) build --release

$(TARGET_BIN): build

run: $(TARGET_BIN)
	$(TARGET_BIN) $(FIXTURE_DIR) \
	  -c $(FIXTURE_DIR)/.cclint.toml \
	  || true

test: test-rust test-fixtures

test-rust:
	$(CARGO) test --workspace

# 違反コードを linter に通し、期待した警告/エラーが出ることを検証する。
# tests/cpp_fixtures/expected.txt に grep パターンを 1 行 1 件で書いておく。
test-fixtures: $(TARGET_BIN)
	@echo "==> running cclint on $(FIXTURE_DIR)"
	@OUT=$$($(TARGET_BIN) $(FIXTURE_DIR) -c $(FIXTURE_DIR)/.cclint.toml --format json 2>/dev/null || true); \
	echo "$$OUT" > $(FIXTURE_DIR)/.last_run.json; \
	FAIL=0; \
	while IFS= read -r pat; do \
	  case "$$pat" in ''|\#*) continue;; esac; \
	  if echo "$$OUT" | grep -q -- "$$pat"; then \
	    echo "  ok: $$pat"; \
	  else \
	    echo "  MISS: $$pat"; FAIL=1; \
	  fi; \
	done < $(FIXTURE_DIR)/expected.txt; \
	if [ $$FAIL -ne 0 ]; then echo "FAIL: 一部の違反が検出されませんでした"; exit 1; fi; \
	echo "PASS: 期待された違反を全て検出"

fmt:
	$(CARGO) fmt --all

lint:
	$(CARGO) clippy --workspace --all-targets -- -D warnings

check:
	$(CARGO) check --workspace

install: build
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 $(TARGET_BIN) $(DESTDIR)$(BINDIR)/cclint

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/cclint

clean:
	$(CARGO) clean
	rm -f $(FIXTURE_DIR)/.last_run.json
