#!/bin/bash
set -e
cd "$(dirname "$0")"

BUILD_DIR="/tmp/granatier-build"
INSTALL_DIR="/tmp/granatier-install"

if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
    echo "Configuring build directory..."
    cmake -B "$BUILD_DIR" -S . -DCMAKE_BUILD_TYPE=Debug
fi

cmake --build "$BUILD_DIR" -j"$(nproc)" 2>&1 | tail -5
cmake --install "$BUILD_DIR" --prefix "$INSTALL_DIR" 2>/dev/null
XDG_DATA_DIRS="$INSTALL_DIR/share:/usr/local/share:/usr/share" "$INSTALL_DIR/bin/granatier" "$@"
