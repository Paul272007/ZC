#!/bin/bash

set -e # Stop on error

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}========== ZC Installation ==========${NC}"

# Detect OS and install dependencies
echo -e "${BLUE}[1/6] Installing dependencies...${NC}"

if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "Detected macOS."
  if ! command -v brew &>/dev/null; then
    echo "Homebrew not found. Please install it first at https://brew.sh/"
    exit 1
  fi
  brew install cmake git libarchive openssl curl
elif [ -f /etc/debian_version ]; then
  echo "Detected Debian-based system."
  sudo apt-get update -qq
  sudo apt-get install -y clang libclang-dev llvm-dev cmake make git libz-dev libcurl4-openssl-dev libarchive-dev libssl-dev
  echo "Detected Red Hat-based system."
  sudo dnf install -y clang clang-devel cmake make git libcurl-devel zlib-devel libarchive-devel openssl-devel
elif [ -f /etc/arch-release ]; then
  echo "Detected Arch-based system."
  sudo pacman -S --needed --noconfirm clang cmake make git llvm libedit libarchive openssl curl
elif [ -f /etc/os-release ] && grep -q "suse" /etc/os-release; then
  echo "Installation for openSUSE..."
  sudo zypper install -y clang clang-devel cmake make git zlib-devel libcurl-devel libarchive-devel libopenssl-devel
else
  echo -e "${RED}Error: Unsupported operating system.${NC}"
  exit 1
fi

echo -e "${BLUE}[2/7] Setting up user environment...${NC}"
ZC_DIR="$HOME/.zc"
CONFIG_FILE="$ZC_DIR/zc.json"
CONFIG_EXISTS=false

if [ -f "$CONFIG_FILE" ]; then
  CONFIG_EXISTS=true
fi

if [ ! -d "$ZC_DIR" ]; then
  mkdir -p "$ZC_DIR/lib"
  mkdir -p "$ZC_DIR/include"
  mkdir -p "$ZC_DIR/completions"
  mkdir -p "$ZC_DIR/bin"
fi

for item in etc/*; do
  item_name=$(basename "$item")

  if [ "$item_name" == "zc.json" ] && [ "$CONFIG_EXISTS" == "true" ]; then
    continue
  fi

  if [ "$item_name" == "registry.json" ] && [ -f "$ZC_DIR/registry.json" ]; then
    continue
  fi

  cp -r "$item" "$ZC_DIR/"
done

# Always force updating completions
cp -r etc/completions "$ZC_DIR"
# Ensure bin directory exists
mkdir -p "$ZC_DIR/bin"

# --- PROMPT FOR CONFIGURATION ---
if [ "$CONFIG_EXISTS" == "false" ]; then
  echo -e "\n${GREEN}>>> ZC Initial configuration <<<${NC}"
  echo "Type Enter to accept default values"
  echo "You can always change your configuration by editing ~/.zc/zc.json"

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
  prompt_str "Username" "user" VAL_USERNAME
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
  "username": "$VAL_USERNAME",
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
echo -e "${BLUE}[3/7] Cleaning existing installation...${NC}"

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
echo -e "${BLUE}[4/7] Compiling source code...${NC}"
cmake --build . --config Release --parallel "$(nproc)"

# Installation
echo -e "${BLUE}[5/7] Installing ZC...${NC}"
if [ "$EUID" -ne 0 ]; then
  sudo cmake --install .
else
  cmake --install .
fi

echo -e "${BLUE}[6/7] Configuring Clangd...${NC}"

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

echo -e "${BLUE}[7/7] Updating shell configuration...${NC}"

# Helper to update RC files
update_rc() {
  local rc_file="$1"
  local config_block="$2"
  local check_string="$3"

  if [ -f "$rc_file" ]; then
    if ! grep -Fq "$check_string" "$rc_file"; then
      echo -e "\n# ZC configuration\n$config_block" >>"$rc_file"
      echo "Updated $rc_file"
    else
      echo "$rc_file already configured."
    fi
  fi
}

# Zsh configuration
ZSH_RC="$REAL_HOME/.zshrc"
ZSH_BLOCK="export PATH=\"\$HOME/.zc/bin:\$PATH\"
fpath=(\$HOME/.zc/completions \$fpath)
# In some cases, you might need to run 'rm -f ~/.zcompdump && compinit' if completions don't show up
autoload -Uz compinit && compinit"
update_rc "$ZSH_RC" "$ZSH_BLOCK" ".zc/completions"

# Bash configuration
BASH_RC="$REAL_HOME/.bashrc"
BASH_BLOCK="export PATH=\"\$HOME/.zc/bin:\$PATH\""
update_rc "$BASH_RC" "$BASH_BLOCK" ".zc/bin"

# Global Profile configuration (for GUI sessions and login shells)
PROFILE="$REAL_HOME/.profile"
PROFILE_BLOCK="if [ -d \"\$HOME/.zc/bin\" ] ; then
    PATH=\"\$HOME/.zc/bin:\$PATH\"
fi"
update_rc "$PROFILE" "$PROFILE_BLOCK" ".zc/bin"

# Systemd environment configuration (for services like Waybar, systemd --user)
ENV_D_DIR="$REAL_HOME/.config/environment.d"
if [ ! -d "$ENV_D_DIR" ]; then
  mkdir -p "$ENV_D_DIR"
  chown "$REAL_USER:$(id -gn "$REAL_USER")" "$ENV_D_DIR"
fi

ENV_ZC_CONF="$ENV_D_DIR/60-zc.conf"
ENV_BLOCK="PATH=\$HOME/.zc/bin:\$PATH"

if [ ! -f "$ENV_ZC_CONF" ]; then
  echo "$ENV_BLOCK" >"$ENV_ZC_CONF"
  chown "$REAL_USER:$(id -gn "$REAL_USER")" "$ENV_ZC_CONF"
  echo "Created $ENV_ZC_CONF for systemd user environment."
else
  if ! grep -Fq ".zc/bin" "$ENV_ZC_CONF"; then
    echo "$ENV_BLOCK" >>"$ENV_ZC_CONF"
    echo "Updated $ENV_ZC_CONF"
  fi
fi

echo -e "${GREEN}===== ZC installed successfully! =====${NC}"
echo -e "Changes detected for: zsh, bash, .profile and systemd environment."
echo -e "Please log out and log back in to ensure all services (Waybar, etc.) see the new PATH."
exit 0
