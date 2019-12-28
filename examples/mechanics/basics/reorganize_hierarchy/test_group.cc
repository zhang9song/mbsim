#include "test_group.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/functions/kinematics/kinematics.h"
#include "openmbvcppinterface/ivbody.h"

using namespace MBSim;
using namespace fmatvec;
using namespace std;

TestGroup::TestGroup(const string &projectName) : Group(projectName) {

  double mStab = 0.2;
  double lStab = 0.3;
  double JStab = 1.0/12.0 * mStab * lStab * lStab; 
  double a1 = -0.15*lStab;
  double a2 = 0.15*lStab;

  Vec WrOK(3);
  Vec KrKS(3);
  SymMat Theta(3);

  stab1 = new RigidBody("Stab1");
  addObject(stab1);
  KrKS(0) = a1;
  SqrMat A(3);
  for(int i=0; i<3; i++)
    A(i,i) = 1;

  stab1->addFrame(new FixedRelativeFrame("Ref",-KrKS,A));
  stab1->setFrameForKinematics(stab1->getFrame("Ref"));
  stab1->setFrameOfReference(getFrame("I"));

  stab1->setqSize(1);
  stab1->setuSize(1);

  stab1->setMass(mStab);
  Theta(2,2) = JStab;
  stab1->setInertiaTensor(Theta);
  stab1->setRotation(new RotationAboutFixedAxis<VecV>(Vec("[0;0;1]")));

  std::shared_ptr<OpenMBV::IvBody> obj1=OpenMBV::ObjectFactory::create<OpenMBV::IvBody>();
  obj1->setIvFileName("objects/pendel1.wrl");
  obj1->setScaleFactor(0.1*0.3);
  obj1->setInitialRotation((vector<double>)(Vec("[0;0;1]")*M_PI/2.));
  stab1->setOpenMBVRigidBody(obj1);
  stab1->setOpenMBVFrameOfReference(stab1->getFrame("Ref"));

  stab2 = new RigidBody("Stab2");
  WrOK(0) = lStab/2;
  WrOK(2) = 0.006;
  stab1->addFrame(new FixedRelativeFrame("P",WrOK-KrKS,A));
  KrKS(0) = a2;
  stab2->addFrame(new FixedRelativeFrame("R",-KrKS,A));
  addObject(stab2);
  stab2->setqSize(1);
  stab2->setuSize(1);
  stab2->setFrameOfReference(stab1->getFrame("P"));
  stab2->setFrameForKinematics(stab2->getFrame("R"));
  stab2->setMass(mStab);
  Theta(2,2) = JStab;
  stab2->setInertiaTensor(Theta);
  stab2->setRotation(new RotationAboutFixedAxis<VecV>(Vec("[0;0;1]")));
  stab2->setGeneralizedInitialPosition(Vec("[-1.6]"));

  std::shared_ptr<OpenMBV::IvBody> obj2=OpenMBV::ObjectFactory::create<OpenMBV::IvBody>();
  obj2->setIvFileName("objects/pendel2.wrl");
  obj2->setScaleFactor(0.1*0.3);
  obj2->setInitialRotation((vector<double>)(Vec("[0;0;1]")*M_PI/2.));
  stab2->setOpenMBVRigidBody(obj2);
  stab2->setOpenMBVFrameOfReference(stab2->getFrame("R"));
}

