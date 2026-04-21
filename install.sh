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
CONFIG_FILE="$ZC_DIR/config.json"
CONFIG_EXISTS=false

if [ -f "$CONFIG_FILE" ]; then
  CONFIG_EXISTS=true
fi

if [ ! -d "$ZC_DIR" ]; then
  mkdir -p "$ZC_DIR/lib"
  mkdir -p "$ZC_DIR/include"
  mkdir -p "$ZC_DIR/completions"
fi

for item in etc/*; do
  item_name=$(basename "$item")

  if [ "$item_name" == "config.json" ] && [ "$CONFIG_EXISTS" == "true" ]; then
    continue
  fi

  if [ "$item_name" == "registry.json" ] && [ -f "$ZC_DIR/registry.json" ]; then
    continue
  fi

  cp -r "$item" "$ZC_DIR/"
done

# Always force updating completions
cp -r etc/completions "$ZC_DIR"

# --- PROMPT FOR CONFIGURATION ---
if [ "$CONFIG_EXISTS" == "false" ]; then
  echo -e "\n${GREEN}>>> ZC Initial configuration <<<${NC}"
  echo "Type Enter to accept default values"
  echo "You can always change your configuration by editing ~/.zc/config.json"

  prompt_str() {
    local msg=$1
    local default=$2
    local var=$3
    read -p "$(echo -e "${BLUE}?${NC} $msg ($default): ")" input
    eval $var=\"\${input:-$default}\"
  }

  prompt_bool() {
    local msg=$1
    local default=$2
    local var=$3
    local disp_default="y/N"
    [ "$default" == "true" ] && disp_default="Y/n"

    read -p "$(echo -e "${BLUE}?${NC} $msg ($disp_default): ")" input
    input=$(echo "$input" | tr '[:upper:]' '[:lower:]') # Conversion minuscule compatible Bash 3 (macOS)

    if [ -z "$input" ]; then
      eval $var="$default"
    elif [[ "$input" == "y" || "$input" == "yes" || "$input" == "true" || "$input" == "o" || "$input" == "oui" ]]; then
      eval $var="true"
    else
      eval $var="false"
    fi
  }

  prompt_str "C Compiler" "clang" VAL_C_COMP
  prompt_str "C++ Compiler" "clang++" VAL_CPP_COMP
  prompt_str "C default standard" "c17" VAL_C_STD
  prompt_str "C++ default standard" "c++20" VAL_CPP_STD
  prompt_str "Additionnal flags for compiling ?" "-Wall -Wextra" VAL_FLAGS
  prompt_str "Text editor to use" "${EDITOR:-nvim}" VAL_EDITOR
  prompt_bool "Always add standard flag ?" "false" VAL_AUTO_STD
  prompt_bool "Always clear terminal before using zc run ?" "false" VAL_CLEAR
  prompt_bool "Always keep binaries created by zc run ?" "false" VAL_KEEP
  prompt_bool "Always open file in text editor after zc create ?" "false" VAL_EDIT_CREATE
  prompt_bool "Always open project in text editor after zc init ?" "false" VAL_EDIT_INIT

  # Transform list of flags into json array
  JSON_FLAGS=""
  for flag in $VAL_FLAGS; do
    if [ -z "$JSON_FLAGS" ]; then
      JSON_FLAGS="\"$flag\""
    else
      JSON_FLAGS="$JSON_FLAGS, \"$flag\""
    fi
  done

  # Generate config file
  cat <<EOF >"$CONFIG_FILE"
{
  "c_compiler": "$VAL_C_COMP",
  "cpp_compiler": "$VAL_CPP_COMP",
  "c_std": "$VAL_C_STD",
  "cpp_std": "$VAL_CPP_STD",
  "auto_add_std": $VAL_AUTO_STD,
  "flags": [$JSON_FLAGS],
  "editor": "$VAL_EDITOR",
  "clear_before_run": $VAL_CLEAR,
  "auto_keep": $VAL_KEEP,
  "edit_on_create": $VAL_EDIT_CREATE,
  "edit_on_init": $VAL_EDIT_INIT
}
EOF
  echo -e "${GREEN}Configuration successfully generated${NC}\n"
fi
# ------------------------------------------

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

printf "You can now append 'export PATH=\"\$HOME/.zc/bin:\$PATH\"' to your ~/.bashrc or ~/.zshrc file to use commands installed via zc\n"
exit 0
