# grib2dec
### Grib2 format decoder

grib2dec is a C/C++ library to parse files in grib2 format.

## What it does ?

 * For now, __complex packing__ and __complex packing with spatial differencing__ data representations are implemented.

 * Spatial filter capability : fetch data from a subregion

 * C and C++ interfaces

## Why ?

Because I was stuck with g2clib, the C library to decode grib2, that didn't work.
I hope this library will be easy to extand when needed.

## How ?

 * Compile library and demo program with ```cmake && make```. Install it with ```make install```.

 * Library usage exemple: see apps/grib2dec.cpp

 * demo program : run ```./apps/grib2dec``` for usage help.

## Grib2 ressources

 * grib2 format documentation from noaa : https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/

 * NOAA winds download:
   https://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_1p00.pl
   (select "10m above ground", and _UGRD_ and _VGRD_ variables).

 * Files Usage help for virtual-regatta :
   http://www.tecepe.com.br/wiki/index.php?title=NOAAWinds
