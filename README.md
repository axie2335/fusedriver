# fusedriver
A lightweight, extensible filesystem driver interfaced with the Linux fuse.ko kernel module.

# dependencies
You may need to install the following packages:
libfuse-dev
libbsd-dev
pkg-config

# usage
make nufs - compiles the binary.
make mount - mounts a filesystem under mnt/ in the current directory.
make unmount - unmounts the filesystem.
make clean - cleans the executable files and object files.