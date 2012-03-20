####################################
#         Kinect Data Server       #
####################################

A small application that allows you to serve input from the Microsoft Kinect over a TCP port.

Additional features:
 * Record to/ play from file
 * Interactively create a calibration and use this on Kinect data
 
*************
* Platforms *
*************

The full server can only be used on Windows (the Kinect API is only available on Windows).
You can compile this on other platforms, but in this case you can only play back pre-recorded Kinect data from files created on Windows.
Cross compilation has only been checked on Linux so far.

****************
* Requirements *
****************

 * CMake (http://www.cmake.org)
 * The OpenSceneGraph library (http://www.openscenegraph.com)
   * Precompiled binaries available at http://openscenegraph.alphapixel.com/osg/downloads/free-openscenegraph-binary-downloads
 * The FOX toolkit (http://www.fox-toolkit.org)

****************
* Installation *
****************

Use CMake to create platform specific build files, specifying location of required libraries.
