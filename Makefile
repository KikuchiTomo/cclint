.PHONY: build debug release clean install

# デフォルトビルド (Debug)
build: debug

# Debugビルド
debug:
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && $(MAKE) -j$(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
	@echo "✓ Debug build complete: ./build/src/cclint"

# Releaseビルド
release:
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && $(MAKE) -j$(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
	@echo "✓ Release build complete: ./build/src/cclint"

# クリーン
clean:
	@rm -rf build CMakeCache.txt CMakeFiles cmake_install.cmake
	@echo "✓ Clean complete"

# インストール
install: release
	@cd build && $(MAKE) install
	@echo "✓ Install complete"
