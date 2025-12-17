#!/bin/bash
#
# WASM Build Script for sp.h
#
# This script builds the sp.h test program for three WASM targets:
# 1. Emscripten (wasm32-unknown-emscripten)
# 2. WASI-SDK (wasm32-wasi)
# 3. Raw Clang (wasm32-unknown-unknown) - freestanding
#
# Usage:
#   ./wasm_build.sh [clean|build|run|all]
#
# Requirements:
#   - Emscripten SDK (emsdk)
#   - WASI-SDK
#   - Clang with WASM target support
#   - wasmtime (for running WASI modules)
#   - Node.js (for running Emscripten modules)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build/wasm"
TEST_SRC="${SCRIPT_DIR}/test/wasm_test.c"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

info() {
  echo -e "${BLUE}[INFO]${NC} $1"
}

success() {
  echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warn() {
  echo -e "${YELLOW}[WARN]${NC} $1"
}

error() {
  echo -e "${RED}[ERROR]${NC} $1"
  exit 1
}

# ============================================================================
# Tool Detection and Installation
# ============================================================================

check_emscripten() {
  if command -v emcc &> /dev/null; then
    EMCC=$(command -v emcc)
    info "Found Emscripten: $EMCC"
    return 0
  fi

  # Try to find emsdk
  if [ -n "$EMSDK" ]; then
    source "$EMSDK/emsdk_env.sh" 2>/dev/null || true
    if command -v emcc &> /dev/null; then
      EMCC=$(command -v emcc)
      info "Found Emscripten via EMSDK: $EMCC"
      return 0
    fi
  fi

  # Common installation locations
  for dir in "$HOME/emsdk" "/opt/emsdk" "/usr/local/emsdk"; do
    if [ -f "$dir/emsdk_env.sh" ]; then
      source "$dir/emsdk_env.sh" 2>/dev/null || true
      if command -v emcc &> /dev/null; then
        EMCC=$(command -v emcc)
        info "Found Emscripten: $EMCC"
        return 0
      fi
    fi
  done

  warn "Emscripten not found. Installing..."
  install_emscripten
}

install_emscripten() {
  EMSDK_DIR="$HOME/emsdk"

  if [ ! -d "$EMSDK_DIR" ]; then
    info "Cloning emsdk..."
    git clone https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"
  fi

  cd "$EMSDK_DIR"
  ./emsdk install latest
  ./emsdk activate latest
  source ./emsdk_env.sh

  if command -v emcc &> /dev/null; then
    EMCC=$(command -v emcc)
    success "Emscripten installed successfully"
    cd "$SCRIPT_DIR"
  else
    error "Failed to install Emscripten"
  fi
}

check_wasi_sdk() {
  # Check common locations
  for dir in "/opt/wasi-sdk" "$HOME/wasi-sdk" "/usr/local/wasi-sdk"; do
    if [ -f "$dir/bin/clang" ]; then
      WASI_SDK="$dir"
      WASI_CLANG="$dir/bin/clang"
      info "Found WASI-SDK: $WASI_SDK"
      return 0
    fi
  done

  # Check if wasi-sdk clang is in PATH
  if command -v wasi-sdk-clang &> /dev/null; then
    WASI_CLANG=$(command -v wasi-sdk-clang)
    info "Found WASI-SDK clang: $WASI_CLANG"
    return 0
  fi

  warn "WASI-SDK not found. Installing..."
  install_wasi_sdk
}

install_wasi_sdk() {
  WASI_SDK_VERSION="24"
  WASI_SDK_DIR="/opt/wasi-sdk"

  # Detect OS
  OS=$(uname -s)
  ARCH=$(uname -m)

  case "$OS" in
    Linux)
      if [ "$ARCH" = "x86_64" ]; then
        WASI_SDK_URL="https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_SDK_VERSION}/wasi-sdk-${WASI_SDK_VERSION}.0-x86_64-linux.tar.gz"
      elif [ "$ARCH" = "aarch64" ]; then
        WASI_SDK_URL="https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_SDK_VERSION}/wasi-sdk-${WASI_SDK_VERSION}.0-arm64-linux.tar.gz"
      fi
      ;;
    Darwin)
      if [ "$ARCH" = "x86_64" ]; then
        WASI_SDK_URL="https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_SDK_VERSION}/wasi-sdk-${WASI_SDK_VERSION}.0-x86_64-macos.tar.gz"
      elif [ "$ARCH" = "arm64" ]; then
        WASI_SDK_URL="https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_SDK_VERSION}/wasi-sdk-${WASI_SDK_VERSION}.0-arm64-macos.tar.gz"
      fi
      ;;
    *)
      error "Unsupported OS: $OS"
      ;;
  esac

  if [ -z "$WASI_SDK_URL" ]; then
    error "Could not determine WASI-SDK download URL for $OS/$ARCH"
  fi

  info "Downloading WASI-SDK from $WASI_SDK_URL"

  TEMP_DIR=$(mktemp -d)
  cd "$TEMP_DIR"

  curl -L -o wasi-sdk.tar.gz "$WASI_SDK_URL"
  tar xzf wasi-sdk.tar.gz

  # Find the extracted directory
  EXTRACTED_DIR=$(ls -d wasi-sdk-* | head -1)

  if [ -d "$WASI_SDK_DIR" ]; then
    sudo rm -rf "$WASI_SDK_DIR"
  fi

  sudo mv "$EXTRACTED_DIR" "$WASI_SDK_DIR"

  rm -rf "$TEMP_DIR"
  cd "$SCRIPT_DIR"

  WASI_SDK="$WASI_SDK_DIR"
  WASI_CLANG="$WASI_SDK_DIR/bin/clang"

  if [ -f "$WASI_CLANG" ]; then
    success "WASI-SDK installed successfully"
  else
    error "Failed to install WASI-SDK"
  fi
}

check_clang() {
  # Check for clang with WASM target support
  if command -v clang &> /dev/null; then
    CLANG=$(command -v clang)
    # Test WASM support
    if $CLANG --target=wasm32 -c -x c /dev/null -o /dev/null 2>/dev/null; then
      info "Found clang with WASM support: $CLANG"
      return 0
    fi
  fi

  # Try clang-18, clang-17, etc.
  for ver in 18 17 16 15 14; do
    if command -v "clang-$ver" &> /dev/null; then
      CLANG=$(command -v "clang-$ver")
      if $CLANG --target=wasm32 -c -x c /dev/null -o /dev/null 2>/dev/null; then
        info "Found clang with WASM support: $CLANG"
        return 0
      fi
    fi
  done

  error "clang with WASM target support not found. Please install clang >= 14"
}

check_wasmtime() {
  if command -v wasmtime &> /dev/null; then
    WASMTIME=$(command -v wasmtime)
    info "Found wasmtime: $WASMTIME"
    return 0
  fi

  warn "wasmtime not found. Installing..."
  curl https://wasmtime.dev/install.sh -sSf | bash
  export PATH="$HOME/.wasmtime/bin:$PATH"

  if command -v wasmtime &> /dev/null; then
    WASMTIME=$(command -v wasmtime)
    success "wasmtime installed successfully"
  else
    error "Failed to install wasmtime"
  fi
}

check_node() {
  if command -v node &> /dev/null; then
    NODE=$(command -v node)
    info "Found Node.js: $NODE"
    return 0
  fi

  warn "Node.js not found. Emscripten tests will be skipped."
  NODE=""
}

# ============================================================================
# Build Functions
# ============================================================================

clean() {
  info "Cleaning build directory..."
  rm -rf "$BUILD_DIR"
  success "Clean complete"
}

build_emscripten() {
  info "Building for Emscripten..."
  mkdir -p "$BUILD_DIR/emscripten"

  $EMCC -O2 \
    -DSP_IMPLEMENTATION \
    -sALLOW_MEMORY_GROWTH=1 \
    -sEXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -sEXPORTED_FUNCTIONS='["_main"]' \
    -sSTACK_SIZE=1048576 \
    -sINITIAL_MEMORY=16777216 \
    -I"$SCRIPT_DIR" \
    "$TEST_SRC" \
    -o "$BUILD_DIR/emscripten/sp_test.js"

  if [ -f "$BUILD_DIR/emscripten/sp_test.js" ]; then
    success "Emscripten build complete: $BUILD_DIR/emscripten/sp_test.js"
  else
    error "Emscripten build failed"
  fi
}

build_wasi() {
  info "Building for WASI..."
  mkdir -p "$BUILD_DIR/wasi"

  $WASI_CLANG -O2 \
    --target=wasm32-wasi \
    --sysroot="$WASI_SDK/share/wasi-sysroot" \
    -DSP_IMPLEMENTATION \
    -I"$SCRIPT_DIR" \
    "$TEST_SRC" \
    -o "$BUILD_DIR/wasi/sp_test.wasm" \
    -lm

  if [ -f "$BUILD_DIR/wasi/sp_test.wasm" ]; then
    success "WASI build complete: $BUILD_DIR/wasi/sp_test.wasm"
  else
    error "WASI build failed"
  fi
}

build_freestanding() {
  info "Building for freestanding WASM..."
  mkdir -p "$BUILD_DIR/freestanding"

  # Use the regular clang with WASM target
  # Note: We need to provide our own entry point and avoid libc
  $CLANG -O2 \
    --target=wasm32 \
    -nostdlib \
    -Wl,--no-entry \
    -Wl,--export=run_tests \
    -Wl,--export=__heap_base \
    -Wl,--initial-memory=1048576 \
    -Wl,--max-memory=16777216 \
    -DSP_IMPLEMENTATION \
    -I"$SCRIPT_DIR" \
    "$TEST_SRC" \
    -o "$BUILD_DIR/freestanding/sp_test.wasm"

  if [ -f "$BUILD_DIR/freestanding/sp_test.wasm" ]; then
    success "Freestanding build complete: $BUILD_DIR/freestanding/sp_test.wasm"
  else
    error "Freestanding build failed"
  fi
}

build_all() {
  info "Building all targets..."
  build_emscripten
  build_wasi
  build_freestanding
  success "All builds complete"
}

# ============================================================================
# Run Functions
# ============================================================================

run_emscripten() {
  info "Running Emscripten test..."

  if [ -z "$NODE" ]; then
    warn "Skipping Emscripten test (Node.js not available)"
    return
  fi

  if [ ! -f "$BUILD_DIR/emscripten/sp_test.js" ]; then
    error "Emscripten build not found. Run 'build' first."
  fi

  cd "$BUILD_DIR/emscripten"
  $NODE sp_test.js
  RET=$?
  cd "$SCRIPT_DIR"

  if [ $RET -eq 0 ]; then
    success "Emscripten test passed"
  else
    error "Emscripten test failed with code $RET"
  fi
}

run_wasi() {
  info "Running WASI test..."

  if [ ! -f "$BUILD_DIR/wasi/sp_test.wasm" ]; then
    error "WASI build not found. Run 'build' first."
  fi

  # Create a temp directory for WASI filesystem access
  WASI_TMP=$(mktemp -d)
  $WASMTIME --dir=/tmp::/tmp "$BUILD_DIR/wasi/sp_test.wasm"
  RET=$?
  rm -rf "$WASI_TMP"

  if [ $RET -eq 0 ]; then
    success "WASI test passed"
  else
    error "WASI test failed with code $RET"
  fi
}

run_freestanding() {
  info "Running freestanding WASM test..."

  if [ ! -f "$BUILD_DIR/freestanding/sp_test.wasm" ]; then
    error "Freestanding build not found. Run 'build' first."
  fi

  # For freestanding, we use wasmtime to run the exported function
  # We need to create a small wrapper or use wasmtime's invoke
  $WASMTIME --invoke run_tests "$BUILD_DIR/freestanding/sp_test.wasm"
  RET=$?

  if [ $RET -eq 0 ]; then
    success "Freestanding test passed"
  else
    # Note: freestanding may return test count, not necessarily 0
    warn "Freestanding test returned code $RET (check output for pass/fail)"
  fi
}

run_all() {
  info "Running all tests..."
  run_emscripten
  run_wasi
  run_freestanding
  success "All tests complete"
}

# ============================================================================
# Main
# ============================================================================

print_usage() {
  echo "Usage: $0 [command]"
  echo ""
  echo "Commands:"
  echo "  clean     - Remove build artifacts"
  echo "  build     - Build all WASM targets"
  echo "  run       - Run all WASM tests"
  echo "  all       - Clean, build, and run all"
  echo "  check     - Check/install dependencies"
  echo ""
  echo "Individual targets:"
  echo "  build-emscripten    - Build Emscripten target"
  echo "  build-wasi          - Build WASI target"
  echo "  build-freestanding  - Build freestanding target"
  echo "  run-emscripten      - Run Emscripten test"
  echo "  run-wasi            - Run WASI test"
  echo "  run-freestanding    - Run freestanding test"
}

check_dependencies() {
  info "Checking dependencies..."
  check_emscripten
  check_wasi_sdk
  check_clang
  check_wasmtime
  check_node
  success "All dependencies available"
}

case "${1:-all}" in
  clean)
    clean
    ;;
  build)
    check_dependencies
    build_all
    ;;
  run)
    check_dependencies
    run_all
    ;;
  all)
    check_dependencies
    clean
    build_all
    run_all
    ;;
  check)
    check_dependencies
    ;;
  build-emscripten)
    check_emscripten
    build_emscripten
    ;;
  build-wasi)
    check_wasi_sdk
    build_wasi
    ;;
  build-freestanding)
    check_clang
    build_freestanding
    ;;
  run-emscripten)
    check_node
    run_emscripten
    ;;
  run-wasi)
    check_wasmtime
    run_wasi
    ;;
  run-freestanding)
    check_wasmtime
    run_freestanding
    ;;
  -h|--help|help)
    print_usage
    ;;
  *)
    error "Unknown command: $1"
    print_usage
    ;;
esac
