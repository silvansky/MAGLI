#!/bin/bash
g++ magli.cpp magli_constants.cpp magli_viewer.cpp skeletonglut/skeletonglut.cpp pnm_tools/pnm_tools.cpp shaderloader/shaderloader.cpp -o magli_viewer.out -framework GLUT -framework OpenGL -lGLEW $*
