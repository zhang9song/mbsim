/* Copyright (C) 2004-2006  Martin Förg
 
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
 * Contact:
 *   martin.o.foerg@googlemail.com
 *
 */

#ifndef _RADAU5_INTEGRATOR_H_
#define _RADAU5_INTEGRATOR_H_

#include "integrator.h"

namespace MBSimIntegrator {

  /** \brief DAE-Integrator RADAU5
  */
  class RADAU5Integrator : public Integrator {
    public:
      enum Formalism {
        ODE=0,
        DAE=1
      };

    private:
      typedef void (*massptr)(int* n, double* m, int* lmas, double* rpar, int* ipar);

      static massptr mass;
      static void fzdotODE(int* n, double* t, double* z, double* zd, double* rpar, int* ipar);
      static void fzdotDAE(int* n, double* t, double* y, double* yd, double* rpar, int* ipar);
      static void massidentity(int* n, double* m, int* lmas, double* rpar, int* ipar);
      static void massreducedidentity(int* n, double* m, int* lmas, double* rpar, int* ipar);
      static void plot(int* nr, double* told, double* t, double* z, double* cont, int* lrc, int* n, double* rpar, int* ipar, int* irtrn);

      double tPlot{0};
      double dtOut{0};
      std::ofstream integPlot;
      double s0; 
      double time{0}; 

      /** Absolute Toleranz */
      fmatvec::Vec aTol;
      /** Relative Toleranz */
      fmatvec::Vec rTol;
      /** step size for the first step */
      double dt0{0};
      /** maximum number of steps */
      int maxSteps{0};
      /** maximal step size */
      double dtMax{0};
      /** formalism **/
      Formalism formalism{ODE};
      /** coordinate transformation **/
      bool specialStructure{true};

    public:

      ~RADAU5Integrator() override = default;

      void setAbsoluteTolerance(const fmatvec::Vec &aTol_) { aTol = aTol_; }
      void setAbsoluteTolerance(double aTol_) { aTol = fmatvec::Vec(1,fmatvec::INIT,aTol_); }
      void setRelativeTolerance(const fmatvec::Vec &rTol_) { rTol = rTol_; }
      void setRelativeTolerance(double rTol_) { rTol = fmatvec::Vec(1,fmatvec::INIT,rTol_); }
      void setInitialStepSize(double dt0_) { dt0 = dt0_; }
      void setMaximumStepSize(double dtMax_) { dtMax = dtMax_; }
      void setStepLimit(int maxSteps_) { maxSteps = maxSteps_; }
      void setFormalism(Formalism formalism_) { formalism = formalism_; }
      void setSpecialStructure(bool specialStructure_) { specialStructure = specialStructure_; }

      using Integrator::integrate;
      void integrate() override;

      void initializeUsingXML(xercesc::DOMElement *element) override;
  };

}

#endif
