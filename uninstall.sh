#!/bin/bash

echo "Warning: this will remove all the packages installed via ZC. Back up important data before proceeding."

sudo rm -rf "$HOME/.zc"
sudo rm -f /usr/bin/zc
sudo rm -f /usr/share/zsh/site-functions/_zc

echo "Uninstallation complete."
