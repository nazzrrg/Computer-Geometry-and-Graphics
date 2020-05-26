## Line drawing with smoothing and gamma correction

This simple console application allows you to dither P5 PNM images

**Arguments format: binary_execurion_file <input_file_name> <output_file_name> \<gradient> \<dithering_type> \<bit_rate> \<gamma>**
>**Note**: All arguments are reqired

| Argument | Format | Description |
|---|---|---|
|**<input_file_name>**|*Path ending with .pnm file*|Name of the input file|
|**<output_file_name>**|*Path ending with .pnm file*|Name of the outnput file|
|**\<gradient>**|*1 or 0*|If set 1, the picture will be replaced with horizontal gradient from 0 to 255|
|**\<dithering_type>**|*Positive real number*|0 - No Dithering(Thresholding)<br>1 - Ordered 8x8<br>2 - Random<br>3 - Floyd-Steinberg<br>4 - Jarvis, Judice, Ninke<br>5 - Sierra-3<br>6 - Atkinson<br>7 - Halftone orthogonal 4x4|
|**\<bit_rate>**|*Number between 1 and 8*|New bit count per pixel|
|**\<gamma>**|*Positive real number*|Gamma value, 0 equals sRGB|
