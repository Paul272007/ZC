#!/bin/bash

sudo mkdir -p /etc/zc
sudo mkdir -p /usr/lib/zc

CONF=/etc/zc

sudo cp src/conf.json $CONF
sudo cp src/zc /usr/local/bin/zc
sudo cp src/zcsh /usr/lib/zc/zcsh