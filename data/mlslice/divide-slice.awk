#!/usr/bin/awk -f
BEGIN {filecount=0}

{current_filename = "tmp/" filecount  ".txt"}
{last_filename = "tmp/" filecount-1  ".txt"}

/====/ {filecount++;close(last_filename)}
# /slice criteria/ {print $3 > current_filename}
# !/====/ && !/slice criteria/ {print  > current_filename}
!/====/ {print  > current_filename}
