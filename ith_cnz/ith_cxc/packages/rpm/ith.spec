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
Summary:   Installation package for ITH.
Version:   %{_prNr}
Release:   %{_rel}
License:   Ericsson Proprietary
Vendor:    Ericsson LM
Packager:  %packer
Group:     Library
BuildRoot: %_tmppath
Requires: APOS_OSCONFBIN

#BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-build

%define ith_cxc_path %{_cxcdir}

%description
Installation package for ITH.

%pre

%install
echo "Installing FIXS_ITH package"

mkdir -p $RPM_BUILD_ROOT/opt/ap/fixs/bin
mkdir -p $RPM_BUILD_ROOT/opt/ap/fixs/conf

cp %ith_cxc_path/bin/fixs_ithd $RPM_BUILD_ROOT/opt/ap/fixs/bin/fixs_ithd
cp %ith_cxc_path/bin/fixs_ith_clc $RPM_BUILD_ROOT/opt/ap/fixs/bin/

cp %ith_cxc_path/bin/smxpatchexec $RPM_BUILD_ROOT/opt/ap/fixs/bin/smxpatchexec
cp %ith_cxc_path/bin/smxpatchexec.sh $RPM_BUILD_ROOT/opt/ap/fixs/bin/smxpatchexec.sh

%post
if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} post-install script during installation phase"
fi
if [ $1 == 2 ]
then
        echo "This is the %{_name} package %{_rel} post-install script during upgrade phase"
fi

echo "Finalizing FIXS_ITH package installation"
ln -sf /opt/ap/fixs/bin/fixs_ithd $RPM_BUILD_ROOT/usr/bin/fixs_ithd
ln -sf /opt/ap/fixs/bin/smxpatchexec.sh $RPM_BUILD_ROOT/usr/bin/smxpatchexec
rm -Rf /cluster/FIXS/ITH/

mkdir -p $RPM_BUILD_ROOT/var/run/ap

%preun
if [ $1 == 0 ]
then
        echo "This is the %{_name} package %{_rel} pre-uninstall script during uninstall phase"		
		rm -f /usr/bin/fixs_ithd
		rm -f /usr/bin/smxpatchexec
fi
if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} pre-uninstall script during upgrade phase"
fi

%postun
if [ $1 == 0 ]
then
        echo "This is the %{_name} package %{_rel} post-uninstall script during uninstall phase"		
		rm -f /opt/ap/fixs/bin/fixs_ithd
		rm -f /opt/ap/fixs/bin/fixs_ith_clc
		rm -f /opt/ap/fixs/bin/smxpatchexec.sh
		rm -f /opt/ap/fixs/bin/smxpatchexec
		rm -Rf /cluster/etc/ap/fixs/ith/

fi

if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} post-uninstall script during upgrade phase"
fi

%files
%defattr(-,root,root)
%attr(555,root,root) /opt/ap/fixs/bin/fixs_ithd
%attr(555,root,root) /opt/ap/fixs/bin/fixs_ith_clc
%attr(555,root,root) /opt/ap/fixs/bin/smxpatchexec
%attr(555,root,root) /opt/ap/fixs/bin/smxpatchexec.sh
%attr(555,root,root) /opt/ap/fixs/conf

%changelog
* Wed Jan  19  2016 - its
- Initial implementation

