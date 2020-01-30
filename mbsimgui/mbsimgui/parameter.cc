/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2012 Martin Förg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <config.h>
#include "parameter.h"
#include "objectfactory.h"
#include "utils.h"
#include <xercesc/dom/DOMDocument.hpp>

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  void Parameter::initializeUsingXML(DOMElement *element) {
    this->element = element;
    config = true;
  }

  void Parameter::removeXMLElements() {
    DOMNode *e = element->getFirstChild();
    while(e) {
      DOMNode *en=e->getNextSibling();
      element->removeChild(e);
      e = en;
    }
  }

  DOMElement* Parameter::createXMLElement(DOMNode *parent) {
    DOMDocument *doc=parent->getOwnerDocument();
    element=D(doc)->createElement(PV%getType().toStdString());
    E(element)->setAttribute("name", getType().toStdString());
    parent->insertBefore(element, nullptr);
    return element;
  }

  vector<Parameter*> Parameter::initializeParametersUsingXML(DOMElement *element) {
    vector<Parameter*> param;
    DOMElement *e=element->getFirstElementChild();
    while(e) {
      Parameter *parameter=ObjectFactory::getInstance()->createParameter(e);
      parameter->initializeUsingXML(e);
      param.push_back(parameter);
      e=e->getNextElementSibling();
    }
    return param;
  }

  QString VectorParameter::getValue() const {
    DOMElement *ele=element->getFirstElementChild();
    if(ele and E(ele)->getTagName() == PV%"xmlVector")
      return "xmlVector";
    else
      return Parameter::getValue();
  }

  void VectorParameter::initializeUsingXML(DOMElement *element) {
    Parameter::initializeUsingXML(element);
    DOMElement *ele=element->getFirstElementChild();
    if(ele) {
      if(E(ele)->getTagName() == PV%"xmlVector") {
        DOMElement *ei=ele->getFirstElementChild();
        vector<QString> value;
        while(ei and E(ei)->getTagName()==PV%"ele") {
          value.push_back(QString::fromStdString(X()%E(ei)->getFirstTextChild()->getData()));
          ei=ei->getNextElementSibling();
        }
      }
    }
  }

  QString MatrixParameter::getValue() const {
    DOMElement *ele=element->getFirstElementChild();
    if(ele and E(ele)->getTagName() == PV%"xmlMatrix")
      return "xmlMatrix";
    else
      return Parameter::getValue();
  }

  void MatrixParameter::initializeUsingXML(DOMElement *element) {
    Parameter::initializeUsingXML(element);
    DOMElement *ele=element->getFirstElementChild();
    if(ele) {
      if(E(ele)->getTagName() == PV%"xmlMatrix") {
        DOMElement *ei=ele->getFirstElementChild();
        vector<vector<QString> > value;
        while(ei and E(ei)->getTagName()==PV%"row") {
          DOMElement *ej=ei->getFirstElementChild();
          value.emplace_back();
          while(ej and E(ej)->getTagName()==PV%"ele") {
            value[value.size()-1].push_back(QString::fromStdString(X()%E(ej)->getFirstTextChild()->getData()));
            ej=ej->getNextElementSibling();
          }
          ei=ei->getNextElementSibling();
        }
      }
    }
  }

}
