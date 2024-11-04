#!/bin/bash

set -e

JANSSON_VERSION=2.13

wget https://github.com/akheron/jansson/releases/download/v${JANSSON_VERSION}/jansson-${JANSSON_VERSION}.tar.bz2
{
    bunzip2 -c jansson-${JANSSON_VERSION}.tar.bz2 | tar xf -
    rm jansson-${JANSSON_VERSION}.tar.bz2
    cd jansson-${JANSSON_VERSION}
    ./configure
    make
    make check
    make install
    cd ..
    rm -rf jansson-${JANSSON_VERSION}
} || {
    rm -rf jansson-${JANSSON_VERSION}
}

