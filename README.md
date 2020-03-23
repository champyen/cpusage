# cpusage
calculate per-core cpu usage with /proc/stat on linux

build:
$ mkdir build; cd build
$ cmake $(cpuage_path)
$ make

# usage:
## default repeat 5 times, each 1000ms sample time
$ ./cpusage

change sampling time by 't' option
## run 5 times, sample time is 500ms
$ ./cpusage -t 500

change iterations by 's' option
## run 10 times
$ ./cpusage -s 10
