/* Copyright (C) 2004-2018 MBSim Development Team
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

#include <config.h>
#include "mbsim/contours/cylindrical_gear.h"
#include "mbsim/utils/rotarymatrices.h"

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, CylindricalGear)

  Vec3 CylindricalGear::evalKrPS(const Vec2 &zeta) {
    static Vec3 KrPS(NONINIT);
    double signe = (ext?1:-1);
    double eta = zeta(0);
    double xi = zeta(1);
    double x = -r0*eta;
    double l = (x-signe*xi*tan(be))*pow(sin(al),2)/(pow(sin(al),2)+pow(cos(be)*cos(al),2));
    double a = x-l-signe*xi*tan(be);
    double b = signi*l*cos(be)/tan(al)-r0;
    double c = signe*xi;
    KrPS(0) = a*cos(eta)-b*sin(eta);
    KrPS(1) = a*sin(eta)+b*cos(eta);
    KrPS(2) = c;
    return BasicRotAIKz(k*2*M_PI/N-signi*delh)*KrPS;
  }

  Vec3 CylindricalGear::evalKs(const Vec2 &zeta) {
    static Vec3 Ks;
    double signe = (ext?1:-1);
    double eta = zeta(0);
    double xi = zeta(1);
    double x = -r0*eta;
    double l = (x-signe*xi*tan(be))*pow(sin(al),2)/(pow(sin(al),2)+pow(cos(be)*cos(al),2));
    double a = x-l-signe*xi*tan(be);
    double b = signi*l*cos(be)/tan(al)-r0;
    double ls = -r0*pow(sin(al),2)/(pow(sin(al),2)+pow(cos(be)*cos(al),2));
    double as = -r0-ls;
    double bs = signi*ls*cos(be)/tan(al);
    Ks(0) = as*cos(eta)-a*sin(eta)-bs*sin(eta)-b*cos(eta);
    Ks(1) = as*sin(eta)+a*cos(eta)+bs*cos(eta)-b*sin(eta);
    return BasicRotAIKz(k*2*M_PI/N-signi*delh)*Ks;
  }

  Vec3 CylindricalGear::evalKt(const Vec2 &zeta) {
    static Vec3 Kt(NONINIT);
    double signe = (ext?1:-1);
    double eta = zeta(0);
    double lz = -signe*tan(be)*pow(sin(al),2)/(pow(sin(al),2)+pow(cos(be)*cos(al),2));
    double az = -lz-signe*tan(be);
    double bz = signi*lz*cos(be)/tan(al);
    double cz = signe;
    Kt(0) = az*cos(eta)-bz*sin(eta);
    Kt(1) = az*sin(eta)+bz*cos(eta);
    Kt(2) = cz;
    return BasicRotAIKz(k*2*M_PI/N-signi*delh)*Kt;
  }

  Vec3 CylindricalGear::evalParDer1Ks(const Vec2 &zeta) {
    static Vec3 parDer1Ks;
    double signe = (ext?1:-1);
    double eta = zeta(0);
    double xi = zeta(1);
    double x = -r0*eta;
    double l = (x-signe*xi*tan(be))*pow(sin(al),2)/(pow(sin(al),2)+pow(cos(be)*cos(al),2));
    double a = x-l-signe*xi*tan(be);
    double b = signi*l*cos(be)/tan(al)-r0;
    double ls = -r0*pow(sin(al),2)/(pow(sin(al),2)+pow(cos(be)*cos(al),2));
    double as = -r0-ls;
    double bs = signi*ls*cos(be)/tan(al);
    parDer1Ks(0) = -2*as*sin(eta)-a*cos(eta)-2*bs*cos(eta)+b*sin(eta);
    parDer1Ks(1) = 2*as*cos(eta)-a*sin(eta)-2*bs*sin(eta)-b*cos(eta);
    return BasicRotAIKz(k*2*M_PI/N-signi*delh)*parDer1Ks;
  }

  Vec3 CylindricalGear::evalParDer2Ks(const Vec2 &zeta) {
    static Vec3 parDer2Ks;
    return parDer2Ks;
  }

  Vec3 CylindricalGear::evalParDer1Kt(const Vec2 &zeta) {
    static Vec3 parDer1Kt;
    return parDer1Kt;
  }

  Vec3 CylindricalGear::evalParDer2Kt(const Vec2 &zeta) {
    static Vec3 parDer2Kt;
    return parDer2Kt;
  }

  void CylindricalGear::init(InitStage stage, const InitConfigSet &config) {
    if(stage==preInit) {
      delh = (M_PI/2-(ext?1:-1)*b/m*cos(be))/N;
      r0 = m*N/cos(be)/2;
    }
    else if(stage==plotting) {
      if(plotFeature[openMBV] && openMBVRigidBody) {
        static_pointer_cast<OpenMBV::CylindricalGear>(openMBVRigidBody)->setNumberOfTeeth(N);
        static_pointer_cast<OpenMBV::CylindricalGear>(openMBVRigidBody)->setWidth(w);
        static_pointer_cast<OpenMBV::CylindricalGear>(openMBVRigidBody)->setHelixAngle(be);
        static_pointer_cast<OpenMBV::CylindricalGear>(openMBVRigidBody)->setModule(m);
        static_pointer_cast<OpenMBV::CylindricalGear>(openMBVRigidBody)->setPressureAngle(al);
        static_pointer_cast<OpenMBV::CylindricalGear>(openMBVRigidBody)->setBacklash(b);
        static_pointer_cast<OpenMBV::CylindricalGear>(openMBVRigidBody)->setExternalToothed(ext);
      }
    }
    RigidContour::init(stage, config);
  }

  void CylindricalGear::initializeUsingXML(DOMElement *element) {
    RigidContour::initializeUsingXML(element);
    DOMElement* e;
    e=E(element)->getFirstElementChildNamed(MBSIM%"numberOfTeeth");
    setNumberOfTeeth(E(e)->getText<int>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"width");
    setWidth(E(e)->getText<double>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"helixAngle");
    if(e) setHelixAngle(E(e)->getText<double>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"module");
    if(e) setModule(E(e)->getText<double>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"pressureAngle");
    if(e) setPressureAngle(E(e)->getText<double>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"backlash");
    if(e) setBacklash(E(e)->getText<double>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"externalToothed");
    if(e) setExternalToothed(E(e)->getText<bool>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBV");
    if(e) {
      OpenMBVColoredBody ombv;
      ombv.initializeUsingXML(e);
      openMBVRigidBody=ombv.createOpenMBV<OpenMBV::CylindricalGear>();
    }
  }

}
