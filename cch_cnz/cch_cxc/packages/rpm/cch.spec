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
Summary:   Installation package for CCH.
Version:   %{_prNr}
Release:   %{_rel}
License:   Ericsson Proprietary
Vendor:    Ericsson LM
Packager:  %packer
Group:     Library
BuildRoot: %_tmppath
Requires: APOS_OSCONFBIN

#BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-build

%define cch_cxc_path %{_cxcdir}

%description
Installation package for CCH.

%pre

%install
echo "Installing FIXS_CCH package"

mkdir -p $RPM_BUILD_ROOT/opt/ap/fixs/bin
mkdir -p $RPM_BUILD_ROOT/opt/ap/fixs/conf

cp %cch_cxc_path/bin/fixs_cchd $RPM_BUILD_ROOT/opt/ap/fixs/bin/fixs_cchd
cp %cch_cxc_path/bin/xcountls $RPM_BUILD_ROOT/opt/ap/fixs/bin/xcountls
cp %cch_cxc_path/bin/fwupgexec $RPM_BUILD_ROOT/opt/ap/fixs/bin/fwupgexec
cp %cch_cxc_path/bin/fwprint $RPM_BUILD_ROOT/opt/ap/fixs/bin/fwprint
cp %cch_cxc_path/bin/pfmfwprint $RPM_BUILD_ROOT/opt/ap/fixs/bin/pfmfwprint
cp %cch_cxc_path/bin/pfmupgexec $RPM_BUILD_ROOT/opt/ap/fixs/bin/pfmupgexec
cp %cch_cxc_path/bin/ipmifwprint	$RPM_BUILD_ROOT/opt/ap/fixs/bin/ipmifwprint
cp %cch_cxc_path/bin/ipmiupgexec	$RPM_BUILD_ROOT/opt/ap/fixs/bin/ipmiupgexec
cp %cch_cxc_path/bin/xcountls.sh	$RPM_BUILD_ROOT/opt/ap/fixs/bin/xcountls.sh
cp %cch_cxc_path/bin/fwupgexec.sh $RPM_BUILD_ROOT/opt/ap/fixs/bin/fwupgexec.sh
cp %cch_cxc_path/bin/fwprint.sh $RPM_BUILD_ROOT/opt/ap/fixs/bin/fwprint.sh
cp %cch_cxc_path/bin/pfmfwprint.sh $RPM_BUILD_ROOT/opt/ap/fixs/bin/pfmfwprint.sh
cp %cch_cxc_path/bin/pfmupgexec.sh $RPM_BUILD_ROOT/opt/ap/fixs/bin/pfmupgexec.sh
cp %cch_cxc_path/bin/ipmifwprint.sh $RPM_BUILD_ROOT/opt/ap/fixs/bin/ipmifwprint.sh
cp %cch_cxc_path/bin/ipmiupgexec.sh $RPM_BUILD_ROOT/opt/ap/fixs/bin/ipmiupgexec.sh

cp %cch_cxc_path/script/fixs_cch_clc $RPM_BUILD_ROOT/opt/ap/fixs/bin/

cp %cch_cxc_path/conf/AxeBladeSwManagement_mp.xml $RPM_BUILD_ROOT/opt/ap/fixs/conf/
cp %cch_cxc_path/conf/c_AxeBladeSwManagement_imm_classes.xml $RPM_BUILD_ROOT/opt/ap/fixs/conf/
cp %cch_cxc_path/conf/o_AxeBladeSwManagementBladeSwMinstances_imm_objects.xml $RPM_BUILD_ROOT/opt/ap/fixs/conf/
cp %cch_cxc_path/conf/ha_fixs_cch_objects.xml $RPM_BUILD_ROOT/opt/ap/fixs/conf/ha_fixs_cch_objects.xml

%post
if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} post-install script during installation phase"
fi
if [ $1 == 2 ]
then
        echo "This is the %{_name} package %{_rel} post-install script during upgrade phase"
fi

echo "Finalizing FIXS_CCH package installation"
ln -sf /opt/ap/fixs/bin/fixs_cchd $RPM_BUILD_ROOT/usr/bin/fixs_cchd
ln -sf /opt/ap/fixs/bin/xcountls.sh $RPM_BUILD_ROOT/usr/bin/xcountls
ln -sf /opt/ap/fixs/bin/fwupgexec.sh $RPM_BUILD_ROOT/usr/bin/fwupgexec
ln -sf /opt/ap/fixs/bin/fwprint.sh $RPM_BUILD_ROOT/usr/bin/fwprint
ln -sf /opt/ap/fixs/bin/pfmupgexec.sh $RPM_BUILD_ROOT/usr/bin/pfmupgexec
ln -sf /opt/ap/fixs/bin/pfmfwprint.sh $RPM_BUILD_ROOT/usr/bin/pfmfwprint
ln -sf /opt/ap/fixs/bin/ipmiupgexec.sh $RPM_BUILD_ROOT/usr/bin/ipmiupgexec
ln -sf /opt/ap/fixs/bin/ipmifwprint.sh $RPM_BUILD_ROOT/usr/bin/ipmifwprint
rm -Rf /cluster/FIXS/CCH/

mkdir -p $RPM_BUILD_ROOT/var/run/ap

%preun
if [ $1 == 0 ]
then
        echo "This is the %{_name} package %{_rel} pre-uninstall script during uninstall phase"		
		rm -f /usr/bin/fixs_cchd
		rm -f /usr/bin/xcountls		
		rm -f /usr/bin/fwupgexec		
		rm -f /usr/bin/fwprint		
		rm -f /usr/bin/pfmupgexec		
		rm -f /usr/bin/pfmfwprint		
		rm -f /usr/bin/ipmiupgexec		
		rm -f /usr/bin/ipmifwprint		
fi
if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} pre-uninstall script during upgrade phase"
fi

%postun
if [ $1 == 0 ]
then
        echo "This is the %{_name} package %{_rel} post-uninstall script during uninstall phase"		
		rm -f /opt/ap/fixs/bin/fixs_cchd
		rm -f /opt/ap/fixs/bin/xcountls
		rm -f /opt/ap/fixs/bin/xcountls.sh
		rm -f /opt/ap/fixs/bin/fwupgexec
		rm -f /opt/ap/fixs/bin/fwupgexec.sh
		rm -f /opt/ap/fixs/bin/fwprint
		rm -f /opt/ap/fixs/bin/fwprint.sh
		rm -f /opt/ap/fixs/bin/pfmupgexec
		rm -f /opt/ap/fixs/bin/pfmupgexec.sh
		rm -f /opt/ap/fixs/bin/pfmfwprint
		rm -f /opt/ap/fixs/bin/pfmfwprint.sh
		rm -f /opt/ap/fixs/bin/ipmiupgexec
		rm -f /opt/ap/fixs/bin/ipmiupgexec.sh
		rm -f /opt/ap/fixs/bin/ipmifwprint
		rm -f /opt/ap/fixs/bin/ipmifwprint.sh
		rm -f /opt/ap/fixs/bin/fixs_cch_clc
		rm -Rf /cluster/etc/ap/fixs/cch/

fi

if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} post-uninstall script during upgrade phase"
fi

%files
%defattr(-,root,root)
%attr(555,root,root) /opt/ap/fixs/bin/fixs_cchd
%attr(555,root,root) /opt/ap/fixs/bin/xcountls
%attr(555,root,root) /opt/ap/fixs/bin/xcountls.sh
%attr(555,root,root) /opt/ap/fixs/bin/fwupgexec
%attr(555,root,root) /opt/ap/fixs/bin/fwupgexec.sh
%attr(555,root,root) /opt/ap/fixs/bin/fwprint
%attr(555,root,root) /opt/ap/fixs/bin/fwprint.sh
%attr(555,root,root) /opt/ap/fixs/bin/pfmupgexec
%attr(555,root,root) /opt/ap/fixs/bin/pfmupgexec.sh
%attr(555,root,root) /opt/ap/fixs/bin/pfmfwprint
%attr(555,root,root) /opt/ap/fixs/bin/pfmfwprint.sh
%attr(555,root,root) /opt/ap/fixs/bin/ipmiupgexec
%attr(555,root,root) /opt/ap/fixs/bin/ipmiupgexec.sh
%attr(555,root,root) /opt/ap/fixs/bin/ipmifwprint
%attr(555,root,root) /opt/ap/fixs/bin/ipmifwprint.sh

%attr(555,root,root) /opt/ap/fixs/bin/fixs_cch_clc
/opt/ap/fixs/conf/AxeBladeSwManagement_mp.xml
/opt/ap/fixs/conf/c_AxeBladeSwManagement_imm_classes.xml
/opt/ap/fixs/conf/o_AxeBladeSwManagementBladeSwMinstances_imm_objects.xml
/opt/ap/fixs/conf/ha_fixs_cch_objects.xml

%changelog
* Wed Oct 17 2012 - andrea formica tei
- Added command scripts
* Wed Jul 07 2010 - a.f tei
- Initial implementation

