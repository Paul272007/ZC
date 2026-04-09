#!/bin/bash

echo "Warning: this will delete all the libraries installed via ZC. Back up important data before proceeding."

if [ ! -f "build/install_manifest.txt" ]; then
  echo "Error: install_manifest.txt not found. Please run the install script first."
  exit 1
fi

echo "Deleting installed files..."
xargs sudo rm -f <build/install_manifest.txt

ZC_DIR="$HOME/.zc"
rm -rf "$ZC_DIR"

echo "Uninstallation complete."
