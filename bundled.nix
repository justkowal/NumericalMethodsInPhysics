{
  description = "Numerical Methods in Physics";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        inherit system;
      };

      pythonEnv = pkgs.python312.withPackages (ps:
        with ps; [
          numpy
          scipy
          matplotlib
        ]);
    in {
      devShells.default = pkgs.mkShell {
        name = "nmp-lab-shell";

        buildInputs = [
          pkgs.gcc
          pkgs.gnumake
          pkgs.cmake
          pkgs.pkg-config
          pkgs.openblas
          pkgs.fftw
          pkgs.nushell
          pythonEnv
          pkgs.zip
          pkgs.unzip
        ];

        NIX_CFLAGS_COMPILE = "-I${pkgs.openblas.dev}/include -I${pkgs.openblas.dev}/include/openblas -I${pkgs.fftw.dev}/include";
        NIX_LDFLAGS = "-L${pkgs.glibc}/lib -L${pkgs.openblas}/lib -L${pkgs.fftw}/lib";
        LIBRARY_PATH = "${pkgs.glibc}/lib:${pkgs.openblas}/lib:${pkgs.fftw}/lib";
        CMAKE_LIBRARY_PATH = "${pkgs.glibc}/lib:${pkgs.openblas}/lib:${pkgs.fftw}/lib";
        CMAKE_INCLUDE_PATH = "${pkgs.openblas.dev}/include:${pkgs.openblas.dev}/include/openblas:${pkgs.fftw.dev}/include";

        shellHook = ''
          unset LD_LIBRARY_PATH
          unset NIX_CFLAGS_LINK
          if [ -z "$IN_NUSHELL" ] && [ -z "$DIRENV_IN_ENVRC" ] && [[ "$-" == *i* ]]; then
            export IN_NUSHELL=1
            exec ${pkgs.nushell}/bin/nu
          fi
        '';
      };
    });
}
