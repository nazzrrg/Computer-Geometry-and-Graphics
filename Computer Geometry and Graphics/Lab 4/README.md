## Line drawing with smoothing and gamma correction

This simple console application allows you to convert color spaces of PNM images and merge layers of color spaces.

**Arguments format: binary_execurion_file lab4.exe -f \<from_color_space> -t \<to_color_space> -i \<count> \<input_file_name> <br>-o \<count> \<output_file_name>
>**Note**: All arguments are reqired, order for -f, -t, -i, -o is optional.

| Argument | Format | Description |
|---|---|---|
|**-f \<from_color_space>**||Input color space|
|**-t \<to_color_space**||Output color space|
|**-i \<count> \<input_file_name>**|*count 1 or 3*|if count is 1, input file must be P6 color image (.ppm)<br>if count is 3, input files must be P5 grey images (.pgm) in format "image_1.pgm", "image_2.pgm", "image_3.pgm",  where \<input_file_name> is "image.pgm"|
|**-i \<count> \<output_file_name>**|*count 1 or 3*|if count is 1, output file must be P6 color image (.ppm)<br>if count is 3, output files will be P5 grey images (.pgm) in format "image_1.pgm", "image_2.pgm", "image_3.pgm",  where \<output_file_name> is "image.pgm"|
