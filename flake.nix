{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = {nixpkgs, ...}: let
    system = "x86_64-linux";
    pkgs = import nixpkgs {inherit system;};
  in {
    devShells.${system}.default = pkgs.mkShell {
      nativeBuildInputs = with pkgs; [
        meson
        ninja
        pkg-config
        cmake
        scdoc
      ];
      buildInputs = with pkgs; [
        wayland-protocols
        wayland-scanner
        wayland
        cairo
        libxkbcommon
        cjson
        imagemagick.dev
      ];
      packages = with pkgs; [
        llvmPackages.clang-tools
        grim
      ];
    };
  };
}
