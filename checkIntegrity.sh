#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    exit 0
fi

# File to check (provided as argument)
file="$1"


# Check if the file exists
if [ ! -f "$file" ]; then
    exit 0
fi
chmod 777 "$file";

# Check if any of the specified words are found in the file using grep
if grep -q -e "corrupted" -e "dangerous" -e "risk" -e "attack" -e "malware" -e "malicious" "$file"; then
    # Words found, return -1
    #echo "pattern found"
    exit -1
else
    # Iterate over each character in the file
    while IFS= read -r -n1 char; do
        # Check if the character is printable
         if (( $(printf '%d' "'$char") < 32 || $(printf '%d' "'$char") > 126)); then
            #echo "Non-printable character found: $char (ASCII: $ascii_value)"
            exit -1
        fi
    done <"$file"
fi

# Words not found, return 0
exit 0

