#!/bin/bash
set -e

# --- CONFIGURATION ---
REPO_URL="https://github.com/Paul272007/ZC.git"
BRANCH="stable"

# Colors for output
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}===> ZC Remote Installer (Branch: $BRANCH)${NC}"

# 1. Verification of requirements
if ! command -v git &>/dev/null; then
    echo "Error: 'git' is required to fetch the source code."
    exit 1
fi

# 2. Create a temporary directory
TEMP_DIR=$(mktemp -d)
echo "Downloading ZC into temporary directory: $TEMP_DIR"

# 3. Clone only the required branch with minimum history
git clone --depth 1 --branch "$BRANCH" "$REPO_URL" "$TEMP_DIR"

# 4. Run the actual installer
# We use bash -c to ensure the script is fully loaded before execution, 
# which is safer for interactive scripts.
cd "$TEMP_DIR"
chmod +x install.sh
./install.sh

# 5. Cleanup
echo "Cleaning up..."
cd /
rm -rf "$TEMP_DIR"

echo -e "${BLUE}===> Remote installation process finished.${NC}"
