#!/bin/bash

# Simple build script for CMake-based build
# This replaces the various make targets from the original Makefile

set -e

# Default build directory
BUILD_DIR="build"

# Parse command line arguments
TARGET="all"
if [ $# -gt 0 ]; then
    TARGET="$1"
fi

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure with CMake if not already configured
if [ ! -f "Makefile" ]; then
    echo "Configuring with CMake..."
    cmake ..
fi

# Build based on target
case "$TARGET" in
    "lib")
        echo "Building libraries..."
        make RBR RBRDynamicCorrection
        ;;
    "libdynamiccorrection")
        echo "Building dynamic correction library..."
        make RBRDynamicCorrection
        ;;
    "tests")
        echo "Building and running tests..."
        make tests
        echo "Running tests..."
        ./bin/tests
        ;;
    "docs")
        echo "Building documentation..."
        make docs
        ;;
    "devdocs")
        echo "Building developer documentation..."
        make devdocs
        ;;
    "clean")
        echo "Cleaning build directory..."
        cd ..
        rm -rf "$BUILD_DIR"
        echo "Build directory cleaned."
        exit 0
        ;;
    "all"|*)
        echo "Building all targets..."
        make
        if [ -f "bin/tests" ]; then
            echo "Running tests..."
            ./bin/tests
        fi
        ;;
esac

echo "Build completed successfully!"