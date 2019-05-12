/* Copyright (C) 2004-2016 MBSim Development Team
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
#include "mbsim/observers/mechanical_link_observer.h"
#include "mbsim/links/mechanical_link.h"
#include "mbsim/frames/frame.h"
#include <openmbvcppinterface/arrow.h>
#include <openmbvcppinterface/group.h>

using namespace std;
using namespace MBXMLUtils;
using namespace fmatvec;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, MechanicalLinkObserver)

  MechanicalLinkObserver::MechanicalLinkObserver(const std::string &name) : Observer(name), link(nullptr) {
    evalOMBVForceColorRepresentation[0] = &MechanicalLinkObserver::evalNone;
    evalOMBVForceColorRepresentation[1] = &MechanicalLinkObserver::evalAbsoluteForce;
    evalOMBVMomentColorRepresentation[0] = &MechanicalLinkObserver::evalNone;
    evalOMBVMomentColorRepresentation[1] = &MechanicalLinkObserver::evalAbsoluteMoment;
  }

  void MechanicalLinkObserver::init(InitStage stage, const InitConfigSet &config) {
    if(stage==resolveStringRef) {
      if(not saved_link.empty())
        setMechanicalLink(getByPath<MechanicalLink>(saved_link));
      Observer::init(stage, config);
    }
    else if(stage==plotting) {
      Observer::init(stage, config);
      if(plotFeature[openMBV]) {
//      if(openMBVForce) plotColumns.push_back("Force");
//      if(openMBVMoment) plotColumns.push_back("Moment");
        if(ombvForce) {
          openMBVForce.resize(ombvForce->getSideOfInteraction()==2?link->getNumberOfLinks():link->getNumberOfLinks()/2);
          for(size_t i=0; i<openMBVForce.size(); i++) {
            openMBVForce[i]=ombvForce->createOpenMBV();
            //openMBVForce[i]->setName(string("Force")+(i<size_t(link->getNumberOfLinks()/2)?"R":"A")+to_string(i%size_t(link->getNumberOfLinks()/2)));
            openMBVForce[i]->setName(string("Force")+(openMBVForce.size()>1?to_string(i):string("")));
            getOpenMBVGrp()->addObject(openMBVForce[i]);
          }
        }
        if(ombvMoment) {
          openMBVMoment.resize(ombvMoment->getSideOfInteraction()==2?link->getNumberOfLinks():link->getNumberOfLinks()/2);
          for(size_t i=0; i<openMBVMoment.size(); i++) {
            openMBVMoment[i]=ombvMoment->createOpenMBV();
            //openMBVMoment[i]->setName(string("Moment")+(i<size_t(link->getNumberOfLinks()/2)?"R":"A")+to_string(i%size_t(link->getNumberOfLinks()/2)));
            openMBVMoment[i]->setName(string("Moment")+(openMBVMoment.size()>1?to_string(i):string("")));
            getOpenMBVGrp()->addObject(openMBVMoment[i]);
          }
        }
      }
    }
    else
      Observer::init(stage, config);
  }

  void MechanicalLinkObserver::plot() {
    if(plotFeature[openMBV]) {
      if(ombvForce) {
        int off = ombvForce->getSideOfInteraction()==0?link->getNumberOfLinks()/2:0;
        for(size_t i=0; i<openMBVForce.size(); i++) {
          vector<double> data;
          data.push_back(getTime());
          Vec3 toPoint=link->getPointOfApplication(off+i)->evalPosition();
          data.push_back(toPoint(0));
          data.push_back(toPoint(1));
          data.push_back(toPoint(2));
          Vec3 WF = link->evalForce(off+i);
          data.push_back(WF(0));
          data.push_back(WF(1));
          data.push_back(WF(2));
          data.push_back((this->*evalOMBVForceColorRepresentation[ombvForce->getColorRepresentation()])());
          openMBVForce[i]->append(data);
        }
      }
      if(ombvMoment) {
        int off = ombvMoment->getSideOfInteraction()==0?link->getNumberOfLinks()/2:0;
        for(size_t i=0; i<openMBVMoment.size(); i++) {
          vector<double> data;
          data.push_back(getTime());
          Vec3 toPoint=link->getPointOfApplication(off+i)->evalPosition();
          data.push_back(toPoint(0));
          data.push_back(toPoint(1));
          data.push_back(toPoint(2));
          Vec3 WM = link->evalMoment(off+i);
          data.push_back(WM(0));
          data.push_back(WM(1));
          data.push_back(WM(2));
          data.push_back((this->*evalOMBVMomentColorRepresentation[ombvMoment->getColorRepresentation()])());
          openMBVMoment[i]->append(data);
        }
      }
    }
    Observer::plot();
  }

  void MechanicalLinkObserver::initializeUsingXML(DOMElement *element) {
    Observer::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"mechanicalLink");
    saved_link=E(e)->getAttribute("ref");
    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVForce");
    if(e) {
      ombvForce = shared_ptr<OpenMBVInteractionArrow>(new OpenMBVInteractionArrow(0,1,1,OpenMBVArrow::toHead,OpenMBVArrow::toPoint));
      ombvForce->initializeUsingXML(e);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVMoment");
    if(e) {
      ombvMoment = shared_ptr<OpenMBVInteractionArrow>(new OpenMBVInteractionArrow(0,1,1,OpenMBVArrow::toDoubleHead,OpenMBVArrow::toPoint));
      ombvMoment->initializeUsingXML(e);
    }
  }

  double MechanicalLinkObserver::evalAbsoluteForce() {
    return nrm2(link->evalForce());
  }

  double MechanicalLinkObserver::evalAbsoluteMoment() {
    return nrm2(link->evalMoment());
  }

}
