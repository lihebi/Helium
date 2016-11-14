#!/usr/bin/awk -f

BEGIN {
    isprefix=true;
    insection=false;
    prefixfilename="prefix.txt"
    temp="";
}

/HELIUM_POI_INSTRUMENT/ {temp=""; inprefix=false;}

{
    if (inprefix) {
        print
    } else {
        temp+=$1
    }
}
