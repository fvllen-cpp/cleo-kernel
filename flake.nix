{
  description = "Cleo Kernel";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = {
    self,
    nixpkgs,
    ...
  }: (
    let
      supportedSystems = ["x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin"];

      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
    in {
      # Dev shell
      devShells = forAllSystems (system: let
        pkgs = import nixpkgs {inherit system;};
      in {
        default = (pkgs.mkShell.override {stdenv = pkgs.clangStdenv;}) {
          buildInputs = with pkgs; [
            git
            python3
            cmake
            clang-tools
            clang_22
            lld
            llvm
            qemu
          ];
        };
      });

      shellHook = ''
        echo "Cleo Kernel ᓚᘏᗢ"
      '';
    }
  );
}
