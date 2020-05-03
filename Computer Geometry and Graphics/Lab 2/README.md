## Line drawing with smoothing and gamma correction

This simple console application allows you to draw lines of various thickness on P5 PNM images

**Arguments format: binary_execurion_file <input_file_name> <output_file_name> \<brightness> \<thickness> \<x0> \<y0> \<x1> \<y1> \<gamma>**
>**Note**: All arguments are reqired except for gamma

| Argument | Format | Description |
|---|---|---|
|**<input_file_name>**|*Path ending with .npm file*|Name of the input file|
|**<output_file_name>**|*Path ending with .npm file*|Name of the outnput file|
|**\<brightness>**|*Number between 0 and 255*|0 - minimum, 255 - maximum|
|**\<thickness>**|*Positive real number*||
|**\<x0> \<y0>**|*Positive real numbers*|Start coordinates|
|**\<x1> \<y1>**|*Positive real numbers*|End coordinates|
|**\<gamma>**|*Positive real number*|Gamma value, omitting this argument or value of 0 equals to sRGB|
