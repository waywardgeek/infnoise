Name:           infnoise
Version:        __VERSION__
Release:        __RELEASE__
Summary:        Infinite Noise TRNG
Group:          Applications/Security
License:        GPL
URL:            https://github.com/manuel-domke/infnoise
Vendor:         13-37.org
Source:		infnoise.tar.gz
Prefix:         %{_prefix}
Packager: 	Manuel Domke
BuildRoot:      %{_tmppath}/%{name}-root

%description

%prep
tar -xzf ../SOURCES/infnoise.tar.gz

%build
make

%install
#make DESTDIR=$RPM_BUILD_ROOT install
install -Dvm755 "infnoise" "$RPM_BUILD_ROOT/usr/bin/infnoise"
install -Dvm644 "init_scripts/infnoise.conf.systemd" "$RPM_BUILD_ROOT/etc/infnoise.conf"
install -Dvm644 "init_scripts/75-infnoise.rules" "$RPM_BUILD_ROOT/usr/lib/udev/rules.d/75-infnoise.rules"
install -Dvm644 "init_scripts/infnoise.service.bin" "$RPM_BUILD_ROOT/usr/lib/systemd/system/infnoise.service"

%post
systemctl daemon-reload
systemctl enable infnoise.service
systemctl daemon-reload

%preun
systemctl stop infnoise.service >/dev/null 2>&1
systemctl disable infnoise.service
systemctl daemon-reload

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/infnoise
/etc/infnoise.conf
%{_prefix}/lib/udev/rules.d/75-infnoise.rules
%{_prefix}/lib/systemd/system/infnoise.service
