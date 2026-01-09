{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    self.submodules = true;
  };

  outputs = {nixpkgs, ...}: let
    system = "x86_64-linux";
    pkgs = import nixpkgs {inherit system;};
  in {
    packages.${system}.default = pkgs.callPackage ({
      stdenv,
      pkg-config,
      meson,
      ninja,
      cmake,
      wayland-protocols,
      wayland-scanner,
      wayland,
      cairo,
      libxkbcommon,
      cjson,
      imagemagick,
      scdoc,
      makeWrapper,
      lib,
      grim,
    }:
      stdenv.mkDerivation {
        name = "mscreenshot";
        src = ./.;

        depsBuildBuild = [pkg-config];
        strictDeps = true;

        nativeBuildInputs = [
          meson
          ninja
          pkg-config
          cmake
          scdoc
          makeWrapper
        ];

        buildInputs = [
          wayland-protocols
          wayland-scanner
          wayland
          cairo
          libxkbcommon
          cjson
          imagemagick.dev
        ];

        # postInstall = ''
        #   wrapProgram $out/bin/mscreenshot \
        #   --suffix PATH : ${lib.makeBinPath [grim]} \
        #   --set MAGICK_HOME ${imagemagick} \
        #   --prefix LD_LIBRARY_PATH : ${imagemagick}/lib
        # '';
      }) {};

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
