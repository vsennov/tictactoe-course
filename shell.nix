# this file is for nix package manager users
{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  NIX_LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [
    pkgs.zeromq pkgs.protobuf_21 pkgs.zlib pkgs.abseil-cpp
  ];
  # buildInputs is for dependencies you'd need "at run time",
  # were you to to use nix-build not nix-shell and build whatever you were working on
  buildInputs = [
    pkgs.cmake
    pkgs.protobuf_21
    pkgs.zeromq
    pkgs.cppzmq
    pkgs.zlib
    pkgs.abseil-cpp
  ];
  shellHook = ''
    export LD_LIBRARY_PATH=$NIX_LD_LIBRARY_PATH
    export PB_ADDITIONAL_LIBS="absl_log_internal_check_op;absl_log_internal_message"
  '';
}
