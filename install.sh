#!/bin/bash

sudo mkdir -p /etc/zc
sudo mkdir -p /usr/lib/zc

CONF=/etc/zc

sudo cp src/conf.json $CONF
sudo cp src/zc /usr/local/bin/zc
sudo cp src/zcsh /usr/lib/zc/zcsh

sudo chmod +x /usr/local/bin/zc
sudo chmod +x /usr/lib/zc/zcsh
sudo chmod 644 $CONF

read -p "Do you want to install some extra C libraries ? [Y/n] " choice

if [[ "$choice" == "Y" || "$choice" == "y" || "$choice" == "" ]]; then
    echo "Installing extra C libraries..."
    sudo cp lib/arrays/arrays.h /usr/local/include/arrays.h
    sudo cp lib/utils/utils.h /usr/local/include/utils.h

    sudo cp lib/arrays/libarrays.a /usr/local/lib/libarrays.a
    sudo cp lib/utils/libutils.a /usr/local/lib/libutils.a
else
    echo "Skipping extra C libraries installation."
fi

echo "Installation complete."