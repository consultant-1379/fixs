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
Summary:   Installation package for TREFC.
Version:   %{_prNr}
Release:   %{_rel}
License:   Ericsson Proprietary
Vendor:    Ericsson LM
Packager:  %packer
Group:     Library
BuildRoot: %_tmppath
Requires: APOS_OSCONFBIN

#BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-build

%define trefc_cxc_path %{_cxcdir}

%description
Installation package for TREFC.

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
echo "Installing FIXS_TREFC package"
if [ ! -d $RPM_BUILD_ROOT/opt/ap/fixs/bin ]
then
	mkdir -p $RPM_BUILD_ROOT/opt/ap/fixs/bin
fi

if [ ! -d $RPM_BUILD_ROOT/opt/ap/fixs/conf ]
then
	mkdir -p $RPM_BUILD_ROOT/opt/ap/fixs/conf
fi

cp %trefc_cxc_path/bin/fixs_trefcd	$RPM_BUILD_ROOT/opt/ap/fixs/bin/fixs_trefcd
cp -af %trefc_cxc_path/conf/c_AxeTimeReference_imm_classes.xml	$RPM_BUILD_ROOT/opt/ap/fixs/conf/c_AxeTimeReference_imm_classes.xml
cp -af %trefc_cxc_path/conf/o_AxeTimeReferenceInstances_imm_objects.xml	$RPM_BUILD_ROOT/opt/ap/fixs/conf/o_AxeTimeReferenceInstances_imm_objects.xml
cp -af %trefc_cxc_path/conf/AxeTimeReference_mp.xml $RPM_BUILD_ROOT/opt/ap/fixs/conf/AxeTimeReference_mp.xml
cp %trefc_cxc_path/script/fixs_trefc_clc $RPM_BUILD_ROOT/opt/ap/fixs/bin/fixs_trefc_clc
cp %trefc_cxc_path/conf/ha_fixs_trefc_objects.xml $RPM_BUILD_ROOT/opt/ap/fixs/conf/ha_fixs_trefc_objects.xml

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
chmod +x /opt/ap/fixs/bin/fixs_trefcd 

ln -sf /opt/ap/fixs/bin/fixs_trefcd $RPM_BUILD_ROOT/usr/bin/fixs_trefcd

%preun
if [ $1 == 0 ]
then
        echo "This is the %{_name} package %{_rel} pre-uninstall script during uninstall phase"
		rm -f /usr/bin/fixs_trefcd
fi
if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} pre-uninstall script during upgrade phase"
fi
 
%postun
if [ $1 == 0 ]
then
        echo "This is the %{_name} package %{_rel} post-uninstall script during uninstall phase"
		rm -f /opt/ap/fixs/bin/fixs_trefcd
		rm -f /opt/ap/fixs/bin/fixs_trefc_clc
		rm -Rf /cluster/etc/ap/fixs/trefc/
fi

if [ $1 == 1 ]
then
        echo "This is the %{_name} package %{_rel} post-uninstall script during upgrade phase"
fi

%files
%defattr(-,root,root)
%attr(755,root,root) /opt/ap/fixs/bin/fixs_trefcd
%attr(755,root,root) /opt/ap/fixs/bin/fixs_trefc_clc
%attr(0755,root,root) /opt/ap/fixs/conf/c_AxeTimeReference_imm_classes.xml
%attr(0755,root,root) /opt/ap/fixs/conf/o_AxeTimeReferenceInstances_imm_objects.xml
%attr(0755,root,root) /opt/ap/fixs/conf/AxeTimeReference_mp.xml
%attr(0755,root,root) /opt/ap/fixs/conf/ha_fixs_trefc_objects.xml

%changelog
* Wed Jul 07 2010 - nicola.muto (at) its.na.it
- Initial implementation

