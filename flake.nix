{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    pre-commit-hooks = {
      url = "github:cachix/git-hooks.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = {self, ...} @ inputs:
    inputs.flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import inputs.nixpkgs {
        inherit system;
        config.allowUnfree = true;
      };

      nativeBuildInputs = with pkgs; [
        gcc
        cmake
        ninja
      ];

      buildInputs = with pkgs; [
        cglm
        cmocka
        glslang
        vulkan-extension-layer
        vulkan-headers
        vulkan-loader
        vulkan-validation-layers
        glfw
      ];

      VK_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
    in {
      checks = {
        pre-commit-check = inputs.pre-commit-hooks.lib.${system}.run {
          excludes = ["flake.lock"];
          src = ./.;
          hooks = {
            alejandra.enable = true;
            clang-format = {
              enable = true;
              types_or = ["c" "c++"];
            };
            cmake-format.enable = true;
          };
        };
      };

      packages.default = pkgs.stdenv.mkDerivation {
        name = "space-engine";
        inherit nativeBuildInputs buildInputs VK_LAYER_PATH;
        src = ./.;
      };

      devShells.default = pkgs.mkShell {
        inherit nativeBuildInputs VK_LAYER_PATH;
        inherit (self.checks.${system}.pre-commit-check) shellHook;
        buildInputs = buildInputs ++ self.checks.${system}.pre-commit-check.enabledPackages;
        packages = with pkgs; [
          clang-tools
          gdb
          glslls
          ikos
          nil
          neocmakelsp
          nodejs
          kdePackages.kcachegrind
          cudaPackages.nsight_systems
          vulkan-tools
          vscode-langservers-extracted
          yaml-language-server
        ];
      };

      formatter = pkgs.alejandra;
    });
}
