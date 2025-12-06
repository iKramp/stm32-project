{
  description = "STM32 bare-metal development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          name = "stm32-dev";

          buildInputs = with pkgs; [
            gcc-arm-embedded

            openocd
            stlink

            gdb

            gnumake
            cmake
            ffmpeg
          ];

          shellHook = ''
            exec zsh -c 'nvim'
          '';
        };
      });
}

