## Line drawing with smoothing and gamma correction

This simple console application allows you to dither P5 PNM images

**Arguments format: binary_execurion_file <input_file_name> <output_file_name> \<gradient> \<dithering_type> \<bit_rate> \<gamma>**
>**Note**: All arguments are reqired

| Argument | Format | Description |
|---|---|---|
|**<input_file_name>**|*Path ending with .npm file*|Name of the input file|
|**<output_file_name>**|*Path ending with .npm file*|Name of the outnput file|
|**\<gradient>**|*1 or 0*|If set 1, the picture will be replaced with horizontal gradient from 0 to 255|
|**\<dithering_type>**|*Positive real number*||
|**\<bit_rate>**|*Number between 1 and 8*|New bit count per pixel|
|**\<gamma>**|*Positive real number*|gamma value, 0 equals sRGB|
