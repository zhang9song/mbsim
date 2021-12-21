/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2021 Martin Förg

  This library is free software; you can redistribute it and/or 
  modify it under the terms of the GNU Lesser General Public 
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version. 
   
  This library is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  Lesser General Public License for more details. 
   
  You should have received a copy of the GNU Lesser General Public 
  License along with this library; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
*/

#ifndef _EXPORTER__H_
#define _EXPORTER__H_

#include "solver.h"
#include "solver_property_dialog.h"

namespace XERCES_CPP_NAMESPACE {
  class DOMElement;
  class DOMNode;
}

namespace MBSimGUI {

  class LTISystemExporter : public Solver {
    public:
      MBXMLUtils::FQN getXMLType() const override { return MBSIMCONTROL%"LTISystemExporter"; }
      QString getType() const override { return "LTI system exporter"; }
      PropertyDialog* createPropertyDialog() override { return new LTISystemExporterPropertyDialog(this); }
  };

}

#endif
