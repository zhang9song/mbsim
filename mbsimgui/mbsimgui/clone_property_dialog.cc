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
#include "clone_property_dialog.h"
#include "element.h"
#include "basic_widgets.h"

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  ClonePropertyDialog::ClonePropertyDialog(Element *element) : EmbedItemPropertyDialog(element) {
    addTab("General");
    name = new ExtWidget("Name",new TextWidget(QString::fromStdString(MBXMLUtils::E(item->getXMLElement())->getAttribute("name"))));
    name->setToolTip("Set the name of the element");
    addToTab("General", name);
    clone = new ExtWidget("Clone",new CloneWidget,true,false);
    connect(clone,&Widget::widgetChanged,this,&ClonePropertyDialog::updateName);
    addToTab("General",clone);
  }

  DOMElement* ClonePropertyDialog::initializeUsingXML(DOMElement *parent) {
    static_cast<TextWidget*>(name->getWidget())->setText(QString::fromStdString(MBXMLUtils::E(item->getXMLElement())->getAttribute("name")));
    DOMElement *embed = item->getEmbedXMLElement();
    if(embed) {
      clone->setActive(E(embed)->hasAttribute("count"));
      if(E(embed)->hasAttribute("count")) static_cast<CloneWidget*>(clone->getWidget())->setCount(QString::fromStdString(E(embed)->getAttribute("count")));
      if(E(embed)->hasAttribute("counterName")) static_cast<CloneWidget*>(clone->getWidget())->setCounterName(QString::fromStdString(E(embed)->getAttribute("counterName")));
    }
    return parent;
  }

  DOMElement* ClonePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    E(item->getXMLElement())->setAttribute("name",static_cast<TextWidget*>(name->getWidget())->getText().toStdString());
    item->updateName();
    DOMElement *embedNode = item->getEmbedXMLElement();
    if(clone->isActive()) {
      if(not embedNode) embedNode = item->createEmbedXMLElement();
      E(embedNode)->setAttribute("count",static_cast<CloneWidget*>(clone->getWidget())->getCount().toStdString());
      E(embedNode)->setAttribute("counterName",static_cast<CloneWidget*>(clone->getWidget())->getCounterName().toStdString());
    }
    else if(embedNode) {
      E(embedNode)->removeAttribute("count");
      E(embedNode)->removeAttribute("counterName");
    }
    item->maybeRemoveEmbedXMLElement();
    item->updateStatus();
    return nullptr;
  }

  void ClonePropertyDialog::updateName() {
    TextWidget *textWidget = static_cast<TextWidget*>(name->getWidget());
    if(clone->isActive()) {
      QString text = textWidget->getText();
      int i1 = text.indexOf("{");
      if(i1!=-1) {
        int i2 = text.indexOf("}");
        textWidget->setText(text.replace(i1+1,i2-i1-1,static_cast<CloneWidget*>(clone->getWidget())->getCounterName()));
      }
      else
        textWidget->setText(textWidget->getText()+"{"+static_cast<CloneWidget*>(clone->getWidget())->getCounterName()+"}");
    }
    else {
      QString text = textWidget->getText();
      int i1 = text.indexOf("{");
      int i2 = text.indexOf("}");
      textWidget->setText(text.remove(i1,i2-i1+1));
    }
  }

}
