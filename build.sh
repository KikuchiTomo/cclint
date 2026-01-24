#!/bin/bash
# cclint build script

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Release"
BUILD_DIR="build"
CLEAN_BUILD=false
INSTALL=false
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --install)
            INSTALL=true
            shift
            ;;
        --jobs|-j)
            JOBS="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug         Build in Debug mode (default: Release)"
            echo "  --clean         Clean build directory before building"
            echo "  --install       Install after building"
            echo "  --jobs, -j N    Use N parallel jobs (default: auto-detect)"
            echo "  --help, -h      Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                    # Build in Release mode"
            echo "  $0 --debug           # Build in Debug mode"
            echo "  $0 --clean --install # Clean, build, and install"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo -e "${BLUE}======================================${NC}"
echo -e "${BLUE}  cclint Build Script${NC}"
echo -e "${BLUE}======================================${NC}"
echo ""
echo -e "${GREEN}Build Type:${NC} $BUILD_TYPE"
echo -e "${GREEN}Build Directory:${NC} $BUILD_DIR"
echo -e "${GREEN}Parallel Jobs:${NC} $JOBS"
echo ""

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
    echo -e "${GREEN}✓ Build directory cleaned${NC}"
    echo ""
fi

# Create build directory
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cd "$BUILD_DIR"
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ CMake configuration failed${NC}"
    exit 1
fi
echo -e "${GREEN}✓ CMake configuration successful${NC}"
echo ""

# Build
echo -e "${YELLOW}Building cclint...${NC}"
make -j"$JOBS"

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Build successful${NC}"
echo ""

# Show binary info
if [ -f "src/cclint" ]; then
    BINARY_SIZE=$(du -h src/cclint | cut -f1)
    echo -e "${GREEN}Binary:${NC} src/cclint (${BINARY_SIZE})"
    echo ""
fi

# Install if requested
if [ "$INSTALL" = true ]; then
    echo -e "${YELLOW}Installing cclint...${NC}"
    sudo make install

    if [ $? -ne 0 ]; then
        echo -e "${RED}✗ Installation failed${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ Installation successful${NC}"
    echo ""
fi

cd ..

# Show usage info
echo -e "${BLUE}======================================${NC}"
echo -e "${GREEN}Build complete!${NC}"
echo -e "${BLUE}======================================${NC}"
echo ""
echo "To run cclint:"
echo "  ./$BUILD_DIR/src/cclint --help"
echo ""
echo "To install cclint:"
echo "  sudo make -C $BUILD_DIR install"
echo ""
echo "To run tests (when implemented):"
echo "  ./$BUILD_DIR/tests/cclint_tests"
