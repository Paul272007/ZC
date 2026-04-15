#!/bin/bash

set -e # Stop on error

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}========== ZC Installation ==========${NC}"

# Detect OS
echo -e "${BLUE}[0/5] Installing dependencies...${NC}"
if [ -f /etc/debian_version ]; then
  echo "Detected Debian-based system."
  sudo apt-get update -qq
  sudo apt-get install -y clang libclang-dev llvm-dev cmake make git libz-dev libcurl4-openssl-dev
elif [ -f /etc/redhat-release ] || [ -f /etc/fedora-release ]; then
  echo "Detected Red Hat-based system."
  sudo dnf install -y clang clang-devel cmake make git libcurl-devel zlib-devel
elif [ -f /etc/arch-release ]; then
  echo "Detected Arch-based system."
  sudo pacman -S --needed --noconfirm clang cmake make git llvm libedit
elif [ -f /etc/os-release ] && grep -q "suse" /etc/os-release; then
  echo "Installation for openSUSE..."
  sudo zypper install -y clang clang-devel cmake make git zlib-devel libcurl-devel
else
  echo -e "${RED}Error: Unsupported operating system.${NC}"
  exit 1
fi

echo -e "${BLUE}[1/5] Setting up user environment...${NC}"
ZC_DIR="$HOME/.zc"

# Copy configuration
if [ ! -d "$ZC_DIR" ]; then
  mkdir -p "$ZC_DIR/lib"
  mkdir -p "$ZC_DIR/include"
  cp -r etc/* "$ZC_DIR"
fi

# Build and copy source files, and clean up build artifacts
echo -e "${BLUE}[2/5] Cleaning existing installation...${NC}"

BIN="/usr/local/bin/zc"
# Clean up any existing installation
if [ "$EUID" -ne 0 ]; then
  sudo rm -f "$BIN"
else
  rm -f "$BIN"
fi

if [ -d "build" ]; then
  rm -rf build/
fi

mkdir build/
cd build/
echo "Configuration..."
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DENABLE_DEBUG=OFF ..

# Parallel compilation to compile faster
echo -e "${BLUE}[3/5] Compiling source code...${NC}"
cmake --build . --config Release --parallel "$(nproc)"

# Installation
echo -e "${BLUE}[4/5] Installing ZC...${NC}"
if [ "$EUID" -ne 0 ]; then
  sudo cmake --install .
else
  cmake --install .
fi

echo -e "${BLUE}[5/5] Configuring Clangd...${NC}"

if [ -n "$SUDO_USER" ]; then
  REAL_USER="$SUDO_USER"
  REAL_HOME=$(getent passwd "$SUDO_USER" | cut -d: -f6)
else
  REAL_USER="$USER"
  REAL_HOME="$HOME"
fi

CLANGD_DIR="$REAL_HOME/.config/clangd"
CLANGD_CONFIG="$CLANGD_DIR/config.yaml"
ZC_INCLUDE_PATH="$REAL_HOME/.zc/include"

if [ ! -d "$CLANGD_DIR" ]; then
  mkdir -p "$CLANGD_DIR"
  chown "$REAL_USER:$(id -gn "$REAL_USER")" "$CLANGD_DIR"
fi

CONFIG_BLOCK="CompileFlags:
  Add: [-I$ZC_INCLUDE_PATH]"

if [ ! -f "$CLANGD_CONFIG" ]; then
  echo "$CONFIG_BLOCK" >"$CLANGD_CONFIG"
  echo "Created clangd configuration."
else
  if grep -Fq "$ZC_INCLUDE_PATH" "$CLANGD_CONFIG"; then
    echo "Clangd configuration already present."
  else
    echo -e "\n---\n$CONFIG_BLOCK" >>"$CLANGD_CONFIG"
    echo "Appended to existing clangd configuration."
  fi
fi

chown "$REAL_USER:$(id -gn "$REAL_USER")" "$CLANGD_CONFIG"

echo -e "${GREEN}===== ZC installed successfully! =====${NC}"

printf "You can now append 'export PATH=\"\$HOME/.zc/bin:\$PATH\"' to your ~/.bashrc or ~/.zshrc file to use commands installed via zc"
exit 0
