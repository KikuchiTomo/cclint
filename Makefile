.PHONY: build clean install

# ビルド
build:
	@mkdir -p build
	@cd build && cmake .. && $(MAKE) -j$(shell sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
	@echo "✓ Build complete: ./build/src/cclint"

# クリーン
clean:
	@rm -rf build CMakeCache.txt CMakeFiles cmake_install.cmake
	@echo "✓ Clean complete"

# インストール
install: build
	@cd build && $(MAKE) install
	@echo "✓ Install complete"
