#!/bin/bash
set -e

RED='\033[0;31m'
BLUE='\033[0;34m'
GREEN='\033[0;32m'
RESET='\033[0m'

echo -e "${GREEN}========== >>> ZC Installation <<< ==========${RESET}\n"
echo -e "${BLUE}[1/4] Installing dependencies...${RESET}"

if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "Detected macOS."
  if ! command -v brew &>/dev/null; then
    echo "Homebrew not found. Please install it first at https://brew.sh/"
    exit 1
  fi
  brew install cmake git libarchive openssl curl nlohmann-json
elif [ -f /etc/debian_version ]; then
  echo "Detected Debian-based system."
  sudo apt-get update -qq
  sudo apt-get install -y clang libclang-dev llvm-dev cmake make git libz-dev libcurl4-openssl-dev libarchive-dev libssl-dev nlohmann-json3-dev
elif [ -f /etc/redhat-release ]; then
  echo "Detected Red Hat-based system."
  sudo dnf install -y clang clang-devel cmake make git libcurl-devel zlib-devel libarchive-devel openssl-devel nlohmann-json-devel
elif [ -f /etc/arch-release ]; then
  echo "Detected Arch-based system."
  sudo pacman -S --needed --noconfirm clang cmake make git llvm libedit libarchive openssl curl nlohmann-json
elif [ -f /etc/os-release ] && grep -q "suse" /etc/os-release; then
  echo "Installation for openSUSE..."
  sudo zypper install -y clang clang-devel cmake make git zlib-devel libcurl-devel libarchive-devel libopenssl-devel nlohmann_json-devel
else
  echo -e "${RED}Error: Unsupported operating system.${RESET}"
  exit 1
fi

echo -e "${BLUE}[2/4] Cleaning existing installation...${RESET}"

BIN="/usr/bin/zc"
if [ "$EUID" -ne 0 ]; then
  sudo rm -f "$BIN"
else
  rm -f "$BIN"
fi
if [ -d "build" ]; then
  if [ "$EUID" -ne 0 ]; then
    sudo rm -rf build/
  else
    rm -rf build/
  fi
fi
mkdir build/
cd build/
echo "Configuration..."
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_DEBUG=OFF ..

echo -e "${BLUE}[3/4] Installing ZC...${RESET}"

cmake --build . --config Release --parallel "$(nproc)"
if [ "$EUID" -ne 0 ]; then
  sudo cmake --install .
else
  cmake --install .
fi

echo -e "${BLUE}[4/4] Setting up configuration...${RESET}"

ZC_DIR="$HOME/.zc"
if [ ! -d "$ZC_DIR" ]; then
  mkdir -p "$ZC_DIR"
  cp -r ../etc/project_templates "$ZC_DIR/"
  cp -r ../etc/templates         "$ZC_DIR/"
  cp    ../etc/config.json       "$ZC_DIR/"
fi

if [ "$EUID" -ne 0 ]; then
  sudo mkdir -p /usr/share/zsh/site-functions/
  sudo cp -r ../etc/completions/* /usr/share/zsh/site-functions/
else
  mkdir -p /usr/share/zsh/site-functions/
  cp -r ../etc/completions/* /usr/share/zsh/site-functions/
fi

echo -e "${GREEN}========== >>> ZC installed successfully! <<< ==========${RESET}\n"
echo -e "You can also configure:"
echo -e "  - clangd by appending -I\$HOME/.zc/include to ~/.config/clangd/config.yaml"
echo -e "  - your shell by appending \$HOME/.zc/bin to your \$PATH"
exit 0
