/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2017 Martin Förg

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
#include "embeditemdata.h"
#include "parameter.h"
#include <xercesc/dom/DOMDocument.hpp>

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  EmbedItemData::~EmbedItemData() {
    for (auto & it : parameter)
      delete it;
    for(auto & i : removedParameter)
      delete i;
  }

//  QString EmbedItemData::getName() const {
//      //return element?QString::fromStdString(MBXMLUtils::E(element)->getAttribute("name")):"Name";
//    return QString::fromStdString(MBXMLUtils::E(element)->getAttribute("name"));
//  }

  void EmbedItemData::addParameter(Parameter *param) {
    parameter.push_back(param);
    param->setParent(this);
  }

  void EmbedItemData::removeParameter(Parameter *param) {
    for (auto it = parameter.begin(); it != parameter.end(); it++) {
      if(*it == param) {
        parameter.erase(it);
        break;
      }
    }
    removedParameter.push_back(param);
  }

  int EmbedItemData::getIndexOfParameter(Parameter *param) const {
    for(size_t i=0; i<parameter.size(); i++)
      if(parameter[i] == param)
        return i;
    return -1;
  }

  void EmbedItemData::removeXMLElement() {
    DOMNode *parent = element->getParentNode();
    if(X()%parent->getNodeName()=="Embed") {
      DOMNode *e = parent->getFirstChild();
      while(e) {
        DOMNode *en=e->getNextSibling();
        parent->removeChild(e);
        e = en;
      }
      DOMNode *ps = parent->getPreviousSibling();
      if(ps and X()%ps->getNodeName()=="#text")
        parent->getParentNode()->removeChild(ps);
      parent->getParentNode()->removeChild(parent);
    }
    else {
      DOMNode *ps = element->getPreviousSibling();
      if(ps and X()%ps->getNodeName()=="#text")
        parent->removeChild(ps);
      parent->removeChild(element);
    }
  }

  DOMElement* EmbedItemData::getParameterXMLElement() {
    DOMDocument *doc=element->getOwnerDocument();
    auto* embed = static_cast<DOMElement*>(element->getParentNode());
    if(X()%embed->getNodeName()!="Embed") {
      DOMElement *ele=D(doc)->createElement(PV%"Embed");
      embed->insertBefore(ele,element);
      embed = ele;
      ele=D(doc)->createElement(PV%"Parameter");
      embed->insertBefore(ele,nullptr);
      embed->insertBefore(element,nullptr);
      return ele;
    }
    else if(X()%embed->getFirstElementChild()->getNodeName()!="Parameter") {
      DOMElement *ele=D(doc)->createElement(PV%"Parameter");
      embed->insertBefore(ele,embed->getFirstElementChild());
      return ele;
    }
    return embed->getFirstElementChild();
  }

  DOMElement* EmbedItemData::processFileID(DOMElement *element) {
//    DOMElement *ele1 = static_cast<DOMElement*>(element->getParentNode());
//    if(MBXMLUtils::E(ele1)->hasAttribute("parameterHref")) {
//      DOMElement *ele2 = static_cast<xercesc::DOMElement*>(element->getOwnerDocument()->importNode(parameter[0]->getXMLElement()->getParentNode(),true));
//      ele1->insertBefore(ele2,ele1->getFirstElementChild());
//      MBXMLUtils::E(ele1)->removeAttribute("parameterHref");
//    }
    return element;
  }

}
