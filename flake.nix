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
      }: {
        # Development shell
        devShells.default = pkgs.mkShell {
          name = "geoworld-dev";

          buildInputs = with pkgs; [
            # Build tools
            cmake
            ninja
            pkg-config

            # Conan package manager
            conan

            # Qt6 development
            qt6.full
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
            echo "Conan version: $(conan --version)"
            echo ""
            echo "Available commands:"
            echo "  setup-conan  - Install Conan dependencies"
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
            export CMAKE_PREFIX_PATH="${pkgs.qt6.qtbase}:${pkgs.qt6.qtlocation}:${pkgs.qt6.qtpositioning}"
            export PKG_CONFIG_PATH="${pkgs.qt6.qtbase}/lib/pkgconfig:${pkgs.qt6.qtlocation}/lib/pkgconfig"

            # Helper functions
            setup-conan() {
              echo "Setting up Conan dependencies..."
              conan profile detect --force
              conan install . --build=missing
            }

            configure() {
              echo "Configuring CMake build..."
              cmake --preset=default
            }

            build() {
              echo "Building GeoWorld..."
              cmake --build --preset=default
            }

            run() {
              echo "Running GeoWorld..."
              ./build/default/geoworld
            }

            clean() {
              echo "Cleaning build artifacts..."
              rm -rf build/
              rm -rf ~/.conan2/
            }

            test() {
              echo "Running tests..."
              ctest --preset=default
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
            conan
          ];

          buildInputs = with pkgs; [
            qt6.qtbase
            qt6.qtdeclarative
            qt6.qtlocation
            qt6.qtpositioning
            qt6.qtwebengine
            qt6.qtquick3d

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

            # Setup Conan
            export CONAN_HOME=$TMPDIR/conan
            conan profile detect --force
            conan install . --build=missing --output-folder=build

            # Configure CMake
            cmake -S . -B build \
              -DCMAKE_BUILD_TYPE=Release \
              -DCMAKE_INSTALL_PREFIX=$out \
              -DCMAKE_PREFIX_PATH="${pkgs.qt6.qtbase};${pkgs.qt6.qtlocation};${pkgs.qt6.qtpositioning}" \
              -DCMAKE_TOOLCHAIN_FILE=build/generators/conan_toolchain.cmake

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
