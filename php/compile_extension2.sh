#!/bin/bash

set -ex

cd `dirname $0`
PHP_BASE=$1

if [ "$2" = "--debug" ]; then
  CFLAGS="-O0 -g -DPBPHP_ENABLE_ASSERTS"
fi

./make-preload.py
pushd  ext/google/protobuf2
$PHP_BASE/bin/phpize
./configure --with-php-config=$PHP_BASE/bin/php-config CFLAGS="$CFLAGS"
make clean && make -j$(nproc)
popd
