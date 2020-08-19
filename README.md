# epstps2iap
Elan standalone Trackpoint firmware update (PS/2 Interface)
---
    Get ELAN standalone Trackpoint firmware version & Update Firmware.

Compilation
--- 
    make: to build the exectue project "epstps2iap".
    $ make
   
   
Run
---
Get Firmware Version :
  ./epstps2_updater -g
  
Get Module ID :
  ./epstps2_updater -m
  
Update Firmware : 
  ./epstps2_updater -a {bin_rankA_file}  -b {bin_rankB_file} -c {bin_rankC_file} -u
