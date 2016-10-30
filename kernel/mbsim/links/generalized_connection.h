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

#ifndef _GENERALIZED_CONNECTION_H_
#define _GENERALIZED_CONNECTION_H_

#include "mbsim/links/rigid_body_link.h"
#include "mbsim/functions/function.h"

namespace MBSim {

  class GeneralizedConnection : public RigidBodyLink {
    protected:
      Function<double(double,double)> *func;
      RigidBody *body[2];
    public:
      GeneralizedConnection(const std::string &name="");
      ~GeneralizedConnection();

      void updateGeneralizedForces();
      bool isActive() const { return true; }
      bool gActiveChanged() { return false; }
      std::string getType() const { return "GeneralizedConnection"; }
      void init(InitStage stage);
      bool isSetValued() const { return func?false:true; }
      bool isSingleValued() const { return not(isSetValued()); }

      void setGeneralizedForceFunction(Function<double(double,double)> *func_) {
        func=func_;
        func->setParent(this);
        func->setName("GeneralizedForce");
      }

      void setRigidBodyFirstSide(RigidBody* body_) { body[0] = body_; }
      void setRigidBodySecondSide(RigidBody* body_) { body[1] = body_; }

      void initializeUsingXML(xercesc::DOMElement * element);

    private:
      std::string saved_body1, saved_body2;
  };

}

#endif
