#!/bin/sh

BOLD_ON="\e[1m"
UNDER_ON="\e[4m"
RED_ON="\e[31m"
GREEN_ON="\e[32m"
YELLOW_ON="\e[33m"
FMT_OFF="\e[0m"

CROSSCORE_DIR="."

EXE_DIR=bin/prog

if [ ! -d "$EXE_DIR" ]; then
	mkdir -p $EXE_DIR
fi
EXE_NAME="drac_info"
EXE_PATH="$EXE_DIR/$EXE_NAME"

CXX=${CXX:-g++}

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

printf "Compiling \"$BOLD_ON$YELLOW_ON$UNDER_ON$EXE_PATH$FMT_OFF\" \n"
rm -f $EXE_PATH

SRCS="`ls *.cpp`"
INCS="-I $CROSSCORE_DIR"
$CXX -ggdb -ffast-math -ftree-vectorize -std=c++11 $INCS $SRCS -o $EXE_PATH $*

echo -n "Build result: "
if [ -f "$EXE_PATH" ]; then
	printf "$BOLD_ON$GREEN_ON""Success""$FMT_OFF!"
else
	printf "$BOLD_ON$RED_ON""Failure""$FMT_OFF :("
fi
echo ""