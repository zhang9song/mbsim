#include <config.h>
#include "simulation_classes.h"
#include "mbsimControl/signal_.h"

using namespace fmatvec;
using namespace std;
using namespace MBSim;


namespace MBSimElectronics {

  void ElectronicObject::init(InitStage stage) {
    if(stage==preInit) {
      Object::init(stage);
      if(branch)
	dependency.push_back(branch);
    } 
    else if(stage==plotting) {
      updatePlotFeatures();

      if(getPlotFeature(plotRecursive)==enabled) {
	plotColumns.push_back("Charge");
	plotColumns.push_back("Current");

	Object::init(stage);
      }
    }
    else
      Object::init(stage);
  }

  void ElectronicObject::plot() {
    if(getPlotFeature(plotRecursive)==enabled) {
      plotVector.push_back(evalCharge());
      plotVector.push_back(evalCurrent());
      Object::plot();
    }
  }

  void ElectronicLink::calcgSize(int j) {
    gSize = 1;
  }

  void ElectronicLink::calcgdSize(int j) {
    gdSize = 1;
  }

  void ElectronicLink::calclaSize(int j) {
    laSize = 1;
  }

  void ElectronicLink::init(InitStage stage) {
    if(stage==unknownStage) {
      h[0].push_back(Vec(branch->getJacobian(0).cols()));
      r[0].push_back(Vec(branch->getJacobian(0).cols()));
      W[0].push_back(Mat(branch->getJacobian(0).cols(),laSize));
      V[0].push_back(Mat(branch->getJacobian(0).cols(),laSize));
      h[1].push_back(Vec(branch->getJacobian(1).cols()));
      r[1].push_back(Vec(branch->getJacobian(1).cols()));
      W[1].push_back(Mat(branch->getJacobian(1).cols(),laSize));
      V[1].push_back(Mat(branch->getJacobian(1).cols(),laSize));
  }
    else if(stage==plotting) {
      updatePlotFeatures();

      if(getPlotFeature(plotRecursive)==enabled) {
	plotColumns.push_back("Charge");
	plotColumns.push_back("Current");
	plotColumns.push_back("Voltage");

	Link::init(stage);
      }
    }
    else if(stage==preInit) {
      Link::init(stage);

      g.resize(1);
      gd.resize(1);
      gdn.resize(1);
      lambda.resize(1);
    }
    else
      Link::init(stage);
  }

  void ElectronicLink::updateg() {
    g(0) = evalCharge();
  }

  void ElectronicLink::updategd() {
    gd(0) = evalCurrent();
  }

  void ElectronicLink::plot() {
    if(getPlotFeature(plotRecursive)==enabled) {
      plotVector.push_back(evalCharge());
      plotVector.push_back(evalCurrent());
      plotVector.push_back(evalGeneralizedForce()(0));
      Link::plot();
    }
  }

  void ElectronicLink::updateW(int j) {
    W[j][0] += branch->getJacobian(j).T()*vz;
  }

  void ElectronicLink::updatehRef(const fmatvec::Vec &hParent, int j) {
    RangeV I = RangeV(branch->gethInd(j),branch->gethInd(j)+branch->getJacobian(j).cols()-1);
    h[j][0]>>hParent(I);
  } 

  void ElectronicLink::updaterRef(const Vec &rParent, int j) {
    RangeV I = RangeV(branch->gethInd(j),branch->gethInd(j)+branch->getJacobian(j).cols()-1);
    r[j][0]>>rParent(I);
  } 

  void ElectronicLink::updateWRef(const fmatvec::Mat& WParent, int j) {
    RangeV J = RangeV(laInd,laInd+laSize-1);
    RangeV I = RangeV(branch->gethInd(j),branch->gethInd(j)+branch->getJacobian(j).cols()-1);
    W[j][0].resize()>>WParent(I,J);
  }

  void ElectronicLink::updateVRef(const fmatvec::Mat& ref, int j) {
  }

  void ElectronicLink::updateh(int j) {
    h[j][0] += branch->getJacobian(j).T()*evalGeneralizedForce()(0)*vz;
  }

  void Mesh::init(InitStage stage) {
    if(stage==unknownStage) {
      T(0,0) = 1;
    }
    else if(stage==preInit) {
      Object::init(stage);
      if(precessor)
	dependency.push_back(precessor);
    } 
    else if(stage==plotting) {
      updatePlotFeatures();

      if(getPlotFeature(plotRecursive)==enabled) {
	Object::init(stage);
      }
    }
    else
      Object::init(stage);
  }

  void Branch::updateCharge() {
    Q(0) = 0;
    for(size_t i=0; i<mesh.size(); i++)
      Q(0) += J[0](0,mesh[i]->gethSize()-1)*mesh[i]->getq()(0);
    updQ = false;
  }

  void Branch::updateCurrent() {
    I(0) = 0;
    for(size_t i=0; i<mesh.size(); i++)
      I(0) += J[0](0,mesh[i]->gethSize()-1)*mesh[i]->getu()(0);
    updI = false;
  }

  void Branch::setvz(double vz_, Mesh* mesh_) {
    for(size_t i=0; i<mesh.size(); i++)
      if(mesh_ == mesh[i])
	vz[i] = vz_;
  }

  void Branch::init(InitStage stage) {
    if(stage==unknownStage) {
      if(J[0].cols() == 0) {
	J[0].resize(1,gethSize(0));
	for(size_t i=0; i<mesh.size(); i++)
	  J[0](0,mesh[i]->gethSize()-1) = vz[i]; 
      }
      if(J[1].cols() == 0) {
	J[1].resize(1,gethSize(1),INIT,1);
      }
    }
    else
      Object::init(stage);
  }

  void Branch::addConnectedBranch(Branch *branch) {
    connectedBranch.push_back(branch);
  }

  void Branch::buildTreeBranches(Branch* callingBranch, vector<Branch*> &treeBranch, unsigned int nmax) {
    if(getStartTerminal()->getFlag() != -1 || getEndTerminal()->getFlag() != -1) {
      treeBranch.push_back(this);
      getStartTerminal()->setFlag(-1);
      getEndTerminal()->setFlag(-1);
    }
    for(unsigned int i=0; i<connectedBranch.size(); i++)
      if(connectedBranch[i] != callingBranch) {
	if(connectedBranch[i]->getFlag()==0) {
	  connectedBranch[i]->setFlag(1);

	  if(treeBranch.size() < nmax)
	    connectedBranch[i]->buildTreeBranches(this,treeBranch,nmax);
	}
      }
  }

  void Branch::buildMeshes(Terminal *callingTerminal, Branch* callingBranch, Mesh* currentMesh, bool &foundMesh) {
    if(callingBranch == 0) {
      callingBranch = this;
    } 
    if(getFlag() == 3)
      setFlag(4);
    Terminal *sT;
    sT = callingTerminal == getEndTerminal() ? getStartTerminal() : getEndTerminal();
    for(int i=0; i<sT->getNumberOfConnectedBranches(); i++) {
      if(sT->getBranch(i) != this) { // nur vorwärts laufen
	if(sT->getBranch(i)->getFlag() == 3) // Branch ist TreeBranch
	  sT->getBranch(i)->buildMeshes(sT, callingBranch, currentMesh, foundMesh);
	else if(sT->getBranch(i) == callingBranch)
	  foundMesh = true;
	if(foundMesh) {
	  if(getFlag()==4)
	    setFlag(3);
	  connect(currentMesh);
	  return;
	}
      }
    }
  }

  Inductor::Inductor(const string &name) : ElectronicObject(name), L(1) {
    addTerminal("A");
    addTerminal("B");
    connectTerminal(terminal[0],terminal[1]);
  }

  void Inductor::updateM() {
    M += L*JTJ(branch->getJacobian(0));
  }

  Diode::Diode(const string &name) : ElectronicLink(name), sv(false) {
    addTerminal("A");
    addTerminal("B");
    connectTerminal(terminal[0],terminal[1]);
  }

  void Diode::updateGeneralizedForces() {
    if(isSetValued())
      lambda = evalla();
    else {
      double slope=1e4;
      double I = evalCurrent();
      lambda(0) = (I >= 0) ? 0 : -slope*I;
    }
    updla = false;
  }

  void Diode::checkImpactsForTermination() {

    const double *a = ds->evalGs()();
    const int *ia = ds->getGs().Ip();
    const int *ja = ds->getGs().Jp();
    const Vec &LaMBS = ds->getLa(false);
    const Vec &b = ds->evalbi();

    gdn(0) = b(laInd);
    for(int j=ia[laInd]; j<ia[laInd+1]; j++) {
      gdn(0) += a[j]*LaMBS(ja[j]);
    }

    if(gdn(0) >= -gdTol && fabs(La(0)) <= LaTol);
    else if(La(0) >= -LaTol && fabs(gdn(0)) <= gdTol);
    else {
      ds->setTermination(false);
      return;
    }
  }

  void Diode::solveImpactsGaussSeidel() {
    const double *a = ds->evalGs()();
    const int *ia = ds->getGs().Ip();
    const int *ja = ds->getGs().Jp();
    const Vec &LaMBS = ds->getLa(false);
    const Vec &b = ds->evalbi();

    gdn(0) = b(laInd);
    for(int j=ia[laInd]+1; j<ia[laInd+1]; j++)
      gdn(0) += a[j]*LaMBS(ja[j]);

    if(gdn(0) >= 0)
      La(0) = 0;
    else
      La(0) = -gdn(0)/a[ia[laInd]];
  }

  Switch::Switch(const string &name) : ElectronicLink(name), sv(false) {
    addTerminal("A");
    addTerminal("B");
    connectTerminal(terminal[0],terminal[1]);
  }

  void Switch::updateGeneralizedForces() {
    if(isSetValued())
      lambda = evalla();
    else {
      double gdLim = 0.01;
      double U0 = (*voltageSignal)(getTime())(0);
      double I = evalCurrent();
      if(fabs(I) < gdLim)
        lambda(0) = -U0*I/gdLim;
      else
        lambda(0) = I>0?-U0:U0;
    }
    updla = false;
  }

  void Switch::checkImpactsForTermination() {

    const double *a = ds->evalGs()();
    const int *ia = ds->getGs().Ip();
    const int *ja = ds->getGs().Jp();
    const Vec &LaMBS = ds->getLa(false);
    const Vec &b = ds->evalbi();

    double U0 = (*voltageSignal)(getTime())(0);

    gdn(0) = b(laInd);
    for(int j=ia[laInd]; j<ia[laInd+1]; j++)
      gdn(0) += a[j]*LaMBS(ja[j]);

    if(fabs(La(0) + gdn(0)/fabs(gdn(0))*fabs(U0)) <= LaTol)
      ;
    else if(fabs(La(0)) <= fabs(U0)+LaTol && fabs(gdn(0)) <= gdTol)
      ;
    else {
      ds->setTermination(false);
      return;
    }
  }

  void Switch::solveImpactsGaussSeidel() {
    const double *a = ds->evalGs()();
    const int *ia = ds->getGs().Ip();
    const int *ja = ds->getGs().Jp();
    const Vec &LaMBS = ds->getLa(false);
    const Vec &b = ds->evalbi();

    double U0 = (*voltageSignal)(getTime())(0);

    gdn(0) = b(laInd);
    for(int j=ia[laInd]+1; j<ia[laInd+1]; j++)
      gdn(0) += a[j]*LaMBS(ja[j]);

    double sdG = -gdn(0)/a[ia[laInd]];
    if(fabs(sdG)<=U0)
      La(0) = sdG;
    else
      La(0) = U0<=sdG ? U0 : -U0;
  }

  Resistor::Resistor(const string &name) : ElectronicLink(name), R(1) {
    addTerminal("A");
    addTerminal("B");
    connectTerminal(terminal[0],terminal[1]);
  }

  void Resistor::updateGeneralizedForces() {
    lambda(0) = -R*evalCurrent();
    updla = false;
  }

  Capacitor::Capacitor(const string &name) : ElectronicLink(name), C(1) {
    addTerminal("A");
    addTerminal("B");
    connectTerminal(terminal[0],terminal[1]);
  }

  void Capacitor::updateGeneralizedForces() {
    lambda(0) = -evalCharge()/C;
    updla = false;
  }

  VoltageSource::VoltageSource(const string &name) : ElectronicLink(name) {
    addTerminal("A");
    addTerminal("B");
    connectTerminal(terminal[0],terminal[1]);
  }

  void VoltageSource::updateGeneralizedForces() {
    lambda(0) = (*voltageSignal)(getTime())(0);
    updla = false;
  }

}


