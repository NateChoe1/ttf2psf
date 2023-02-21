# ttf2psf - a psf generator that actually works

Linux console fonts are set using the setfont utility, which only accepts fonts
in PC Screen Font (psf) format. The standard way of generating these files, and
the one that Debian uses internally, is to get an OpenType (otf) font file and
use the bdf2psf utility. This utility does not always work, so I wrote this.
