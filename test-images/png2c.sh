#!/bin/bash

echo "Starting conversion, this may take a few minutes."
convert "$IMG.png" -crop 1x1 +repage -depth 8 -format "%[hex:s]\n" info: > "$IMG.c"
