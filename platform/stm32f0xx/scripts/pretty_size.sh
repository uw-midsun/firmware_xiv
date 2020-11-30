#!/bin/bash


#./arm-none-eabi-size
size_data_raw=$($1 $2)
#echo "$size_data_raw"

#bytes_used="$bss + $data"
#bytes_total=16384 #16kb available
#bytes_remaining="$bytes_total - $bytes_used"
#bytes_threshold=2048 #need 2kb static memory left

#echo "power distribution used ${bytes_used}/${bytes_total} bytes of static memory,"
#echo "leaving $bytes_remaining bytes for stack space."

#if (bytes_remaining <= bytes_threshold){
 # echo "Warning: This may not be enough for the program to run!"
#}
