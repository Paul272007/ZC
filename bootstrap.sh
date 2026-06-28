#!/bin/bash
set -e

# --- CONFIGURATION ---
REPO_URL="https://github.com/Paul272007/ZC.git"
BRANCH="stable"

# Colors for output
GREEN='\033[0;32m'
RESET='\033[0m'

echo -e "${GREEN}========== >>> ZC Remote Installer (Branch: $BRANCH) <<< ==========${RESET}"

if ! command -v git &>/dev/null; then
    echo "Error: 'git' is required to fetch the source code."
    exit 1
fi

TEMP_DIR=$(mktemp -d)
echo "Downloading ZC into temporary directory: $TEMP_DIR"

git clone --depth 1 --branch "$BRANCH" "$REPO_URL" "$TEMP_DIR"

cd "$TEMP_DIR"
chmod +x install.sh
./install.sh

echo "Cleaning up..."
cd /
rm -rf "$TEMP_DIR"
