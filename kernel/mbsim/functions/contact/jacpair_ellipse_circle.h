/* Copyright (C) 2004-2010 MBSim Development Team
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: martin.o.foerg@googlemail.com
 */

#ifndef _JACPAIR_ELLIPSE_CIRCLE_H_
#define _JACPAIR_ELLIPSE_CIRCLE_H_

#include <mbsim/functions/contact/jacpair_conesection_circle.h>

namespace MBSim {

  /*!
   * \brief Jacobian of root function for planar pairing Ellipse and Circle
   * \author Thorsten Schindler
   * \date 2009-07-10 some comments (Thorsten Schindler)
   */
  class JacobianPairEllipseCircle : public JacobianPairConeSectionCircle {
    public:
      /*!
       * \brief constructor
       * \param length of first semi-axis
       * \param length of second semi-axis
       */
      JacobianPairEllipseCircle(double a_, double b_) :
          JacobianPairConeSectionCircle(a_, b_) {
      }

      /* INHERITED INTERFACE OF FUNCTION */
      double operator()(const double &phi) override;
      /*************************************************/
  };

  inline double JacobianPairEllipseCircle::operator()(const double &phi) {
    return 2. * (b * (b2(0) * d(0) + b2(1) * d(1) + b2(2) * d(2)) * sin(phi) + a * (b1(0) * d(0) + b1(1) * d(1) + b1(2) * d(2)) * cos(phi) + ((a * a) - (b * b)) * cos(2 * phi));
  }

}

#endif
