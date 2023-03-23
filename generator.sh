#!/bin/bash
set -e

# This script generates precalculated constants and embedded files used by PAX.
# The generated files are marked with a comment reading:
# // WARNING: This is a generated file, do not edit it!

# Embed constants
./precalc.py

# Embed icon files.
bpp=4
rm -f src/gui/icons/icons.c
echo "" > src/gui/icons/icons.h
echo "// WARNING: This is a generated file, do not edit it!" >> src/gui/icons/icons.h
echo "" >> src/gui/icons/icons.h
echo "#include <pax_gfx.h>" >> src/gui/icons/icons.h
echo "" >> src/gui/icons/icons.h
echo "#ifdef __cplusplus" >> src/gui/icons/icons.h
echo "extern \"C\" {" >> src/gui/icons/icons.h
echo "#endif" >> src/gui/icons/icons.h
echo "" >> src/gui/icons/icons.h
for name in file folder text data binary app; do
./png2c.py src/gui/icons/$name.png - pax_icon_$name $bpp 1>>src/gui/icons/icons.c
echo "void pax_icon_${name}_init(pax_buf_t *out);" >> src/gui/icons/icons.h
done
echo "" >> src/gui/icons/icons.h
echo "#ifdef __cplusplus" >> src/gui/icons/icons.h
echo "} // extern \"C\"" >> src/gui/icons/icons.h
echo "#endif" >> src/gui/icons/icons.h
