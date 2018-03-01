#!/bin/bash

VERSION=`git --no-pager describe --tags --always | cut -d'-' -f1`
PKGREL=`git --no-pager describe --tags --always | cut -d'-' -f2`

if [ $VERSION == $PKGREL ]; then # this is a release
	PKGREL=0
fi

# x86_64
mkdir -p x86_64
cd x86_64

cp ../build-scripts/PKGBUILD.arch PKGBUILD
cp ../build-scripts/INSTALL.arch INSTALL
echo "pkgver=$VERSION" >> PKGBUILD
echo "pkgrel=$PKGREL" >> PKGBUILD
echo "arch=('x86_64')" >> PKGBUILD
ls -lah
makepkg -f --sign --key 975DC25C4E730A3C
cd ..

# x86_64
mkdir -p x86
cd x86
cp ../build-scripts/PKGBUILD.arch PKGBUILD
cp ../build-scripts/INSTALL.arch INSTALL
echo "pkgver=$VERSION" >> PKGBUILD
echo "pkgrel=$PKGREL" >> PKGBUILD
echo "arch=('i686')" >> PKGBUILD
makechrootpkg -r /x86 -U jenkins -- --sign --key 975DC25C4E730A3C
