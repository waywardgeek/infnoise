VERSION=`git --no-pager describe --tags --always`
ARCH=$2

PATH=$PATH:/sbin/

mkdir -p SOURCES
tar -czf SOURCES/infnoise.tar.gz . --exclude="SOURCES"

mkdir -p BUILD SPECS RPMS SRPMS

cp build-scripts/infnoise.spec SPECS
sed -i -- 's/__VERSION__/'$VERSION'/g' SPECS/infnoise.spec

rpmbuild --define "_topdir `pwd`" -ba SPECS/infnoise.spec

