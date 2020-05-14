
## Simplest .NPM image editor

This simple console application allows you to rotate, mirror and invert .npm images. 

**Arguments format: binary_execurion_file <input_file_name> <output_file_name> \<action>**
>**Note**: All arguments are reqired, only P5 and P6 grayscale and color images are supported

| Argument | Format | Description |
|---|---|---|
|**<input_file_name>**|*Path ending with .pnm file*|Name of the input file|
|**<output_file_name>**|*Path ending with .pnm file*|Name of the outnput file|
|**\<action>**|*Number between 0 and 5*|0 - Inversion<br>1 - Horizontal mirroring<br>2 - Vertical mirroring<br>3 - 90° rotation clockwise<br>4 - 90° rotation counterclockwise|
