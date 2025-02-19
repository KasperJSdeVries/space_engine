{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    nixpkgs,
    flake-utils,
    ...
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        inherit system;
        config.allowUnfree = true;
      };

      nativeBuildInputs = with pkgs; [
        gcc
        ninja
      ];

      buildInputs = with pkgs; [
        shaderc
        vulkan-extension-layer
        vulkan-loader
        vulkan-validation-layers
        xorg.libX11
      ];

      VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";

    in {
      devShells.default = pkgs.mkShell {
        inherit nativeBuildInputs buildInputs VK_LAYER_PATH;
        packages = with pkgs; [
          clang-tools
          gdb
          glslls
          ikos
          nil
          kcachegrind
          renderdoc
          vulkan-tools
          vscode-langservers-extracted
        ];
      };

      formatter = pkgs.alejandra;
    });
}
