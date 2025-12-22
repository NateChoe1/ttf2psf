## ttf2psf - a simple psf generator

This just converts files to psf format with freetype, nothing else.

## Quickstart

    ttf2psf -g -c [charset file] -e [equivalence file] input.ttf output.psfu.gz

> [!important]
> When the psf/psfu file is being built, it will give a bunch of warnings, they mean nothing but you also cant get rid of them
>
> No matter the amount of warnings you get the output file you have defined will be created either way, the warnings just tell you that the output file doesnt have 100% of the charset but only 95%, which i would hope is plenty - otherwise i guess you can try solving all the warnings

## Usage

### Mandatory

    ttf2psf -c [char set] -e [equivalence file] [input.ttf] [output.psfu]

    -c     : Specify a character set from one of the following
            (see /usr/share/ttf2psf/charsets or ./data/charsets)

    -e     : Specify an equivalence file from one of the following
            (see /usr/share/ttf2psf/equivalence or ./data/equivalence)

### Optional

    -1     : Output a psf1 file
    -2     : Output a psf2 file (default)
    -g     : Output a gzip compressed file
    -h     : Show this help menu
    -w, -r : Sets character width and row count respectively (default: 8x16)
             increase -w to increase the size, increase -r to adjust the spacing


## Licensing/inspiration

This specific utility is released under the GPL-3.0 or later license, but there
are several other projects with other licenses that this one wouldn't have been
possible without.

The charset and equivalence files are taken directly from Anton Zinoviev's
bdf2psf utility, which is released under the GPL-2.0. These specific files are
in the public domain, but I feel like I should include this message.

To write this program I've had to consult Alexey Gladkov's kbd utilities quite a
bit (GPLv2 or later). I haven't copied any code from them, but I feel like they
deserve some recognition.
