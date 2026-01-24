#!/bin/bash
# Install dependencies for cclint

set -e

echo "Installing cclint dependencies..."

# Detect OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
elif [ "$(uname)" = "Darwin" ]; then
    OS="macos"
else
    echo "Unsupported operating system"
    exit 1
fi

echo "Detected OS: $OS"

# Install dependencies based on OS
case "$OS" in
    ubuntu|debian)
        echo "Installing dependencies for Ubuntu/Debian..."
        sudo apt update

        # Basic build tools
        sudo apt install -y \
            build-essential \
            cmake \
            git

        # Optional: LuaJIT
        echo "Installing LuaJIT..."
        sudo apt install -y libluajit-5.1-dev || {
            echo "LuaJIT not available in repositories, trying alternative..."
            sudo apt install -y luajit libluajit-5.1-2
        }

        # Optional: yaml-cpp
        read -p "Install yaml-cpp? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            sudo apt install -y libyaml-cpp-dev
        fi

        # Optional: LLVM/Clang
        read -p "Install LLVM/Clang? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            sudo apt install -y llvm-14-dev libclang-14-dev
        fi

        # Google Test (for testing)
        read -p "Install Google Test? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            sudo apt install -y libgtest-dev
        fi
        ;;

    fedora|rhel|centos)
        echo "Installing dependencies for Fedora/RHEL/CentOS..."
        sudo dnf install -y \
            gcc-c++ \
            cmake \
            git \
            luajit-devel

        # Optional: yaml-cpp
        read -p "Install yaml-cpp? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            sudo dnf install -y yaml-cpp-devel
        fi

        # Optional: LLVM/Clang
        read -p "Install LLVM/Clang? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            sudo dnf install -y llvm-devel clang-devel
        fi
        ;;

    macos)
        echo "Installing dependencies for macOS..."

        # Check if Homebrew is installed
        if ! command -v brew &> /dev/null; then
            echo "Homebrew not found. Installing Homebrew..."
            /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        fi

        # Install dependencies
        brew install cmake git luajit

        # Optional: yaml-cpp
        read -p "Install yaml-cpp? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            brew install yaml-cpp
        fi

        # Optional: LLVM
        read -p "Install LLVM? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            brew install llvm
        fi

        # Google Test
        read -p "Install Google Test? (y/n) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            brew install googletest
        fi
        ;;

    *)
        echo "Unsupported OS: $OS"
        echo "Please install dependencies manually:"
        echo "  - CMake 3.16+"
        echo "  - C++ compiler (g++ or clang++)"
        echo "  - LuaJIT 2.1+ (optional but recommended)"
        echo "  - yaml-cpp (optional)"
        echo "  - LLVM/Clang (optional)"
        exit 1
        ;;
esac

echo ""
echo "Dependencies installed successfully!"
echo ""
echo "To build cclint:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make"
echo ""
echo "To enable optional features:"
echo "  cmake -DUSE_LUAJIT=ON -DUSE_YAML_CPP=ON -DUSE_LLVM_CLANG=ON .."
echo ""
