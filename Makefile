CARGO ?= cargo
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
TARGET_BIN := target/release/cclint

# libclang は `clang` crate の `runtime` フィーチャーで dlopen 経由で読む。
# 環境によっては LIBCLANG_PATH を指定しないと見つからない場合があるため、
# 自動検出して export する。Linux/macOS 両対応。
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  # macOS: Xcode → Homebrew LLVM の順で探す
  XCODE_LIB := $(shell xcrun --find clang >/dev/null 2>&1 && echo "$$(dirname $$(dirname $$(xcrun --find clang)))/lib")
  ifneq ($(wildcard $(XCODE_LIB)/libclang.dylib),)
    export LIBCLANG_PATH ?= $(XCODE_LIB)
  else ifneq ($(wildcard /opt/homebrew/opt/llvm/lib/libclang.dylib),)
    export LIBCLANG_PATH ?= /opt/homebrew/opt/llvm/lib
  else ifneq ($(wildcard /usr/local/opt/llvm/lib/libclang.dylib),)
    export LIBCLANG_PATH ?= /usr/local/opt/llvm/lib
  endif
endif

ifeq ($(UNAME_S),Linux)
  # Linux: llvm-config → 共通パス探索
  LLVM_CONFIG := $(shell command -v llvm-config-18 llvm-config-17 llvm-config-16 llvm-config-15 llvm-config-14 llvm-config 2>/dev/null | head -1)
  ifneq ($(LLVM_CONFIG),)
    DETECTED_LIB := $(shell $(LLVM_CONFIG) --libdir 2>/dev/null)
    ifneq ($(wildcard $(DETECTED_LIB)/libclang.so*),)
      export LIBCLANG_PATH ?= $(DETECTED_LIB)
    endif
  endif
  # フォールバック: Debian/Ubuntu の典型パス
  ifeq ($(LIBCLANG_PATH),)
    SO_GUESS := $(firstword $(wildcard /usr/lib/x86_64-linux-gnu/libclang-*.so* /usr/lib/aarch64-linux-gnu/libclang-*.so* /usr/lib/llvm-*/lib/libclang.so*))
    ifneq ($(SO_GUESS),)
      export LIBCLANG_PATH ?= $(dir $(SO_GUESS))
    endif
  endif
endif

FIXTURE_DIR := tests/cpp_fixtures
CLEAN_FIXTURE_DIR := tests/cpp_fixtures_clean
EXAMPLE_RULES := examples/rules

.PHONY: all build run test test-rust test-fixtures test-fixtures-clean dist install uninstall clean fmt lint check help

all: build

help:
	@echo "make build       - リリースビルド"
	@echo "make run         - tests/cpp_fixtures に対して example ルールを走らせる"
	@echo "make test        - cargo test + 違反検出 e2e テスト"
	@echo "make test-rust   - cargo test のみ"
	@echo "make test-fixtures - C++ fixture に対する検出テストのみ"
	@echo "make fmt         - cargo fmt"
	@echo "make lint        - cargo clippy"
	@echo "make dist        - 配布用 tarball (cclint + libclang) を作る"
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

test: test-rust test-fixtures test-fixtures-clean

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

# 健全なコードに対して 0 件の警告/エラーが出ることを確認 (誤検出ゼロ)。
test-fixtures-clean: $(TARGET_BIN)
	@echo "==> running cclint on $(CLEAN_FIXTURE_DIR) (期待: 0 件)"
	@OUT=$$($(TARGET_BIN) $(CLEAN_FIXTURE_DIR) -c $(CLEAN_FIXTURE_DIR)/.cclint.toml --format json 2>/dev/null || true); \
	echo "$$OUT" > $(CLEAN_FIXTURE_DIR)/.last_run.json; \
	COUNT=$$(echo "$$OUT" | grep -c '"rule"' || true); \
	if [ "$$COUNT" -eq 0 ]; then \
	  echo "PASS: 誤検出 0 件"; \
	else \
	  echo "FAIL: 誤検出 $$COUNT 件"; \
	  echo "$$OUT" | sed -n '1,80p'; \
	  exit 1; \
	fi

fmt:
	$(CARGO) fmt --all

lint:
	$(CARGO) clippy --workspace --all-targets -- -D warnings

check:
	$(CARGO) check --workspace

dist: build
	./scripts/dist.sh dist

install: build
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 $(TARGET_BIN) $(DESTDIR)$(BINDIR)/cclint

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/cclint

clean:
	$(CARGO) clean
	rm -f $(FIXTURE_DIR)/.last_run.json $(CLEAN_FIXTURE_DIR)/.last_run.json
