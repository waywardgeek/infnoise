#!/bin/sh -ex

VERSION=`git --no-pager describe --tags --always | cut -d'-' -f1`
PKGREL=`git --no-pager describe --tags --always | cut -d'-' -f2`

GITREPO=`git config --get remote.origin.url`

if [ $VERSION == $PKGREL ]; then # this is a release
	PKGREL=0
fi


SIGNPACKAGE=true
while test $# -gt 0
do
    case "$1" in
        --notsigned) SIGNPACKAGE=false
            ;;
    esac
    shift
done


# x86_64
mkdir -p x86_64
cd x86_64

cp ../build-scripts/PKGBUILD.arch PKGBUILD
cp ../build-scripts/INSTALL.arch INSTALL

sed -i "s|.*source.*=.*(.*).*|source=('git+$GITREPO')|g" PKGBUILD
echo "pkgver=$VERSION.$PKGREL" >> PKGBUILD
echo "pkgrel=1" >> PKGBUILD
echo "arch=('x86_64')" >> PKGBUILD

if [ "$SIGNPACKAGE" == true ]; then
  makepkg -f --sign --key 975DC25C4E730A3C
else
  makepkg -f
fi

cd ..

# x86
mkdir -p x86
cd x86

cp ../build-scripts/PKGBUILD.arch PKGBUILD
cp ../build-scripts/INSTALL.arch INSTALL

echo "pkgver=$VERSION.$PKGREL" >> PKGBUILD
echo "pkgrel=1" >> PKGBUILD
echo "arch=('i686')" >> PKGBUILD
makechrootpkg -r /x86 -U jenkins -- --sign --key 975DC25C4E730A3C
