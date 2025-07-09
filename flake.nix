{
  description = "GeoWorld - Geospatial Data Visualization Platform";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    systems.url = "github:nix-systems/default";
  };

  outputs = inputs @ {
    flake-parts,
    systems,
    ...
  }:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = import systems;

      perSystem = {
        config,
        self',
        inputs',
        pkgs,
        system,
        ...
      }: let
        # Create qt-advanced-docking-system package since it's not in nixpkgs
        qt-advanced-docking-system = pkgs.stdenv.mkDerivation rec {
          pname = "qt-advanced-docking-system";
          version = "4.3.1";
          
          src = pkgs.fetchFromGitHub {
            owner = "githubuser0xFFFF";
            repo = "Qt-Advanced-Docking-System";
            rev = "${version}";
            sha256 = "sha256-5wOmhjV/RoKvd018YC4J8EFCCkq+3B6AXAsPtW+RZHU=";
          };
          
          nativeBuildInputs = with pkgs; [
            cmake
            qt6.wrapQtAppsHook
            pkg-config
          ];
          
          buildInputs = with pkgs; [
            qt6.qtbase
            qt6.qtdeclarative
          ];
          
          cmakeFlags = [
            "-DCMAKE_BUILD_TYPE=Release"
            "-DBUILD_EXAMPLES=OFF"
            "-DBUILD_TESTS=OFF"
            "-DADS_VERSION=${version}"
            "-DBUILD_STATIC=OFF"
          ];
          
          # Patch the CMakeLists.txt to fix version issue
          postPatch = ''
            substituteInPlace CMakeLists.txt \
              --replace 'set(ADS_VERSION ''${ADS_VERSION_MAJOR}.''${ADS_VERSION_MINOR}.''${ADS_VERSION_PATCH})' \
                        'set(ADS_VERSION ${version})'
          '';
          
          # Create a pkg-config file manually if not created
          postInstall = ''
            mkdir -p $out/lib/pkgconfig
            cat > $out/lib/pkgconfig/qtadvanceddocking.pc << EOF
            prefix=$out
            exec_prefix=$out
            libdir=$out/lib
            includedir=$out/include
            
            Name: Qt Advanced Docking System
            Description: Advanced Docking System for Qt applications
            Version: ${version}
            Libs: -L$out/lib -lqtadvanceddocking
            Cflags: -I$out/include -I$out/include/ads
            EOF
          '';
        };
      in {
        # Development shell
        devShells.default = pkgs.mkShell {
          name = "geoworld-dev";

          buildInputs = with pkgs; [
            # Build tools
            cmake
            ninja
            pkg-config

            # Qt Advanced Docking System
            qt-advanced-docking-system

            # Qt6 development
            qt6.qtbase
            qt6.qtdeclarative
            qt6.qtlocation
            qt6.qtpositioning
            qt6.qtwebengine
            qt6.qtquick3d

            # C++ compiler and tools
            gcc
            clang
            clang-tools

            # Additional development tools
            gdb
            valgrind
            cppcheck

            # Documentation tools
            doxygen
            graphviz

            # Version control
            git

            # System libraries
            xorg.libX11
            xorg.libXext
            xorg.libXrender
            xorg.libXrandr
            xorg.libXi
            xorg.libXcursor
            xorg.libXinerama
            xorg.libXxf86vm
            libGL
            libGLU

            # Audio/Video libraries (for Qt multimedia)
            alsa-lib
            pulseaudio

            # Networking
            openssl

            # Image libraries
            libpng
            libjpeg

            # Font libraries
            fontconfig
            freetype
          ];

          shellHook = ''
            echo "🌍 GeoWorld Development Environment"
            echo "=================================="
            echo "Qt version: $(qmake -query QT_VERSION)"
            echo "CMake version: $(cmake --version | head -n1)"
            echo "Qt Advanced Docking System available"
            echo ""
            echo "Available commands:"
            echo "  configure    - Configure CMake build"
            echo "  build        - Build the project"
            echo "  run          - Run the application"
            echo "  clean        - Clean build artifacts"
            echo "  test         - Run tests"
            echo ""

            # Set up Qt environment
            export QT_QPA_PLATFORM_PLUGIN_PATH="${pkgs.qt6.qtbase}/lib/qt-6/plugins/platforms"
            export QT_PLUGIN_PATH="${pkgs.qt6.qtbase}/lib/qt-6/plugins"
            export QML_IMPORT_PATH="${pkgs.qt6.qtdeclarative}/lib/qt-6/qml"

            # Set up build environment
            export CMAKE_PREFIX_PATH="${pkgs.qt6.qtbase}:${pkgs.qt6.qtlocation}:${pkgs.qt6.qtpositioning}:${qt-advanced-docking-system}"
            export PKG_CONFIG_PATH="${pkgs.qt6.qtbase}/lib/pkgconfig:${pkgs.qt6.qtlocation}/lib/pkgconfig"

            # Helper functions

            configure() {
              echo "Configuring CMake build..."
              cmake -S . -B build \
                -DCMAKE_BUILD_TYPE=Release \
                -DBUILD_PLUGINS=ON \
                -DCMAKE_PREFIX_PATH="${pkgs.qt6.qtbase};${pkgs.qt6.qtlocation};${pkgs.qt6.qtpositioning};${qt-advanced-docking-system}"
            }

            build() {
              echo "Building GeoWorld..."
              cmake --build build --parallel $NIX_BUILD_CORES
            }

            run() {
              echo "Running GeoWorld..."
              ./build/geoworld
            }

            clean() {
              echo "Cleaning build artifacts..."
              rm -rf build/
            }

            test() {
              echo "Running tests..."
              cd build && ctest
            }
          '';
        };

        # Package definition
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "geoworld";
          version = "1.0.0";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            pkg-config
            qt6.wrapQtAppsHook
          ];

          buildInputs = with pkgs; [
            qt6.qtbase
            qt6.qtdeclarative
            qt6.qtlocation
            qt6.qtpositioning
            qt6.qtwebengine
            qt6.qtquick3d
            qt-advanced-docking-system

            # System libraries
            xorg.libX11
            xorg.libXext
            xorg.libXrender
            xorg.libXrandr
            xorg.libXi
            xorg.libXcursor
            xorg.libXinerama
            libGL
            libGLU

            # Additional libraries
            openssl
            alsa-lib
            pulseaudio
            libpng
            libjpeg
            fontconfig
            freetype
          ];

          configurePhase = ''
            runHook preConfigure

            # Configure CMake
            cmake -S . -B build \
              -DCMAKE_BUILD_TYPE=Release \
              -DCMAKE_INSTALL_PREFIX=$out \
              -DBUILD_PLUGINS=ON \
              -DCMAKE_PREFIX_PATH="${pkgs.qt6.qtbase};${pkgs.qt6.qtlocation};${pkgs.qt6.qtpositioning};${qt-advanced-docking-system}"

            runHook postConfigure
          '';

          buildPhase = ''
            runHook preBuild
            cmake --build build --parallel $NIX_BUILD_CORES
            runHook postBuild
          '';

          installPhase = ''
            runHook preInstall
            cmake --install build
            runHook postInstall
          '';

          meta = with pkgs.lib; {
            description = "Geospatial Data Visualization Platform";
            homepage = "https://github.com/user/geoworld";
            license = licenses.mit;
            platforms = platforms.linux;
            maintainers = [];
          };
        };

        # Application for easy running
        apps.default = {
          type = "app";
          program = "${self'.packages.default}/bin/geoworld";
        };
      };
    };
}
