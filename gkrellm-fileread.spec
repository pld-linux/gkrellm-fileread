Summary:	file to krell plugin for gkrellm
Summary(pl):	Plugin dla gkrellma do odczytu danych z pliku
Summary(pt_BR):	Plugin gkrellm para monitora��o de valores em arquivos
Name:		gkrellm-fileread
Version:	0.10.0
Release:	3
License:	GPL
Group:		X11/Applications
Source0:	http://www.yty.net/h/gkrellm/fileread.c
# Source0-md5:	c506c3745eba966e58560bbe32afe1df
Patch0:		%{name}-ucgk.patch
Patch1:		%{name}-small_fix.patch
BuildRequires:	gkrellm-devel
BuildRequires:	gtk+-devel
BuildRequires:	imlib-devel
Requires:	gkrellm >= 1.0.2
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)


%description
A GKrellM plugin that reads a '<label> <value>' pair from a file and
displays the value in a krell. It can be used to monitor any arbitrary
values put in a file by external processes.

%description -l pl
Plugin dla GKrellMa, kt�ry czyta pary '<etykieta> <warto��>' z pliku
oraz wy�wietla je w "krellu". Mo�e by� u�yty do monitorowania
jakichkolwiek warto�ci z zewn�trznych proces�w.

%description -l pt_BR
Um plugin GKrellM que l� um par '<r�tulo> <valor>' de um arquivo e o
mostra na tela. Pode ser usado para monitorar um valor qualquer
colocado em um arquivo por outros processos.

%prep -q
%setup -q -T -c -n %{name}
cp -f %{SOURCE0} .
%patch0
%patch1

%build
%{__cc} %{rpmcflags} -Wall \
	`gtk-config --cflags` \
	`imlib-config --cflags-gdk` \
	-shared -o fileread.so fileread.c \
	`gtk-config --libs` \
	`imlib-config --libs-gdk`

%install
rm -rf $RPM_BUILD_ROOT

install -D fileread.so %{buildroot}%{_libdir}/gkrellm/fileread.so

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%attr(755,root,root) %{_libdir}/gkrellm/fileread.so
