Name:          myservices
Version:       0.0.1
Release:       1%{?dist}
Packager:      Mikko Koppanen <mkoppanen@php.net>
Summary:       MyServices service locator
License:       BSD License
Group:         System/Applications
URL:           http://github.com/mkoppanen/myservices
Source:        %{name}-%{version}.tar.gz
Prefix:        %{_prefix}
Buildroot:     %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: zookeeper-c, zookeeper-c-devel
Requires:      zookeeper-c

%description
MyServices locator for dynamic service discovery

%prep
%setup -q -n %{name}-%{version}

%build
# Clean the buildroot so that it does not contain any stuff from previous builds
[ "%{buildroot}" != "/" ] && %{__rm} -rf %{buildroot}

if [ -f autogen.sh ]; then
    echo "Building from non-dist package"
    ./autogen.sh
fi

# Configure
%{configure} --prefix=/usr --sysconfdir=/etc

# Install to build root
%{makeinstall}

%{__mkdir} -p %{buildroot}/etc/init.d
%{__cp} %{_builddir}/%{name}-%{version}/scripts/myservices.init %{buildroot}/etc/init.d/myservices

%clean
[ "%{buildroot}" != "/" ] && %{__rm} -rf %{buildroot}

%pre
# Add the "myservices" user
/usr/sbin/useradd -c "MyServices" -u 321 \
-s /sbin/nologin -r -d %{_localstatedir}/run myservices 2> /dev/null || :

%post
chkconfig zookeeper on

%files
%{_bindir}/myservices
%{_sysconfdir}/myservices/myservices.ini
%attr(755, root, root) %{_sysconfdir}/init.d/myservices

%changelog
* Thu Jun 24 2010 Mikko Koppanen <mkoppanen@php.net>
 - Initial spec file
