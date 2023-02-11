#!/bin/sh -ex

ARCH=$1
VERSION=`git --no-pager describe --tags --always`

PATH=$PATH:/sbin/

make -f Makefile.linux clean
make -f Makefile.linux

rm -rf build
mkdir -p build/DEBIAN
cp build-scripts/control.debian.infnoise build/DEBIAN/control
cp build-scripts/infnoise.postinst build/DEBIAN/postinst
chmod 775 build/DEBIAN/postinst
echo "Version: $VERSION" >> build/DEBIAN/control
echo "Architecture: $ARCH" >> build/DEBIAN/control

mkdir -p build/usr/sbin/
mkdir -p build/etc/udev/rules.d/
mkdir -p build/lib/systemd/system/

cp infnoise build/usr/sbin/
cp init_scripts/infnoise.conf.systemd build/etc/infnoise.conf
cp init_scripts/75-infnoise.rules build/etc/udev/rules.d/75-infnoise.rules
cp init_scripts/infnoise.service.sbin build/lib/systemd/system/infnoise.service

if [ ! -e build/usr/sbin/infnoise ] ; then
	echo "$2 binary missing"
	exit 1;
fi

fakeroot dpkg-deb -Zxz -b build/ infnoise_${VERSION}_${ARCH}.deb

### build infnoise-tools ###
rm -rf build

cd tools
mkdir -p build/usr/bin/

make -f Makefile

cp passgen build/usr/bin/infnoise-passgen
cp dice build/usr/bin/infnoise-dice
cp entcheck build/usr/bin/infnoise-entcheck
cp healthcheck build/usr/bin/infnoise-healthcheck
cp hex2bin build/usr/bin/infnoise-hex2bin
cp bin2hex build/usr/bin/infnoise-bin2hex
cp findlongest build/usr/bin/infnoise-findlongest
cp flipbits build/usr/bin/infnoise-flipbits

mkdir -p build/DEBIAN
cp ../build-scripts/control.debian.tools build/DEBIAN/control
echo "Version: $VERSION" >> build/DEBIAN/control
echo "Architecture: $ARCH" >> build/DEBIAN/control

fakeroot dpkg-deb -Zxz -b build/ infnoise-tools_${VERSION}_${ARCH}.deb

rm -rf build
cd ..

### build libinfnoise ###
rm -rf build

mkdir -p build/usr/lib
mkdir -p build/usr/include

make -f Makefile.linux libinfnoise.so

cp libinfnoise.so build/usr/lib/
cp libinfnoise.h build/usr/include

mkdir -p build/DEBIAN
cp build-scripts/control.debian.lib build/DEBIAN/control
echo "Version: $VERSION" >> build/DEBIAN/control
echo "Architecture: $ARCH" >> build/DEBIAN/control

fakeroot dpkg-deb -Zxz -b build/ libinfnoise_${VERSION}_${ARCH}.deb

rm -rf build
