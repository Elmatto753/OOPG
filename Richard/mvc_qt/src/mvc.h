#ifndef MVC_H
#define MVC_H

#include <vector>

#ifdef WIN32
#include <array>
#else
#include <tr1/array>
#endif

#include <math.h>
#include <limits>

/// These types are required by the coordinates computation function
typedef std::tr1::array<double,2>  Point2D;
typedef std::tr1::array<int,3>     TriIndex;

/// Compute the MVC (barycentric) coordinates of the query point in the cage coordinates.
int  mean_value_coordinates(const std::vector<Point2D> &cageCoords,
                            const Point2D &queryCoord,
                            std::vector<double> &baryCoords);


#endif // MVC_H
