#!/bin/bash

set -ex

cd `dirname $0`

PHP_BASE=$1
PHP=$PHP_BASE/bin/php

# Newest version that still supports PHP 7.2
# But throws lots of deprecation warnings.
#PHPUNIT=phpunit-8.5.4.phar

# Oldest major version that supports PHP 7.2
PHPUNIT=phpunit-6.5.9.phar

if [ -z "$PHP_BASE"] ; then
  echo "Usage: test.sh <php-base>"
  exit 1
fi

if [ ! -x $PHP ]; then
  echo Could not find executable file: $PHP
fi

# Download phpunit.
[ -f $PHPUNIT ] || wget https://phar.phpunit.de/$PHPUNIT

./generate_protos.sh
(cd .. && ./make-preload.py)

# Compile c extension
pushd  ../ext/google/protobuf2
$PHP_BASE/bin/phpize
./configure --with-php-config=$PHP_BASE/bin/php-config CFLAGS="-O0 -g -DPBPHP_ENABLE_ASSERTS"
make clean && make -j$(nproc)
popd

#export ZEND_DONT_UNLOAD_MODULES=1
#export USE_ZEND_ALLOC=0

CMD="$PHP -dextension=../ext/google/protobuf2/modules/protobuf.so $PHPUNIT --bootstrap autoload.php test2"
$CMD
#gdb -ex=r --args $CMD
#rr record $CMD
#valgrind --leak-check=full --track-origins=yes $CMD
#$PHP -d protobuf.keep_descriptor_pool_after_request=1 -dextension=../ext/google/protobuf/modules/protobuf.so $PHPUNIT --bootstrap autoload.php .

# # Make sure to run the memory test in debug mode.
# php -dextension=../ext/google/protobuf/modules/protobuf.so memory_leak_test.php

#export ZEND_DONT_UNLOAD_MODULES=1
#export USE_ZEND_ALLOC=0
#valgrind --leak-check=yes $PHP -dextension=../ext/google/protobuf/modules/protobuf.so memory_leak_test.php
#valgrind --leak-check=yes $PHP -d protobuf.keep_descriptor_pool_after_request=1 -dextension=../ext/google/protobuf/modules/protobuf.so memory_leak_test.php
#
#exit 0

# TODO(teboring): Only for debug (phpunit has memory leak which blocks this beging used by
# regresssion test.)

# for t in "${tests[@]}"
# do
#   echo "****************************"
#   echo "* $t (memory leak)"
#   echo "****************************"
#   valgrind --leak-check=yes php -dextension=../ext/google/protobuf/modules/protobuf.so `which phpunit` --bootstrap autoload.php $t
#   echo ""
# done
