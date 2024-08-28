#
# spec file for configuration of package FIXELH commands
#
# Copyright  (c)  2010  Ericsson LM
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to paolo.palmieri@ericsson.com
#                                     giovanni.gambardella@ericsson.com
#

Name:      %{_name}
Summary:   Installation package for FIXELH commands.
Version:   %{_prNr}
Release:   %{_rel}
License:   Ericsson Proprietary
Vendor:    Ericsson LM
Packager:  %packer
Group:     Application
BuildRoot: %_tmppath
Requires: APOS_OSCONFBIN

# This will make rpm not find the dependencies 
# TODO: should ignore dependecies?
Autoreq: 0

%define fixelh_cxc_path %{_cxcdir}
%define fixelh_cxc_bin %{fixelh_cxc_path}/bin
%define FIXSBINdir /opt/ap/fixs/bin

%description
Installation package for FIXELH commands.

%pre
if [ $1 == 1 ] 
then
    echo "This is the %{_name} package %{_rel} pre-install script during installation phase"
fi

if [ $1 == 2 ]
then
    echo "This is the %{_name} package %{_rel} pre-install script during upgrade phase"
    # Remove the soft link created before
    rm -f %{_bindir}/fixerls
    # other real files will be overrided
fi

%install
echo "This is the %{_name} package %{_rel} install script"

# Create necessary directories if needed
#rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{FIXSBINdir}

# Copy fixerls command to appropiate directories
cp %{fixelh_cxc_bin}/fixerls.sh $RPM_BUILD_ROOT/%{FIXSBINdir}/fixerls.sh
cp %{fixelh_cxc_bin}/fixerls $RPM_BUILD_ROOT/%{FIXSBINdir}/fixerls

%post
if [ $1 == 1 ]
then
    echo "This is the %{_name} package %{_rel} post-install script during installation phase"
fi

if [ $1 == 2 ]
then
    echo "This is the %{_name} package %{_rel} post-install script during upgrade phase"
fi

chmod +x %{FIXSBINdir}/fixerls.sh
chmod +x %{FIXSBINdir}/fixerls
ln -sf %{FIXSBINdir}/fixerls.sh %{_bindir}/fixerls

%preun
if [ $1 == 0 ]
then
    echo "This is the %{_name} package %{_rel} pre-uninstall script during unistall phase"
fi

if [ $1 == 1 ]
then
    echo "This is the %{_name} package %{_rel} pre-uninstall script during upgrade phase"
fi

%postun
if [ $1 == 0 ]
then
    echo "This is the %{_name} package %{_rel} post-uninstall script uninstall phase"
    # remove the soft link
    rm -f %{_bindir}/fixerls
fi

if [ $1 == 1 ]
then
    echo "This is the %{_name} package %{_rel} post-uninstall script upgrade phase"
fi

%files
%defattr(-,root,root)
%{FIXSBINdir}/fixerls.sh
%{FIXSBINdir}/fixerls

%changelog
* Wed Jun 05 2013 - thanh.nguyen (at) dektech.com.au
- Created

