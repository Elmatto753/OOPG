#ifndef GLUTIL_H
#define GLUTIL_H

#include "glinc.h"
#include <iostream>

/// Find the last GL error encountered and display with the given label
GLvoid CheckError( const char* const label );

/// Check the frame buffer status
GLvoid CheckFrameBuffer();

#endif // GLUTIL_H
