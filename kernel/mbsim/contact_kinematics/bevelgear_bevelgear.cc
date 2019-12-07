/* Copyright (C) 2004-2019 MBSim Development Team
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
#include "bevelgear_bevelgear.h"
#include "mbsim/frames/contour_frame.h"
#include "mbsim/contours/bevel_gear.h"
#include <mbsim/utils/rotarymatrices.h>

using namespace fmatvec;
using namespace std;

namespace MBSim {

  void ContactKinematicsBevelGearBevelGear::assignContours(const vector<Contour*> &contour) {
    gear[0] = static_cast<BevelGear*>(contour[0]);
    gear[1] = static_cast<BevelGear*>(contour[1]);
    igear[0] = 0;
    igear[1] = 1;
    beta[0] = gear[0]->getHelixAngle();
    beta[1] = gear[1]->getHelixAngle();
    m = gear[0]->getModule()/cos(beta[0]);
    al0 = gear[0]->getPressureAngle();
    z[0] = gear[0]->getNumberOfTeeth();
    z[1] = gear[1]->getNumberOfTeeth();
    delh2 = (M_PI/2-gear[1]->getBacklash()/m)/z[1];
    delh1 = (M_PI/2-gear[0]->getBacklash()/m)/z[0];
    etamax1[0][0] = gear[0]->getEtaMax(0,0); // TODO: determine correct etamax
    etamax1[1][0] = gear[0]->getEtaMax(0,0); // TODO: determine correct etamax
    etamax1[0][1] = gear[0]->getEtaMax(0,0); // TODO: determine correct etamax
    etamax1[1][1] = gear[0]->getEtaMax(0,0); // TODO: determine correct etamax
    etamax2[0][0] = gear[1]->getEtaMax(0,0); // TODO: determine correct etamax
    etamax2[1][0] = gear[1]->getEtaMax(0,0); // TODO: determine correct etamax
    etamax2[0][1] = gear[1]->getEtaMax(0,0); // TODO: determine correct etamax
    etamax2[1][1] = gear[1]->getEtaMax(0,0); // TODO: determine correct etamax
  }

  void ContactKinematicsBevelGearBevelGear::updateg(SingleContact &contact, int ii) {
    contact.getGeneralizedRelativePosition(false)(0) = 1e10;
    Vec3 ez1 = gear[0]->getFrame()->evalOrientation().T()*(-gear[1]->getFrame()->evalOrientation().col(2));
    Vec3 ez2 = gear[1]->getFrame()->getOrientation().T()*(-gear[0]->getFrame()->getOrientation().col(2));
    double phi1 = (ez1(0)>=0?1:-1)*acos(ez1(1)/sqrt(pow(ez1(0),2)+pow(ez1(1),2))); 
    double phi2 = (ez2(0)>=0?1:-1)*acos(ez2(1)/sqrt(pow(ez2(0),2)+pow(ez2(1),2))); 
    if(nrm2(gear[1]->getFrame()->evalPosition()-gear[0]->getFrame()->evalPosition()+m/2*(z[0]/tan(gear[0]->getPitchAngle())*gear[0]->getFrame()->getOrientation().col(2)-z[1]/tan(gear[1]->getPitchAngle())*gear[1]->getFrame()->getOrientation().col(2)))>1e-8)
      msg(Warn)<<"Large devitation detected at t="<<gear[1]->getTime()<<"\nContact kinematics may be wrong!" <<endl;

    for(int i=0; i<2; i++) {
      int signi = i?-1:1;
      Vec3 rOP[2];
      vector<int> v[2];
      if(maxNumContacts==1) {
        v[1].push_back((round(-(phi2 - signi*delh2)/(2*M_PI/z[1]))));
        double phi1corr = (phi2 + v[1][0]*2*M_PI/z[1] - signi*delh2)*z[1]/z[0];
        v[0].push_back((round(-(phi1 - signi*delh1 + phi1corr)/(2*M_PI/z[0]))));
      }
      else {
        int kmax = floor(-(-etamax1[1][i] + (phi1 - signi*delh1))/(2*M_PI/z[0]));
        int kmin = ceil(-(etamax1[1][not i] + (phi1 - signi*delh1))/(2*M_PI/z[0]));
        for(int k_=kmin; k_<=kmax; k_++)
          v[0].push_back(k_);
        kmax = floor(-(-etamax2[1][i] + (phi2 - signi*delh2))/(2*M_PI/z[0]));
        kmin = ceil(-(etamax2[1][not i] + (phi2 - signi*delh2))/(2*M_PI/z[0]));
        for(int k_=kmin; k_<=kmax; k_++)
          v[1].push_back(k_);
      }

      double k[2];
      for (auto & i0 : v[0]) {
        for (auto & i1 : v[1]) {
          k[0] = i0;
          k[1] = i1;
          if(ii==0 or not(k[0]==ksave[0][0] and k[1]==ksave[0][1])) {
            Vec2 zeta1(NONINIT), zeta2(NONINIT);
            zeta1(0) = -(phi1+k[0]*2*M_PI/z[0]-signi*delh1);
            double s = 0;
            if(zeta1(0)>etamax1[0][not i])
              s = (beta[0]>=0?-1:1)*max(s,gear[0]->getWidth()/2/(etamax1[1][not i]-etamax1[0][not i])*(zeta1(0)-etamax1[0][not i]));
            else if(zeta1(0)<-etamax1[0][i])
              s = (beta[0]>=0?1:-1)*max(s,gear[0]->getWidth()/2/(-etamax1[1][i]+etamax1[0][i])*(zeta1(0)+etamax1[0][i]));
            zeta2(0) = -(phi2+k[1]*2*M_PI/z[1]-signi*delh1);
            if(zeta2(0)>etamax2[0][not i])
              s = (beta[1]>=0?-1:1)*max(fabs(s),gear[0]->getWidth()/2/(etamax2[1][not i]-etamax2[0][not i])*(zeta2(0)-etamax2[0][not i]));
            else if(zeta2(0)<-etamax2[0][i])
              s = (beta[1]>=0?1:-1)*max(fabs(s),gear[0]->getWidth()/2/(-etamax2[1][i]+etamax2[0][i])*(zeta2(0)+etamax2[0][i]));
            double phi2q = -sin(gear[0]->getPitchAngle())*zeta1(0);
            zeta1(1) = (s*cos(phi2q-beta[0])+m*z[0]/sin(gear[0]->getPitchAngle())/2*sin(phi2q)*pow(sin(al0),2)*sin(beta[0]))/(-sin(phi2q-beta[0])*pow(sin(al0),2)*sin(beta[0])+cos(phi2q-beta[0])*cos(beta[0]));
            gear[0]->setFlank(signi);
            gear[0]->setTooth(k[0]);
            rOP[0] = gear[0]->evalPosition(zeta1);
            phi2q = -sin(gear[1]->getPitchAngle())*zeta2(0);
            zeta2(1) = (s*cos(phi2q-beta[1])+m*z[1]/sin(gear[1]->getPitchAngle())/2*sin(phi2q)*pow(sin(al0),2)*sin(beta[1]))/(-sin(phi2q-beta[1])*pow(sin(al0),2)*sin(beta[1])+cos(phi2q-beta[1])*cos(beta[1]));
            gear[1]->setFlank(signi);
            gear[1]->setTooth(k[1]);
            rOP[1] = gear[1]->evalPosition(zeta2);

            Vec n1 = gear[0]->evalWn(zeta1);

            double g = n1.T()*(rOP[1]-rOP[0]);
            if(g>-0.5*M_PI*m and g<contact.getGeneralizedRelativePosition(false)(0)) {
              ksave[ii][0] = k[0];
              ksave[ii][1] = k[1];
              signisave[ii] = signi;

              contact.getContourFrame(igear[0])->setZeta(zeta1);
              contact.getContourFrame(igear[0])->setPosition(rOP[0]);
              contact.getContourFrame(igear[0])->getOrientation(false).set(0,n1);
              contact.getContourFrame(igear[0])->getOrientation(false).set(1,gear[0]->evalWu(zeta1));
              contact.getContourFrame(igear[0])->getOrientation(false).set(2,crossProduct(contact.getContourFrame(igear[0])->getOrientation(false).col(0),contact.getContourFrame(igear[0])->getOrientation(false).col(1)));

              contact.getContourFrame(igear[1])->setZeta(zeta2);
              contact.getContourFrame(igear[1])->setPosition(rOP[1]);
              contact.getContourFrame(igear[1])->getOrientation(false).set(0,gear[1]->evalWn(zeta1));
              contact.getContourFrame(igear[1])->getOrientation(false).set(1,gear[1]->evalWu(zeta2));
              contact.getContourFrame(igear[1])->getOrientation(false).set(2,crossProduct(contact.getContourFrame(igear[1])->getOrientation(false).col(0),contact.getContourFrame(igear[1])->getOrientation(false).col(1)));

              contact.getGeneralizedRelativePosition(false)(0) = g;
            }
          }
        }
      }
    }
  }

  void ContactKinematicsBevelGearBevelGear::updatewb(SingleContact &contact, int ii) {
    const Vec3 n1 = contact.getContourFrame(igear[0])->evalOrientation().col(0);
    const Vec3 u1 = contact.getContourFrame(igear[0])->evalOrientation().col(1);
    const Vec3 u2 = contact.getContourFrame(igear[1])->evalOrientation().col(1);
    const Vec3 vC1 = contact.getContourFrame(igear[0])->evalVelocity();
    const Vec3 vC2 = contact.getContourFrame(igear[1])->evalVelocity();
    gear[0]->setFlank(signisave[ii]);
    gear[0]->setTooth(ksave[ii][0]);
    gear[1]->setFlank(signisave[ii]);
    gear[1]->setTooth(ksave[ii][1]);
    Vec3 R1 = gear[0]->evalWs(contact.getContourFrame(igear[0])->getZeta());
    Vec3 R2 = gear[1]->evalWs(contact.getContourFrame(igear[1])->getZeta());
    Vec3 N1 = gear[0]->evalParDer1Wn(contact.getContourFrame(igear[0])->getZeta());
    Vec3 U2 = gear[1]->evalParDer1Wu(contact.getContourFrame(igear[1])->getZeta());
    const Vec3 parnPart1 = crossProduct(gear[0]->getFrame()->evalAngularVelocity(),n1);
    const Vec3 paruPart2 = crossProduct(gear[1]->getFrame()->evalAngularVelocity(),u2);
    const Vec3 parWvCParZeta1 = crossProduct(gear[0]->getFrame()->evalAngularVelocity(),R1);
    const Vec3 parWvCParZeta2 = crossProduct(gear[1]->getFrame()->evalAngularVelocity(),R2);

    SqrMat A(2,NONINIT);
    A(0,0)=-u1.T()*R1;
    A(0,1)=u1.T()*R2;
    A(1,0)=u2.T()*N1;
    A(1,1)=n1.T()*U2;

    Vec b(2,NONINIT);
    b(0)=-u1.T()*(vC2-vC1);
    b(1)=-u2.T()*parnPart1-n1.T()*paruPart2;

    Vec zetad = slvLU(A,b);

    if(contact.isNormalForceLawSetValued())
      contact.getwb(false)(0) += (N1*zetad(0)+parnPart1).T()*(vC2-vC1)+n1.T()*(parWvCParZeta2*zetad(1)-parWvCParZeta1*zetad(0));
    if(contact.isTangentialForceLawSetValuedAndActive())
      throw runtime_error("Tangential force law must be single valued for gear to gear contacts");
  }

}
