#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    exit 0
fi

# File to check (provided as argument)
file="$1"
chmod 777 "$file";

# Check if the file exists
if [ ! -f "$file" ]; then
chmod 000 "$file";
    exit 0
fi

# Check if any of the specified words are found in the file using grep
if grep -q -e "corrupted" -e "dangerous" -e "risk" -e "attack" -e "malware" -e "malicious" "$file"; then
    # Words found, return -1
    chmod 000 "$file";
    exit -1
else
    # Words not found, return 0
    chmod 000 "$file";
    exit 0
fi
