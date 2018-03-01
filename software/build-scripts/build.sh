#!/bin/bash

ARCH=$1
VERSION=`git --no-pager describe --tags --always`

PATH=$PATH:/sbin/

make clean
make

rm -rf build
mkdir -p build/DEBIAN
cp build-scripts/control.debian.$ARCH build/DEBIAN/control
cp build-scripts/infnoise.postinst build/DEBIAN/postinst
chmod 775 build/DEBIAN/postinst
echo "Version: $VERSION" >> build/DEBIAN/control

mkdir -p build/usr/sbin/
mkdir -p build/etc/udev/rules.d/
mkdir -p build/etc/systemd/system/

cp infnoise build/usr/sbin/
cp init_scripts/infnoise.conf.systemd build/etc/infnoise.conf
cp init_scripts/75-infnoise.rules build/etc/udev/rules.d/75-infnoise.rules
cp init_scripts/infnoise.service.sbin build/etc/systemd/system/infnoise.service

if [ ! -e build/usr/sbin/infnoise ] ; then
	echo "$2 binary missing"
	exit 1;
fi

# debuild -b -uc -us
dpkg -b build/ infnoise_${VERSION}_${ARCH}.deb
debuild -uc -us

rm -r build
