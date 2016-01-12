/*
  Copyright (C) 2012 Xiaosong Yang

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CONVEX_HULL_H__
#define CONVEX_HULL_H__
/// @file ConvexHull.h
/// @brief 2D ConvexHull algorithm - Gift Wrapping
// must include types.h first for ngl::Real and GLEW if required
#include <ngl/Types.h>
#include <ngl/Vec3.h>
#include <vector>
#include <ngl/VertexArrayObject.h>
#include <ngl/Vec2.h>

//----------------------------------------------------------------------------------------------------------------------
/// @class ConvexHull  "include/ConvexHull.h"
/// @brief Construct 2D convex hull for the input set of 3D points, Only consider 2D ConvexHull based on XY plane
/// @author Xiaosong Yang
/// @version 1.0
/// @date Last Revision 29/12/2012
/// @todo update this class to use more consistent datatype with NGL
//----------------------------------------------------------------------------------------------------------------------
class ConvexHull
{
public :
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief default Ctor
    //----------------------------------------------------------------------------------------------------------------------
    ConvexHull();
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief Ctor passing in an Array of 3D points
    ///  @param[in] _p an array of 3D points
	///  @param[in] _nPoints the size of the Point Array
    //----------------------------------------------------------------------------------------------------------------------
    ConvexHull(const ngl::Vec2 *_p, int _nPoints);

    //----------------------------------------------------------------------------------------------------------------------
	/// @brief copy ctor
    /// @param _c the curve to copy
    //----------------------------------------------------------------------------------------------------------------------
    ConvexHull(	const ConvexHull &_c );
    //----------------------------------------------------------------------------------------------------------------------
	/// @brief destructor
    //----------------------------------------------------------------------------------------------------------------------
    ~ConvexHull();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Draw the convex hull and the input point set
    //----------------------------------------------------------------------------------------------------------------------
    void drawPoints() const;
    void drawConvexHull() const;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Creat the VAO objects
    //----------------------------------------------------------------------------------------------------------------------
    void createVAO();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Creat the ConvexHull
    //----------------------------------------------------------------------------------------------------------------------
    void computeConvexHull();

protected :

    //----------------------------------------------------------------------------------------------------------------------
    /// @brief The Convex hull polygon
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<unsigned int>  		m_ch;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief The ammount of input points
    //----------------------------------------------------------------------------------------------------------------------
    unsigned int m_numP;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief  the input point set
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<ngl::Vec2>        m_p;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a vertex array object for our hull drawing
    //----------------------------------------------------------------------------------------------------------------------
    ngl::VertexArrayObject *m_vaoHull;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a vertex array object for our point drawing
    //----------------------------------------------------------------------------------------------------------------------
    ngl::VertexArrayObject *m_vaoPoints;


}; // end class ConvexHull

#endif // end header file

//----------------------------------------------------------------------------------------------------------------------
