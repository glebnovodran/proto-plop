#!/bin/sh

BOLD_ON="\e[1m"
UNDER_ON="\e[4m"
RED_ON="\e[31m"
GREEN_ON="\e[32m"
YELLOW_ON="\e[33m"
FMT_OFF="\e[0m"

CROSSCORE_DIR="."

# dependencies
if [ ! -f "$CROSSCORE_DIR/crosscore.cpp" ]; then
	printf "$BOLD_ON$RED_ON""Downloading dependencies.""$FMT_OFF\n"
	XCORE_URL="https://raw.githubusercontent.com/schaban/crosscore_dev/main/"
	XCORE_FILES="src/crosscore.hpp src/crosscore.cpp exp/xcore.py"
	for xcore in $XCORE_FILES
	do
		wget $XCORE_URL/$xcore
	done
fi

