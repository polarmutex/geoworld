{
  description = "A Nix-flake-based C/C++ development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs = inputs @ {
    self,
    flake-parts,
    ...
  }:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = [
        "aarch64-linux"
        "aarch64-darwin"
        "x86_64-darwin"
        "x86_64-linux"
      ];
      perSystem = {
        _config,
        pkgs,
        system,
        inputs',
        ...
      }: let
      in {
        _module.args.pkgs = import inputs.nixpkgs {
          inherit system;
          overlays = [
            (_final: _prev: {
              # neovim-nightly = inputs.neovim.packages.${final.system}.neovim;
            })
          ];
        };

        packages = {
          qt6-adv-docking-system = pkgs.qt6Packages.callPackage ./nix/qt-adv-dock.nix {};
        };

        devShells = {
          default = pkgs.mkShell {
            name = "geoworld-shell";
            packages = with pkgs; [
              clang-tools
              cmake
              codespell
              conan
              cppcheck
              cpplint
              # doxygen
              # gtest
              # lcov
              # vcpkg
              # vcpkg-tool
              pkgs.cmake
              pkgs.gnumake
              pkgs.qt6.full
              self.packages.${system}.qt6-adv-docking-system
            ];
          };
        };
      };
    };
}
#     packages = forEachSupportedSystem ({pkgs}: {
#       default = pkgs.qt6Packages.callPackage ./nix/qt-adv-dock.nix {};
#     });
#     devShells = forEachSupportedSystem ({pkgs}: {
#       default =
#         pkgs.mkShell.override
#         {
#           # Override stdenv in order to change compiler:
#           # stdenv = pkgs.clangStdenv;
#         }
#         {
#           packages = with pkgs;
#             [
#               clang-tools
#               cmake
#               codespell
#               conan
#               cppcheck
#               # doxygen
#               # gtest
#               # lcov
#               # vcpkg
#               # vcpkg-tool
#               pkgs.cmake
#               pkgs.gnumake
#               pkgs.qt6.full
#               self.packages.default
#
#               # pkg-config
#               # libGL
#               # xkeyboard_config
#               # xorg.libX11
#               # xorg.libfontenc
#               # xorg.libICE
#               # xorg.libSM
#               # xorg.libXau
#               # xorg.libXaw
#               # xorg.libXcomposite
#               # xorg.libXcursor
#               # xorg.libXdamage
#               # xorg.libXdmcp
#               # xorg.libXext
#               # xorg.libXi
#               # xorg.libXinerama
#               # xorg.libxkbfile
#               # xorg.libXpm
#               # xorg.libXrandr
#               # xorg.libXres
#               # xorg.libXScrnSaver
#               # xorg.libXtst
#               # xorg.libXv
#               # xorg.libXxf86vm
#               # xorg.libxcb
#               # xorg.xcbutil
#               # xorg.xcbutilimage
#               # xorg.xcbutilkeysyms
#               # xorg.xcbutilrenderutil
#               # xorg.xcbutilwm
#               # libxkbcommon
#               # xcbutilxrm
#               # xcb-util-cursor
#               # libossp_uuid
#             ]
#             ++ (
#               if system == "aarch64-darwin"
#               then []
#               else [gdb]
#             );
#         };
#     });
#   };
# }

