#!/usr/bin/env bash
# setup.sh — one command to rule them all
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VCPKG_ROOT="${REPO_ROOT}/vcpkg"

echo "Setting up clutils in $REPO_ROOT ..."

# Clone vcpkg locally if missing
if [ ! -f "${VCPKG_ROOT}/vcpkg" ] && [ ! -f "${VCPKG_ROOT}/bootstrap-vcpkg.sh" ]; then
    echo "vcpkg not found → cloning locally..."
    git clone --depth 1 https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT"
    echo "Removing vcpkg's .git directory (saves ~300 MB)"
    rm -rf "$VCPKG_ROOT/.git"
else
    echo "vcpkg already present → skipping clone"
fi

# Bootstrap if needed
if [ ! -f "${VCPKG_ROOT}/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    if [[ "$OSTYPE" == "darwin"* ]]; then
        "$VCPKG_ROOT/bootstrap-vcpkg.sh"
    else
        "$VCPKG_ROOT/bootstrap-vcpkg.bat" 2>/dev/null || "$VCPKG_ROOT/bootstrap-vcpkg.sh"
    fi
fi

# Build
echo "Building clutils..."
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release

echo
echo "clutils is ready!"
echo "Run: ./build/clutils uuid"
echo
echo "Tip: ln -sf \"$(pwd)/build/clutils\" ~/bin/clutils   # add to PATH forever"