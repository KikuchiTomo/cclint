# Makefile for sample C++ project

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
SRCDIR = src
TARGETDIR = target
OBJDIR = build/obj
BINDIR = build/bin

# Source files (library code)
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Target executables
TARGETS = $(BINDIR)/calculator_demo $(BINDIR)/bad_naming_demo $(BINDIR)/call_graph_test

.PHONY: all clean lint

all: $(OBJDIR) $(BINDIR) $(TARGETS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Compile object files from src/
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Build calculator_demo
$(BINDIR)/calculator_demo: $(TARGETDIR)/calculator_demo.cpp $(OBJDIR)/calculator.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build bad_naming_demo
$(BINDIR)/bad_naming_demo: $(TARGETDIR)/bad_naming_demo.cpp $(OBJDIR)/bad_naming.o | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build call_graph_test
$(BINDIR)/call_graph_test: $(TARGETDIR)/call_graph_test.cpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -rf build

# Run all targets
run: all
	@echo "Running calculator_demo..."
	@$(BINDIR)/calculator_demo
	@echo ""
	@echo "Running bad_naming_demo..."
	@$(BINDIR)/bad_naming_demo
	@echo ""
	@echo "Running call_graph_test..."
	@$(BINDIR)/call_graph_test

# Run cclint on the project
lint:
	@echo "=== Running cclint on the project ==="
	@echo ""
	@echo "Checking include/ directory..."
	@../../build/src/cclint --config .cclint.yaml include/*.hpp || true
	@echo ""
	@echo "Checking src/ directory..."
	@../../build/src/cclint --config .cclint.yaml src/*.cpp || true
	@echo ""
	@echo "Checking target/ directory..."
	@../../build/src/cclint --config .cclint.yaml target/*.cpp || true
	@echo ""
	@echo "=== Lint complete ==="
