#!/bin/bash

size_data_raw=$($1 $2)
echo "$size_data_raw"

read -ra temp <<< "$(echo -e "$size_data_raw" | sed -n '2p')"

data="${temp[1]}"
bss="${temp[2]}"

bytes_used=$(("$bss + $data"))
bytes_total=16384 #16kb available
bytes_remaining=$(("$bytes_total - $bytes_used"))
bytes_threshold=2048 #need 2kb static memory left

echo "Power distribution used ${bytes_used}/${bytes_total} bytes of static memory," \
     "leaving $bytes_remaining bytes for stack space."

if [ "$bytes_remaining" -lt "$bytes_threshold" ]
then
  echo "Warning: This may not be enough for the program to run!"
fi

