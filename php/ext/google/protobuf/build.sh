#!/bin/bash

set -e

PHP_DIR=$1

echo Building extension with PHP in $PHP_DIR

git clean -q -d -X -f
$PHP_DIR/bin/phpize
./configure --with-php-config=$PHP_DIR/bin/php-config
make -j$(nproc --all) V=0
