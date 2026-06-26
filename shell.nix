let
  nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/tarball/nixos-26.05";
  pkgs = import nixpkgs { config = {}; overlays = []; };
in

pkgs.mkShell {
  packages = with pkgs; [
    doxygen
    clang
    clang-tools
    cmake
    pkg-config
  ];
  inputsFrom = [ pkgs.stdenv.cc ];
}
