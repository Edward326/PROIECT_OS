#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -ne 1 ]; then
    exit 0
fi

# File to check (provided as argument)
file="$1"

# Check if the file exists
if [ ! -f "$file" ]; then
    echo "SUCCES"
    exit 0
fi

chmod 777 "$file"

# Check if any of the specified words are found in the file using grep
line_count=$(wc -l < "$file")
word_count=$(wc -w < "$file")
char_count=$(wc -c < "$file")

# Check conditions using if-branch
if [ "$line_count" -lt 3 ] && [ "$word_count" -gt 100 ] && [ "$char_count" -gt 200 ]; then
    echo "CORRUPT"
    exit -1
else
    if grep -q -e "corrupted" -e "dangerous" -e "risk" -e "attack" -e "malware" -e "malicious" "$file"; then
        # Words found, return -1
        #echo "pattern found"
        echo "CORRUPT"
        exit -1
     
    #else
        # Iterate over each character in the file
     #   while IFS= read -r -n1 char; do
            # Check if the character is printable
      #      if (( $(printf '%d' "'$char") < 32 || $(printf '%d' "'$char") > 126)); then
                #echo "Non-printable character found: $char (ASCII: $ascii_value)"
       #         echo "CORRUPT"
        #        exit -1
         #   fi
        #done <"$file"
    fi
fi

# Words not found, return 0
echo "SUCCES"
exit 0
