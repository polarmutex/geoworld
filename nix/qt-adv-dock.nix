{
  # lib,
  stdenv,
  cmake,
  qt6,
  libsForQt5,
  wrapQtAppsHook,
  fetchFromGitHub,
  git,
}:
stdenv.mkDerivation rec {
  pname = "qt-advanced-docking";
  version = "4.3.1";

  src = fetchFromGitHub {
    owner = "githubuser0xFFFF";
    repo = "Qt-Advanced-Docking-System";
    rev = "4.3.1";
    sha256 = "sha256-5wOmhjV/RoKvd018YC4J8EFCCkq+3B6AXAsPtW+RZHU=";
  };

  cmakeFlags = [
    "-DADS_VERSION=4.3.1"
    "-DBUILD_EXAMPLES=true"
  ];

  buildType = "cmake";
  propagatedBuildInputs = [
    qt6.qtbase
    qt6.qtquick3d
  ];
  nativeBuildInputs = [
    cmake
    git
    wrapQtAppsHook
  ];
}
