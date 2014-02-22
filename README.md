# MAGLI

Originally written by Daniel Schroeder and published at [danielschroeder.me](http://danielschroeder.me/projects/magliv2.html)

The only change I did is compiling only for OS X.

Below is original README file contents

## Doing things with this code
  
  In the off-chance that anybody does something nifty with this
  code, please let me know about it.

  The code isn't terribly well documented, and more importantly the
  comments themselves don't do much to explain at a higher level what's
  being done to create the autostereograms and why. I haven't had the
  time to write up any sort of detailed explanation of what's going on
  with all this.

## Compiling

  I use the `build_magli.sh` script to build all this. Not the most elegant,
  but it gets the job done.

  To get this to compile on other systems, the main two areas that would
  need tweaking are `skeletonglut/skeletonglut.h`, where the OpenGL headers
  are included, and in the aforementioned script, where I include the
  appropriate OpenGL-related flags for `g++`.

## Running

  `magli_viewer.out` takes no arguments. Drag with left and right mouse
  buttons to rotate the scene slowly or quickly.

  In mapped-texture mode, middle-clicking and dragging up and down
  increases or decreases the "texture persistence" of the
  mapped-texture autostereogram.

### Keys:
  `k` or `<space>` toggles between autostereogram, original image, and
  original depth buffer.

  `j` toggles between whether the autostereogram output is one-way
  or mapped-texture.

  `t` prompts the user at the terminal for a new texture repetition
  width; a larger width creates a more dramatic 3D effect but also more
  eyestrain. should be in the 50-150 range or so in practice.

  `m` prompts the user for new space-separated window x and y
  dimensions.

  `r` toggles the direction of the autostereogram render between
  left-to-right and right-to-left.

  `s` saves a .ppm-format screenshot of the window contents to the current
  directory.

  `q` quits.
