{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = import nixpkgs {
            inherit system;
          };
          nativeBuildInputs = with pkgs; [
            gdb
            libclang
            gcc
            gnumake
            valgrind
            heaptrack
          ];
          buildInputs = with pkgs; [
            readline
          ];
        in
        with pkgs;
        {
          devShells.default = mkShell {
            LD_LIBRARY_PATH = lib.makeLibraryPath buildInputs;
            inherit buildInputs nativeBuildInputs;
          };
        }
      );
}
