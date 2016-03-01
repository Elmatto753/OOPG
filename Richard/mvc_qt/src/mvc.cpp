/*
#include <iomanip>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <stdio.h>
#include <algorithm>
#include <math.h>
*/
#include <assert.h>
#include "mvc.h"



/**
  * Reference: Mean Value Coordinates for Arbitrary Planar Polygons: Kai Hormann and Michael Floater;
  * \param cageCoords Coordinates of closed polygon in the Counter clockwise direction. The input is not tested inside. (input)
  * \param queryCoord the xyCoords of the query Point (input)
  * \param baryCoords: baryCentric Coords of the query Point (output)
  * \returns 0 on success
  * \author Chaman Singh Verma, University of Wisconsin at Madison, 18th March, 2011.
  * \author Richard Southern, Bournemouth University, September 2012
  */
int mean_value_coordinates( const std::vector<Point2D> &cageCoords,
                            const Point2D &queryCoord,
                            std::vector<double> &baryCoords)
{
    int nSize = cageCoords.size();

    assert( nSize );

    double dx, dy;

    std::vector<Point2D>  s(nSize);
    for( int i = 0; i < nSize; i++)
    {
        dx  =   cageCoords[i][0] - queryCoord[0];
        dy  =   cageCoords[i][1] - queryCoord[1];
        s[i][0]  =   dx;
        s[i][1]  =   dy;
    }

    baryCoords.resize(nSize);
    for( int i = 0; i < nSize; i++)
        baryCoords[i] = 0.0;

    int ip, im;      // (i+1) and (i-1)
    double ri, rp, Ai, Di, dl, mu;  // Distance
    double eps = 10.0*std::numeric_limits<double>::min();

    // First check if any coordinates close to the cage point or
    // lie on the cage boundary. These are special cases.
    for( int i = 0; i < nSize; i++)
    {
        ip = (i+1)%nSize;
        ri = sqrt( s[i][0]*s[i][0] + s[i][1]*s[i][1] );
        Ai = 0.5*(s[i][0]*s[ip][1] - s[ip][0]*s[i][1]);
        Di = s[ip][0]*s[i][0] + s[ip][1]*s[i][1];
        if( ri <= eps)
        {
            baryCoords[i] = 1.0;
            return 0;
        }
        else if( fabs(Ai) <= 0 && Di < 0.0)
        {
            dx = cageCoords[ip][0] - cageCoords[i][0];
            dy = cageCoords[ip][1] - cageCoords[i][1];
            dl = sqrt(dx*dx + dy*dy);
            assert(dl > eps);
            dx = queryCoord[0] - cageCoords[i][0];
            dy = queryCoord[1] - cageCoords[i][1];
            mu = sqrt(dx*dx + dy*dy)/dl;
            assert( mu >= 0.0 && mu <= 1.0);
            baryCoords[i]  = 1.0-mu;
            baryCoords[ip] = mu;
            return 0;
        }
    }

    // Page #12, from the paper
    std::vector<double> tanalpha(nSize); // tan(alpha/2)
    for( int i = 0; i < nSize; i++)
    {
        ip = (i+1)%nSize;
        im = (nSize-1+i)%nSize;
        ri = sqrt( s[i][0]*s[i][0] + s[i][1]*s[i][1] );
        rp = sqrt( s[ip][0]*s[ip][0] + s[ip][1]*s[ip][1] );
        Ai = 0.5*(s[i][0]*s[ip][1] - s[ip][0]*s[i][1]);
        Di = s[ip][0]*s[i][0] + s[ip][1]*s[i][1];
        tanalpha[i] = (ri*rp - Di)/(2.0*Ai);
    }

    // Equation #11, from the paper
    double wi, wsum = 0.0;
    for( int i = 0; i < nSize; i++)
    {
        im = (nSize-1+i)%nSize;
        ri = sqrt( s[i][0]*s[i][0] + s[i][1]*s[i][1] );
        wi = 2.0*( tanalpha[i] + tanalpha[im] )/ri;
        wsum += wi;
        baryCoords[i] = wi;
    }

    if( fabs(wsum ) > 0.0)
    {
        for( int i = 0; i < nSize; i++)
            baryCoords[i] /= wsum;
    }

    return 0;
}

