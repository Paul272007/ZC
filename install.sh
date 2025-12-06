#!/bin/bash

sudo mkdir -p /etc/zc

CONF=/etc/zc

sudo cp src/zc /usr/local/bin/zc

sudo chmod +x /usr/local/bin/zc
sudo chmod 775 $CONF

if [ ! -f "$CONF/conf.json" ]; then
    sudo cp src/conf.json $CONF
fi

read -p "Do you want to install some extra C libraries ? [Y/n] " choice

if [[ "$choice" == "Y" || "$choice" == "y" || "$choice" == "" ]]; then
    echo "Installing extra C libraries..."
    sudo cp lib/arrays/arrays.h /usr/local/include/arrays.h
    sudo cp lib/utils/utils.h /usr/local/include/utils.h
    sudo cp lib/colors/colors.h /usr/local/include/colors.h

    sudo cp lib/arrays/libarrays.a /usr/local/lib/libarrays.a
    sudo cp lib/utils/libutils.a /usr/local/lib/libutils.a
    echo "Extra C libraries installed."

    # Update the conf.json to include these libraries by default
    CONF_FILE="$CONF/conf.json"
    if command -v python3 >/dev/null 2>&1; then
        echo "Updating $CONF_FILE to include arrays and utils..."
        sudo python3 - <<PY
import json
from pathlib import Path
conf_file = Path("$CONF_FILE")
conf = {}
if conf_file.exists():
    try:
        conf = json.loads(conf_file.read_text())
    except Exception as e:
        print(f"Failed to parse {conf_file}: {e}")
libs = conf.get("libraries", {})
# add or update entries
libs["arrays"] = "-larrays"
libs["utils"] = "-lutils"
conf["libraries"] = libs
conf_file.write_text(json.dumps(conf, indent=4))
PY
        echo "Configuration file conf.json updated successfully. Compiling options will be automatically added."
    else
        echo "python3 is required to update conf.json automatically."
        echo "You may have to manually add '\"arrays\": \"-larrays\"' and '\"utils\": \"-lutils\"' to the 'libraries' section of $CONF_FILE"
    fi
else
    echo "Skipping extra C libraries installation."
fi

echo "Installation complete ! ✨"