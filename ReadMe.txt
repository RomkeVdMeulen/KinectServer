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

 * Windows: Kinect API
   * Optional but recommended on Windows: openCV (http://opencv.willowgarage.com)
   * If you use dynimacally linked openCV and get an error for missing tbb_debug.dll
     Read http://stackoverflow.com/questions/7293160/c-tbb-debug-dll-missing
 * CMake (http://www.cmake.org)
   * On Windows: Visual C++ studio (or express)
 * The OpenSceneGraph library (http://www.openscenegraph.com)
   * Precompiled binaries available at http://openscenegraph.alphapixel.com/osg/downloads/free-openscenegraph-binary-downloads
 * The FOX toolkit (http://www.fox-toolkit.org)

****************
* Installation *
****************

Use CMake to create platform specific build files, specifying location of required libraries.
CMake tests the following environmental settings for paths of required libraries:
 * KINECTAPI_DIR, KINECTSDK10_DIR
 * FOX_DIR
 * OPENCV_DIR, OPENCVDIR
 * OSG_DIR, OSGDIR, OSG_SOURCE_DIR

