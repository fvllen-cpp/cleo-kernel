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
        default = pkgs.mkShell {
          buildInputs = with pkgs; [
            git
            python3
            clang-tools
            qemu
          ];
        };
      });

      shellHook = ''
        echo "Cleo Kernel ^(owo)^"
      '';
    }
  );
}
