CXX       ?= g++
CXXFLAGS  := -std=c++17 -Wall -Wextra -Isrc
CCLINT    ?= ../../build/src/cclint
CONFIG    := .cclint.yaml
TARGET    := sample
SOURCES   := src/main.cpp
HEADERS   := $(wildcard src/*.hpp)
ALL_FILES := $(SOURCES) $(HEADERS)

.PHONY: all build lint clean

# Lint all files then build
all: lint build

# Normal build (no lint)
build: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES)

# Lint each source and header individually
lint: $(ALL_FILES)
	@for f in $(ALL_FILES); do \
		$(CCLINT) --config=$(CONFIG) --no-cache $(CXX) -fsyntax-only $(CXXFLAGS) $$f 2>&1 || true; \
	done

clean:
	rm -f $(TARGET)
