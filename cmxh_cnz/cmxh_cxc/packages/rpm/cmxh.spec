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

%define cmxh_cxc_path %{_cxcdir}

%description
Installation package for CMXH.

%pre

%install
echo "Installing FIXS_CMXH package"

mkdir -p $RPM_BUILD_ROOT/opt/ap/fixs/bin
mkdir -p $RPM_BUILD_ROOT/opt/ap/fixs/conf

cp %cmxh_cxc_path/bin/fixs_cmxhd $RPM_BUILD_ROOT/opt/ap/fixs/bin/fixs_cmxhd

cp %cmxh_cxc_path/script/fixs_cmxh_clc $RPM_BUILD_ROOT/opt/ap/fixs/bin/

#cp %cmxh_cxc_path/conf/AxeBladeSwManagement_mp.xml $RPM_BUILD_ROOT/opt/ap/fixs/conf/
cp %cmxh_cxc_path/conf/c_ComponentMainSwitchHandler_imm_classes.xml $RPM_BUILD_ROOT/opt/ap/fixs/conf/
cp %cmxh_cxc_path/conf/o_ComponentMainSwitchHandlerinstances_imm_objects.xml $RPM_BUILD_ROOT/opt/ap/fixs/conf/
#cp /vobs/cm4ap/fixs/cch_cnz/cch_cxc/conf/ha_fixs_cch_objects.xml $RPM_BUILD_ROOT/opt/ap/fixs/conf/ha_fixs_cch_objects.xml

%post
if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} post-install script during installation phase"
fi
if [ $1 == 2 ]
then
        echo "This is the %{_name} package %{_rel} post-install script during upgrade phase"
fi

echo "Finalizing FIXS_CMXH package installation"
ln -sf /opt/ap/fixs/bin/fixs_cmxhd $RPM_BUILD_ROOT/usr/bin/fixs_cmxhd
rm -Rf /cluster/FIXS/CMXH/

mkdir -p $RPM_BUILD_ROOT/var/run/ap

%preun
if [ $1 == 0 ]
then
        echo "This is the %{_name} package %{_rel} pre-uninstall script during uninstall phase"		
		rm -f /usr/bin/fixs_cmxhd
fi
if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} pre-uninstall script during upgrade phase"
fi

%postun
if [ $1 == 0 ]
then
        echo "This is the %{_name} package %{_rel} post-uninstall script during uninstall phase"		
		rm -f /opt/ap/fixs/bin/fixs_cmxhd
		rm -f /opt/ap/fixs/bin/fixs_cmxh_clc
		rm -Rf /cluster/etc/ap/fixs/cmxh/

fi

if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} post-uninstall script during upgrade phase"
fi

%files
%defattr(-,root,root)
%attr(555,root,root) /opt/ap/fixs/bin/fixs_cmxhd

%attr(555,root,root) /opt/ap/fixs/bin/fixs_cmxh_clc
#/opt/ap/fixs/conf/AxeBladeSwManagement_mp.xml
/opt/ap/fixs/conf/c_ComponentMainSwitchHandler_imm_classes.xml
/opt/ap/fixs/conf/o_ComponentMainSwitchHandlerinstances_imm_objects.xml
#/opt/ap/fixs/conf/ha_fixs_cch_objects.xml

%changelog
* Wed Oct 17 2012 - andrea formica tei
- Added command scripts
* Wed Jul 07 2010 - a.f tei
- Initial implementation

