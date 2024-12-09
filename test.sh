#!/bin/bash

RED='\033[0;31m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
GRAY='\033[1;30m'
RESET='\033[0m'

find res/test-case -type f -name "*.func" | while read -r file; do
    echo -e "${CYAN}Test input file: ${BLUE}$file${RESET}"
    echo -e "${GRAY}"
    cat "$file"
    echo -e "${RESET}"
    echo -e "${RED}Compiler output: ${RESET}"
    ./src/build/compiler < "$file"
done
