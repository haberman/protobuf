#!/usr/bin/env python3

import os
import sys

os.chdir(os.path.dirname(sys.argv[0]) + "/src")

cmd = "grep -r -l 'Generated by the protocol buffer' * | grep -v Internal"
filenames = os.popen(cmd).read().splitlines()
filenames.append("Google/Protobuf/DescriptorPool.php")
output = "ext/google/protobuf2/bundled_php.c"

def to_class_name(filename):
  # Google/Protobuf/BoolValue.php -> Google\\Protobuf\\BoolValue
  assert filename.endswith(".php")
  name = filename[:-4]
  return name.replace("/", "\\\\")

def to_c_symbol_name(filename):
  # Google/Protobuf/BoolValue.php -> Google__Protobuf__BoolValue
  assert filename.endswith(".php")
  name = filename[:-4]
  return name.replace("/", "__")

with open("../" + output, "w") as f:
  f.write('#include "bundled_php.h"\n')
  f.write('#include "stdlib.h"\n')

  for filename in filenames:
    print("Reading %s..." % filename)
    contents = open(filename, "rb").read()[5:]
    f.write('static const char %s[] = {' % to_c_symbol_name(filename))
    for i in range(0, len(contents)):
      if i % 10 == 0:
        f.write('\n')
      f.write('  0x%02x,' % contents[i])
    f.write('0};\n')

  f.write('static pbphp_bundled php[] = {\n')
  for filename in filenames:
    values = (to_class_name(filename), to_c_symbol_name(filename))
    f.write('  {"%s", %s},\n' % values)

  f.write('  {NULL, NULL}\n')
  f.write('};\n')
  f.write('pbphp_bundled *bundled_files = &php[0];\n')

print("Wrote %s" % output)
