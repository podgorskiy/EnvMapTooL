#[EnvMapTool](https://github.com/podgorskiy/EnvMapTool) - Cross-platform tool for working with environmental maps. 
============================================================================

Performs a set of different actions:

 * Converts cube map to sphere map
 * Converts sphere map to cube map
 * Blurs cubmap
 * Assembles cubemaps from a set of images
 * Disassembles cubemap to a set of images 

Supported formats fow read/write:

 * DDS. Not compressed.
 * TARGA
 * PNG

#Usage:
Input and output files can be specified in two ways:

 * As single file. Use keys -i / -o
 * As six files that represent x+, x-, y+, y-, z+, z- faces, use keys -I / -O 

Specify output format with key -f. It can be: TGA, PNG, DDS. For example `-f PNG`. Default is TGA.

Specify desired action:

 * `cube2sphere` - converts cube map to sphere map
 * `sphere2cube` - converts sphere map to cube map
 * `blurCubemap` - blurs cubemap using Monte Carlo method. Accurate but slow approach.
 * `fastBlurCubemap` - blurs cubemap using Monte Carlo method. Inaccurate but fast approach.
 * `convert` - this action does nothing. Should be used to convert cubemap from one format to other.

If you specifed a single file for output, that does not support multiple faces (not DDS format), than omly one face will be written. This face can be specified by flag `-F`

You may specify gamma for input texture `-g`, and gamma for output texture `-G`. Default is 2.2

You may specify size of output texture using keys `-W` and `-H`.

For actions *blurCubemap* and *fastBlurCubemap* you may set bluring radius by use of key `-b`

For action *blurCubemap* blur quality can be specified by key `-q`. Effects the number of samples in Monte Carlo integration. Reasonable values are between 4 - 10. Large values will increase calculation time dramatically. Default is 4

#Exapmles
----------------------------------------------------------------------------
##Converting DDS to a series of png files: 
Let we have cubemap in DDS format: uffizi_cros.dds
To convert it to six *.png files that represent x+, x-, y+, y-, z+, z- faces you need to execite following command:
```
EnvMapTool -i uffizi_cros.dds -O 1.png -O 2.png -O 3.png -O 4.png -O 5.png -O 6.png -f PNG convert
```
The output will be:

![xp](https://cloud.githubusercontent.com/assets/3229783/9427142/57dc08f4-493a-11e5-9345-2d78482f615d.png)
![xm](https://cloud.githubusercontent.com/assets/3229783/9427145/73e78fc8-493a-11e5-945c-bf870b41db3a.png)
![yp](https://cloud.githubusercontent.com/assets/3229783/9427148/9a02fba2-493a-11e5-9a3b-519753b706c8.png)
![ym](https://cloud.githubusercontent.com/assets/3229783/9427150/9c48470a-493a-11e5-9fe7-d879d4271c33.png)
![zp](https://cloud.githubusercontent.com/assets/3229783/9427151/9e4ef850-493a-11e5-812c-1740b645658b.png)
![zm](https://cloud.githubusercontent.com/assets/3229783/9427153/b51c0488-493a-11e5-8a87-4adeae8d2d97.png)

##Converting DDS cubemap to sphere map
Lets convert uffizi_cros.dds to sphere map and specify size of sphere map of 765x765:
```
EnvMapTool -i uffizi_cros.dds -o sphere.png -W 765 -H 765 -f PNG cube2sphere
```
![sphere](https://cloud.githubusercontent.com/assets/3229783/9427205/38cc2576-493e-11e5-830b-140eb3495635.png)

##Converting sphere map to cubemap
Lets convert generated in previous example sphere map to a series of png texture of size 256x256:
```
EnvMapTool -i sphere.png -O xp_.png -O xm_.png -O yp_.png -O ym_.png -O zp_.png -O zm_.png -W 256 -H 256 -f PNG sphere2cube
```

![xp_](https://cloud.githubusercontent.com/assets/3229783/9427223/d79eb7fe-493e-11e5-96cb-b2e19ceaf6b0.png)
![xm_](https://cloud.githubusercontent.com/assets/3229783/9427221/d5a96bf6-493e-11e5-9279-7b00cec3cfa8.png)
![yp_](https://cloud.githubusercontent.com/assets/3229783/9427225/dc91d07a-493e-11e5-936e-7b9587a209ed.png)
![ym_](https://cloud.githubusercontent.com/assets/3229783/9427224/da638cb2-493e-11e5-9e1a-fd43d17ae7aa.png)
![zp_](https://cloud.githubusercontent.com/assets/3229783/9427228/e0585396-493e-11e5-843b-fedc8914b3b3.png)
![zm_](https://cloud.githubusercontent.com/assets/3229783/9427227/de91b318-493e-11e5-92d6-50c1e24426cb.png)

##Bluring cubemap using Monte-Carlo approach and converting it to spheremap:
```
EnvMapTool -i uffizi_cros.dds -O 1.png -O 2.png -O 3.png -O 4.png -O 5.png -O 6.png -W 256 -H 256 -f PNG blurCubemap -b 60 -q 10
EnvMapTool -I 1.png -I 2.png -I 3.png -I 4.png -I 5.png -I 6.png -o bluredSphere.png -W 765 -H 765 -f PNG cube2sphere
```
![bluredsphere](https://cloud.githubusercontent.com/assets/3229783/9427291/0fa9994e-4943-11e5-8561-ab076144bc67.png)

##The same as above, but using fast blur:
```
EnvMapTool -i uffizi_cros.dds -O 1.png -O 2.png -O 3.png -O 4.png -O 5.png -O 6.png -f PNG fastBlurCubemap -b 30
EnvMapTool -I 1.png -I 2.png -I 3.png -I 4.png -I 5.png -I 6.png -o fastBluredSphere.png -W 765 -H 765 -f PNG cube2sphere
```
![fastbluredsphere](https://cloud.githubusercontent.com/assets/3229783/9427292/1b0e2e8a-4943-11e5-8d9a-07ba8844a7d2.png)

##Detailed usage:
```
USAGE: 

   ./EnvMapTool  {-o <Output file>|-O <Output files> ... } {-i <Input file>
                 |-I <Input files> ... } [-f <Output format>] [-F <Face to
                 write>] [-q <Blur quality>] [-b <Blur radius>] [-l] [-g
                 <Input gamma>] [-G <Output gamma>] [-H <Output texture
                 height>] [-W <Output texture width>] [--version] [-h]
                 <cube2sphere|sphere2cube|blurCubemap|fastBlurCubemap
                 |convert>


Where: 

   -o <Output file>,  --output <Output file>
     (OR required)  The output texture file.
         -- OR --
   -O <Output files>,  --outputSequence <Output files>  (accepted multiple
      times)
     (OR required)  The output texture files for cube map. You need specify
     six files: xp, xn yp, yn, zp, zn


   -i <Input file>,  --input <Input file>
     (OR required)  The input texture file. Can be of the following
     formats: *.tga, *.png, *.dds
         -- OR --
   -I <Input files>,  --inputSequence <Input files>  (accepted multiple
      times)
     (OR required)  The input texture files for cube map. You need specify
     six files: xp, xn yp, yn, zp, zn. WARNING! All the files MUST be the
     same format and size!


   -f <Output format>,  --format <Output format>
     Output texture file format. Can be one of the following "TGA", "DDS",
     "PNG". Default TGA.

   -F <Face to write>,  --faceToWrite <Face to write>
     If cubemap texture is written to format that does not support faces,
     this face will be written

   -q <Blur quality>,  --blurQuality <Blur quality>
     Effects the number of samples in Monte Carlo integration. Reasonable
     values are between 4 - 8. Large values will increase calculation time
     dramatically. Default is 4

   -b <Blur radius>,  --blurRadius <Blur radius>
     Gaussian blur radius. Default is 10.0

   -l,  --leaveOuter
     If flag is set, than while cubemap -> sphere transform area around the
     sphere circule are not filled black, but represent mathematical
     extrapolation.

   -g <Input gamma>,  --inputGamma <Input gamma>
     Gamma of input texture. Default is 2.2

   -G <Output gamma>,  --outputGamma <Output gamma>
     Gamma of output texture. Default is 2.2

   -H <Output texture height>,  --outputHeight <Output texture height>
     Height of output texture. Default is the same as input, or 4 times
     upscaled in case of cube2sphere transform, or 4 times downscaled in
     case of sphere2cube transform

   -W <Output texture width>,  --outputWidth <Output texture width>
     Width of output texture. Default is the same as input, or 4 times
     upscaled in case of cube2sphere transform, or 4 times downscaled in
     case of sphere2cube transform

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.

   <cube2sphere|sphere2cube|blurCubemap|fastBlurCubemap|convert>
     (required)  Action. Can be:

     	cube2sphere - Converts cube map texture to spherical map

     	sphere2cube - Converts spherical map texture to cube map

     	blurCubemap - Gaussian blur of cubemap

     	convert - Do nothing. Just to convert txture from one format to
     other
```
