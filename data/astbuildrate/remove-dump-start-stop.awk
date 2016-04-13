# BEGIN {ignore=0}
# /====DUMP START/ {ignore=1; print "ignoring ..."}
# /====DUMP STOP/ {ignore=0; print "stop ignoreing .."}
# ignore=1 {print}

# END {print "end"}

!/====/ && !/benchmark/ && !/\.\// {print}
