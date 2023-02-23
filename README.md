## ttf2psf - a simple psf generator

This just converts files to psf format with freetype, nothing else.

## Usage

    ttf2psf -g -c [charset file] -e [equivalence file] input.ttf output.psfu.gz

## Licensing/inspiration

This specific utility is released under the GPL-3.0 or later license, but there
are several other that this one wouldn't have been possible without.

The charset and equivalence files are taken directly from Anton Zinoviev's
bdf2psf utility, which is released under the GPL-2.0. These specific files are
in the public domain, but I feel like I should include this message.

To write this program I've had to consult Alexey Gladkov's kbd utilities quite a
bit. I haven't copied any code from them, but I feel like they deserve some
recognition.
