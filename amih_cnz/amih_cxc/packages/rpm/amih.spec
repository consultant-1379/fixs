#
# spec file for configuration of package apache
#
# Copyright  (c)  2010  Ericsson LM
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to paolo.palmieri@ericsson.com
#

Name:      %{_name}
Summary:   Installation package for AMIH.
Version:   %{_prNr}
Release:   %{_rel}
License:   Ericsson Proprietary
Vendor:    Ericsson LM
Packager:  %packer
Group:     Library
BuildRoot: %_tmppath
Requires: APOS_OSCONFBIN

#BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-build

%define amih_cxc_path %{_cxcdir}

%description
Installation package for AMIH.

%pre
if [ $1 == 1 ]
then
	echo "This is the %{_name} package %{_rel} pre-install script during installation phase"
fi
if [ $1 == 2 ]
then
	echo "This is the %{_name} package %{_rel} pre-install script during upgrade phase"
fi

%install
echo "Installing FIXS_AMIH package"
if [ ! -d $RPM_BUILD_ROOT/opt/ap/fixs/bin ]
then
	mkdir -p $RPM_BUILD_ROOT/opt/ap/fixs/bin
fi

if [ ! -d $RPM_BUILD_ROOT/opt/ap/fixs/conf ]
then
	mkdir -p $RPM_BUILD_ROOT/opt/ap/fixs/conf
fi

cp %amih_cxc_path/bin/fixs_amihd	$RPM_BUILD_ROOT/opt/ap/fixs/bin/fixs_amihd
cp %amih_cxc_path/script/fixs_amih_clc $RPM_BUILD_ROOT/opt/ap/fixs/bin/fixs_amih_clc

%post
if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} post-install script during installation phase"
fi
if [ $1 == 2 ]
then
        echo "This is the %{_name} package %{_rel} post-install script during upgrade phase"
fi

#Start chmod
chmod +x /opt/ap/fixs/bin/fixs_amihd 

ln -sf /opt/ap/fixs/bin/fixs_amihd $RPM_BUILD_ROOT/usr/bin/fixs_amihd

%preun
if [ $1 == 0 ]
then
        echo "This is the %{_name} package %{_rel} pre-uninstall script during uninstall phase"
		rm -f /usr/bin/fixs_amihd
fi
if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} pre-uninstall script during upgrade phase"
fi

%postun
if [ $1 == 0 ]
then
    echo "This is the %{_name} package %{_rel} post-uninstall script during uninstall phase"
	rm -f /opt/ap/fixs/bin/fixs_amihd
	rm -f /opt/ap/fixs/bin/fixs_amih_clc
	rm -Rf /cluster/etc/ap/fixs/amih/
fi

if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} post-uninstall script during upgrade phase"
fi

%files
%defattr(-,root,root)
%attr(755,root,root) /opt/ap/fixs/bin/fixs_amihd
%attr(755,root,root) /opt/ap/fixs/bin/fixs_amih_clc

%changelog
* Wed Feb 22 2010 - s.v tei
- Initial implementation

