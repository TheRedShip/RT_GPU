{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {
    nativeBuildInputs = with pkgs; [ libGL xorg.libX11 libGLU glfw ffmpeg xorg.xvfb];

    __NV_PRIME_RENDER_OFFLOAD="1";
	__GLX_VENDOR_LIBRARY_NAME="nvidia";
	shellHook=''
	alias xdum="sudo Xorg :99 -noreset & export DISPLAY=:99";
	alias xdumstop='sudo pkill Xorg'
	make -j;
	'';
}

