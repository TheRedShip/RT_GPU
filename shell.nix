{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {
    nativeBuildInputs = with pkgs; [ libGL xorg.libX11 libGLU glfw ffmpeg xorg.xvfb];
	shellHook=''
	alias xdum="Xvfb :99 -screen 0 1920x1080x24 +extension GLX +render -noreset & export DISPLAY=:99";
	alias xdumstop='pkill Xvfb'
	make -j;
	'';
}

