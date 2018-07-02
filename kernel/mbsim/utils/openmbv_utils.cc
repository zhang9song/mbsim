/* Copyright (C) 2004-2013 MBSim Development Team
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
 * Contact: martin.o.foerg@gmail.com
 */

#include <config.h>
#include "mbsim/element.h"
#include "mbsim/utils/openmbv_utils.h"
#include <xercesc/dom/DOMProcessingInstruction.hpp>

using namespace std;
using namespace MBXMLUtils;
using namespace fmatvec;
using namespace xercesc;


namespace MBSim {

  void OpenMBVBody::initializeUsingXML(DOMElement *e) {
    DOMProcessingInstruction *ID = E(e)->getFirstProcessingInstructionChildNamed("OPENMBV_ID");
    if(ID)
      id = X()%ID->getData();
  }

  void OpenMBVBody::initializeObject(const shared_ptr<OpenMBV::Body> &object) {
    object->setID(id);
  }

  void OpenMBVColoredBody::initializeUsingXML(DOMElement *e) {
    OpenMBVBody::initializeUsingXML(e);
    DOMElement *ee;
    ee=E(e)->getFirstElementChildNamed(MBSIM%"diffuseColor");
    if(ee) dc = E(ee)->getText<Vec>(3);
    ee=E(e)->getFirstElementChildNamed(MBSIM%"transparency");
    if(ee) tp = E(ee)->getText<double>();
  }

  void OpenMBVColoredBody::initializeObject(const shared_ptr<OpenMBV::DynamicColoredBody> &object) {
    OpenMBVBody::initializeObject(object);
    object->setDiffuseColor(dc(0),dc(1),dc(2));
    object->setTransparency(tp);
  }

  void OpenMBVDynamicColoredBody::initializeUsingXML(DOMElement *e) {
    OpenMBVColoredBody::initializeUsingXML(e);
    DOMElement *ee;
    ee = E(e)->getFirstElementChildNamed(MBSIM%"colorRepresentation");
    if(ee) {
      string str=string(X()%E(ee)->getFirstTextChild()->getData()).substr(1,string(X()%E(ee)->getFirstTextChild()->getData()).length()-2);
      unsigned int i;
      for(i=0; i<cRL.size(); i++) {
        if(str==cRL[i]) break;
      }
      cR=i;
    }
    ee=E(e)->getFirstElementChildNamed(MBSIM%"minimalColorValue");
    if(ee) minCol = E(ee)->getText<double>();
    ee=E(e)->getFirstElementChildNamed(MBSIM%"maximalColorValue");
    if(ee) maxCol = E(ee)->getText<double>();
  }

  void OpenMBVDynamicColoredBody::initializeObject(const shared_ptr<OpenMBV::DynamicColoredBody> &object) {
    OpenMBVColoredBody::initializeObject(object);
    object->setMinimalColorValue(minCol);
    object->setMaximalColorValue(maxCol);
  }

  void OpenMBVArrow::initializeUsingXML(DOMElement *e) {
    OpenMBVDynamicColoredBody::initializeUsingXML(e);
    DOMElement *ee = E(e)->getFirstElementChildNamed(MBSIM%"scaleLength");
    if(ee) sL = E(ee)->getText<double>();
    ee = E(e)->getFirstElementChildNamed(MBSIM%"scaleSize");
    if(ee) sS = E(ee)->getText<double>();
    ee = E(e)->getFirstElementChildNamed(MBSIM%"type");
    if(ee) {
      string t=string(X()%E(ee)->getFirstTextChild()->getData()).substr(1,string(X()%E(ee)->getFirstTextChild()->getData()).length()-2);
      if(t=="line")            type=OpenMBV::Arrow::line;
      else if(t=="fromHead")        type=OpenMBV::Arrow::fromHead;
      else if(t=="toHead")          type=OpenMBV::Arrow::toHead;
      else if(t=="bothHeads")       type=OpenMBV::Arrow::bothHeads;
      else if(t=="fromDoubleHead")  type=OpenMBV::Arrow::fromDoubleHead;
      else if(t=="toDoubleHead")    type=OpenMBV::Arrow::toDoubleHead;
      else if(t=="bothDoubleHeads") type=OpenMBV::Arrow::bothDoubleHeads;
    }
    ee = E(e)->getFirstElementChildNamed(MBSIM%"referencePoint");
    if(ee) {
      string rP=string(X()%E(ee)->getFirstTextChild()->getData()).substr(1,string(X()%E(ee)->getFirstTextChild()->getData()).length()-2);
      if(rP=="toPoint")   refPoint=OpenMBV::Arrow::toPoint;
      else if(rP=="fromPoint") refPoint=OpenMBV::Arrow::fromPoint;
      else if(rP=="midPoint")  refPoint=OpenMBV::Arrow::midPoint;
    }
  }

  shared_ptr<OpenMBV::Arrow> OpenMBVArrow::createOpenMBV() {
    shared_ptr<OpenMBV::Arrow> object = OpenMBV::ObjectFactory::create<OpenMBV::Arrow>();
    initializeObject(object);
    return object;
  }

  void OpenMBVArrow::initializeObject(const shared_ptr<OpenMBV::Arrow> &object) {
    OpenMBVDynamicColoredBody::initializeObject(object);
    object->setDiameter(0.25*sS);
    object->setHeadDiameter(0.5*sS);
    object->setHeadLength(0.75*sS);
    object->setType(type);
    object->setReferencePoint(refPoint);
    object->setScaleLength(sL);
  }

  void OpenMBVFrame::initializeUsingXML(DOMElement *e) {
    OpenMBVColoredBody::initializeUsingXML(e);
    DOMElement *ee = E(e)->getFirstElementChildNamed(MBSIM%"size");
    if(ee) size = E(ee)->getText<double>();
    ee = E(e)->getFirstElementChildNamed(MBSIM%"offset");
    if(ee) offset = E(ee)->getText<double>();
  }

  shared_ptr<OpenMBV::Frame> OpenMBVFrame::createOpenMBV() {
    shared_ptr<OpenMBV::Frame> object = OpenMBV::ObjectFactory::create<OpenMBV::Frame>();
    initializeObject(object);
    return object;
  }

  void OpenMBVFrame::initializeObject(const shared_ptr<OpenMBV::Frame> &object) {
    OpenMBVColoredBody::initializeObject(object);
    object->setSize(size);
    object->setOffset(offset);
  }

  void OpenMBVSphere::initializeUsingXML(DOMElement *e) {
    OpenMBVColoredBody::initializeUsingXML(e);
    DOMElement *ee;
    ee = E(e)->getFirstElementChildNamed(MBSIM%xml);
    if(ee) r = E(ee)->getText<double>();
  }

  shared_ptr<OpenMBV::Sphere> OpenMBVSphere::createOpenMBV() {
    shared_ptr<OpenMBV::Sphere> object = OpenMBV::ObjectFactory::create<OpenMBV::Sphere>();
    initializeObject(object);
    return object;
  }

  void OpenMBVSphere::initializeObject(const shared_ptr<OpenMBV::Sphere> &object) {
    OpenMBVColoredBody::initializeObject(object);
    object->setRadius(r);
  }

  void OpenMBVLine::initializeUsingXML(DOMElement *e) {
    OpenMBVColoredBody::initializeUsingXML(e);
    DOMElement *ee;
    ee = E(e)->getFirstElementChildNamed(MBSIM%"length");
    if(ee) l = E(ee)->getText<double>();
  }

  shared_ptr<OpenMBV::Cuboid> OpenMBVLine::createOpenMBV() {
    shared_ptr<OpenMBV::Cuboid> object = OpenMBV::ObjectFactory::create<OpenMBV::Cuboid>();
    initializeObject(object);
    return object;
  }

  void OpenMBVLine::initializeObject(const shared_ptr<OpenMBV::Cuboid> &object) {
    OpenMBVColoredBody::initializeObject(object);
    object->setLength(0,l,0);
  }

  void OpenMBVPlane::initializeUsingXML(DOMElement *e) {
    OpenMBVColoredBody::initializeUsingXML(e);
    DOMElement *ee;
    ee = E(e)->getFirstElementChildNamed(MBSIM%"length");
    if(ee) l = E(ee)->getText<Vec>(2);
  }

  shared_ptr<OpenMBV::Cuboid> OpenMBVPlane::createOpenMBV() {
    shared_ptr<OpenMBV::Cuboid> object = OpenMBV::ObjectFactory::create<OpenMBV::Cuboid>();
    initializeObject(object);
    return object;
  }

  void OpenMBVPlane::initializeObject(const shared_ptr<OpenMBV::Cuboid> &object) {
    OpenMBVColoredBody::initializeObject(object);
    object->setLength(0,l(0),l(1));
  }

  void OpenMBVCuboid::initializeUsingXML(DOMElement *e) {
    OpenMBVColoredBody::initializeUsingXML(e);
    DOMElement *ee;
    ee = E(e)->getFirstElementChildNamed(MBSIM%"length");
    if(ee) l = E(ee)->getText<Vec>(3);
  }

  shared_ptr<OpenMBV::Cuboid> OpenMBVCuboid::createOpenMBV() {
    shared_ptr<OpenMBV::Cuboid> object = OpenMBV::ObjectFactory::create<OpenMBV::Cuboid>();
    initializeObject(object);
    return object;
  }

  void OpenMBVCuboid::initializeObject(const shared_ptr<OpenMBV::Cuboid> &object) {
    OpenMBVColoredBody::initializeObject(object);
    object->setLength(l(0),l(1),l(2));
  }

  void OpenMBVCircle::initializeUsingXML(DOMElement *e) {
    OpenMBVColoredBody::initializeUsingXML(e);
    DOMElement *ee;
    ee = E(e)->getFirstElementChildNamed(MBSIM%"radius");
    if(ee) r = E(ee)->getText<double>();
  }

  shared_ptr<OpenMBV::Frustum> OpenMBVCircle::createOpenMBV() {
    shared_ptr<OpenMBV::Frustum> object = OpenMBV::ObjectFactory::create<OpenMBV::Frustum>();
    initializeObject(object);
    return object;
  }

  void OpenMBVCircle::initializeObject(const shared_ptr<OpenMBV::Frustum> &object) {
    OpenMBVColoredBody::initializeObject(object);
    object->setTopRadius(r);
    object->setBaseRadius(r);
    object->setHeight(0);
  }

  void OpenMBVFrustum::initializeUsingXML(DOMElement *e) {
    OpenMBVColoredBody::initializeUsingXML(e);
    DOMElement *ee;
    ee = E(e)->getFirstElementChildNamed(MBSIM%"topRadius");
    if(ee) t = E(ee)->getText<double>();
    ee = E(e)->getFirstElementChildNamed(MBSIM%"baseRadius");
    if(ee) b = E(ee)->getText<double>();
    ee = E(e)->getFirstElementChildNamed(MBSIM%"height");
    if(ee) h = E(ee)->getText<double>();
  }

  shared_ptr<OpenMBV::Frustum> OpenMBVFrustum::createOpenMBV() {
    shared_ptr<OpenMBV::Frustum> object = OpenMBV::ObjectFactory::create<OpenMBV::Frustum>();
    initializeObject(object);
    return object;
  }

  void OpenMBVFrustum::initializeObject(const shared_ptr<OpenMBV::Frustum> &object) {
    OpenMBVColoredBody::initializeObject(object);
    object->setTopRadius(t);
    object->setBaseRadius(b);
    object->setHeight(h);
  }

  void OpenMBVExtrusion::initializeUsingXML(DOMElement *e) {
    OpenMBVColoredBody::initializeUsingXML(e);
    DOMElement *ee = E(e)->getFirstElementChildNamed(MBSIM%"height");
    if(ee) h = E(ee)->getText<double>();
  }

  shared_ptr<OpenMBV::Extrusion> OpenMBVExtrusion::createOpenMBV() {
    shared_ptr<OpenMBV::Extrusion> object = OpenMBV::ObjectFactory::create<OpenMBV::Extrusion>();
    initializeObject(object);
    return object;
  }

  void OpenMBVExtrusion::initializeObject(const shared_ptr<OpenMBV::Extrusion> &object) {
    OpenMBVColoredBody::initializeObject(object);
    object->setHeight(h);
  }

  void OpenMBVCoilSpring::initializeUsingXML(DOMElement *e) {
    OpenMBVDynamicColoredBody::initializeUsingXML(e);
    DOMElement *ee = E(e)->getFirstElementChildNamed(MBSIM%"numberOfCoils");
    if(ee) n = E(ee)->getText<int>();
    ee = E(e)->getFirstElementChildNamed(MBSIM%"springRadius");
    if(ee) r = E(ee)->getText<double>();
    ee = E(e)->getFirstElementChildNamed(MBSIM%"crossSectionRadius");
    if(ee) cr = E(ee)->getText<double>();
    ee = E(e)->getFirstElementChildNamed(MBSIM%"scaleFactor");
    if(ee) sf = E(ee)->getText<double>();
    ee = E(e)->getFirstElementChildNamed(MBSIM%"nominalLength");
    if(ee) l = E(ee)->getText<double>();
    ee = E(e)->getFirstElementChildNamed(MBSIM%"type");
    if(ee) {
      string typeStr=string(X()%E(ee)->getFirstTextChild()->getData()).substr(1,string(X()%E(ee)->getFirstTextChild()->getData()).length()-2);
      if(typeStr=="tube") type=OpenMBV::CoilSpring::tube;
      else if(typeStr=="scaledTube") type=OpenMBV::CoilSpring::scaledTube;
      else if(typeStr=="polyline") type=OpenMBV::CoilSpring::polyline;
    }
  }

  shared_ptr<OpenMBV::CoilSpring> OpenMBVCoilSpring::createOpenMBV() {
    shared_ptr<OpenMBV::CoilSpring> object = OpenMBV::ObjectFactory::create<OpenMBV::CoilSpring>();
    initializeObject(object);
    return object;
  }

  void OpenMBVCoilSpring::initializeObject(const shared_ptr<OpenMBV::CoilSpring> &object) {
    OpenMBVDynamicColoredBody::initializeObject(object);
    object->setSpringRadius(r);
    object->setCrossSectionRadius(cr);
    object->setScaleFactor(sf);
    object->setNumberOfCoils(n);
    object->setNominalLength(l);
    object->setType(type);
  }

  shared_ptr<OpenMBV::IndexedLineSet> OpenMBVIndexedLineSet::createOpenMBV() {
    shared_ptr<OpenMBV::IndexedLineSet> object = OpenMBV::ObjectFactory::create<OpenMBV::IndexedLineSet>();
    initializeObject(object);
    return object;
  }

  shared_ptr<OpenMBV::IndexedFaceSet> OpenMBVIndexedFaceSet::createOpenMBV() {
    shared_ptr<OpenMBV::IndexedFaceSet> object = OpenMBV::ObjectFactory::create<OpenMBV::IndexedFaceSet>();
    initializeObject(object);
    return object;
  }

  shared_ptr<OpenMBV::NurbsCurve> OpenMBVNurbsCurve::createOpenMBV() {
    shared_ptr<OpenMBV::NurbsCurve> object = OpenMBV::ObjectFactory::create<OpenMBV::NurbsCurve>();
    initializeObject(object);
    return object;
  }

  shared_ptr<OpenMBV::NurbsSurface> OpenMBVNurbsSurface::createOpenMBV() {
    shared_ptr<OpenMBV::NurbsSurface> object = OpenMBV::ObjectFactory::create<OpenMBV::NurbsSurface>();
    initializeObject(object);
    return object;
  }

}
