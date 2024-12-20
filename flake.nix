{
  inputs.nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";

  outputs = { self, nixpkgs }:
   let pkgs = import nixpkgs { system = "x86_64-linux"; };
   in
   {
        devShells.x86_64-linux.default = pkgs.mkShell {
            buildInputs = [ pkgs.bashInteractive ];
            packages = with pkgs; [
                cmake
                gcc
                llvm
            ];
            env = {
                LC_ALL = "C.UTF-8";
            };
        };
    };
}