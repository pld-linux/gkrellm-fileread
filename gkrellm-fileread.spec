Summary:	file to krell plugin for gkrellm
Summary(pl):	Plugin dla gkrellm'a do odczytu danych z pliku
Summary(pt_BR):	Plugin gkrellm para monitoração de valores em arquivos
Name:		gkrellm-fileread
Version:	0.10.0
Release:	1
License:	GPL
Group:		X11/Applications
Source0:	http://www.yty.net/h/gkrellm/fileread.c
Patch0:		%{name}-ucgk.patch
Patch1:		%{name}-small_fix.patch
BuildRequires:	gkrellm-devel
BuildRequires:	gtk+-devel
BuildRequires:	imlib-devel
Requires:	gkrellm >= 1.0.2
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%define		_prefix		/usr/X11R6
%define		_mandir		%{_prefix}/man

%description
A GKrellM plugin that reads a '<label> <value>' pair from a file and
displays the value in a krell. It can be used to monitor any arbitrary
values put in a file by external processes.

%description -l pl
Plugin dla GKrellM'a, który czyta pary '<etykieta> <warto¶æ>' z pliku
oraz wy¶witla je w "krellu". Mo¿e byæ u¿yty do monitorowania
jakichkolwiek warto¶ci z zewnêtrznych procesów.

%description -l pt_BR
Um plugin GKrellM que lê um par '<rótulo> <valor>' de um arquivo e o
mostra na tela. Pode ser usado para monitorar um valor qualquer
colocado em um arquivo por outros processos.

%prep -q
%setup -q -T -c -n %{name}
cp %{_sourcedir}/fileread.c .
%patch0
%patch1

%build
%{__cc} %{rpmcflags} -Wall `gtk-config --cflags` `imlib-config --cflags-gdk` -shared -o fileread.so fileread.c `gtk-config --libs` `imlib-config --libs-gdk`

%install
rm -rf $RPM_BUILD_ROOT
rm -rf %{buildroot}
install -D fileread.so %{buildroot}%{_libdir}/gkrellm/plugins/fileread.so

%clean
rm -rf %{buildroot}

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_libdir}/gkrellm/plugins/fileread.so
