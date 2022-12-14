#!/bin/bash
make Debug
if [ "$?" != "0" ]; then
	exit -1
fi

#LIBDIR=$(pkg-config --variable libdir libudjat)
#sudo ln -sf $(readlink -f .bin/Debug/lib*.so.*) ${LIBDIR}
#if [ "$?" != "0" ]; then
#	exit -1
#fi

MODULEDIR=$(pkg-config --variable module_path libudjat)
mkdir -p ${MODULEDIR}
sudo ln -sf $(readlink -f .bin/Debug/udjat-module-*.so) ${MODULEDIR}
if [ "$?" != "0" ]; then
	exit -1
fi

