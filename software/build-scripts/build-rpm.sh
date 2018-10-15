#!/bin/sh -ex

VERSION=`git --no-pager describe --tags --always | cut -d'-' -f1`
RELEASE=`git --no-pager describe --tags --always | cut -d'-' -f2`

if [ $VERSION == $RELEASE ]; then # this is a release
        RELEASE=0
fi

ARCH=$2

PATH=$PATH:/sbin/

mkdir -p SOURCES
tar -czf SOURCES/infnoise-$VERSION.tar.gz . --exclude="SOURCES"

SIGNPACKAGE=1

while test $# -gt 0
do
    case "$1" in
        --notsigned) SIGNPACKAGE=0
            ;;
    esac
    shift
done
if [ $SIGNPACKAGE -eq 1 ] ; then
	gpg --sign SOURCES/infnoise-$VERSION.tar.gz
fi

mkdir -p BUILD SPECS RPMS SRPMS

cp build-scripts/infnoise.spec build-scripts/infnoise-tools.spec SPECS
sed -i -- 's/__VERSION__/'$VERSION'/g' SPECS/infnoise.spec
sed -i -- 's/__RELEASE__/'$RELEASE'/g' SPECS/infnoise.spec
sed -i -- 's/__VERSION__/'$VERSION'/g' SPECS/infnoise-tools.spec
sed -i -- 's/__RELEASE__/'$RELEASE'/g' SPECS/infnoise-tools.spec

rpmbuild --define "_topdir `pwd`" -ba SPECS/infnoise.spec
rpmbuild --define "_topdir `pwd`" -ba SPECS/infnoise-tools.spec
