#include "system.h"
#include <boost/mpl/set/set30.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/order.hpp>
#include <mbsim/integrators/boost_odeint_integrator_predef.h>
#include <mbsim/integrators/daskr_integrator.h>
#include <mbsim/integrators/daspk_integrator.h>
#include <mbsim/integrators/dop853_integrator.h>
#include <mbsim/integrators/dopri5_integrator.h>
#include <mbsim/integrators/explicit_euler_integrator.h>
#include <mbsim/integrators/hets2_integrator.h>
#include <mbsim/integrators/implicit_euler_integrator.h>
#include <mbsim/integrators/lsoda_integrator.h>
#include <mbsim/integrators/lsodar_integrator.h>
#include <mbsim/integrators/lsode_integrator.h>
#include <mbsim/integrators/lsodi_integrator.h>
#include <mbsim/integrators/lsodkr_integrator.h>
#include <mbsim/integrators/odex_integrator.h>
#include <mbsim/integrators/radau5_integrator.h>
#include <mbsim/integrators/radau_integrator.h>
#include <mbsim/integrators/rksuite_integrator.h>
#include <mbsim/integrators/rodas_integrator.h>
#include <mbsim/integrators/seulex_integrator.h>
#include <mbsim/integrators/theta_time_stepping_integrator.h>
#include <mbsim/integrators/time_stepping_integrator.h>
#include <mbsim/integrators/time_stepping_ssc_integrator.h>

using namespace std;
using namespace MBSim;
using namespace MBSimIntegrator;

bool rigidContacts;

class Integrate {
  public:
    template<typename Int>
    void operator()(Int& integrator);
};

// commented out integrators do not work with this example
typedef boost::mpl::set15<
  BoostOdeintDOS_RKDOPRI5,
//  BoostOdeintDOS_BulirschStoer,
//  BoostOdeintDOS_Rosenbrock4,
//  DASKRIntegrator,
  DASPKIntegrator,
  DOP853Integrator,
  DOPRI5Integrator,
  ExplicitEulerIntegrator,
//  HETS2Integrator,
  ImplicitEulerIntegrator,
  LSODAIntegrator,
//  LSODARIntegrator,
  LSODEIntegrator,
  LSODIIntegrator,
  LSODKRIntegrator,
  ODEXIntegrator,
//  RADAU5Integrator,
//  RADAUIntegrator,
  RKSuiteIntegrator,
//  RODASIntegrator,
//  SEULEXIntegrator,
  ThetaTimeSteppingIntegrator,
  TimeSteppingIntegrator,
  TimeSteppingSSCIntegrator
> Integrators;

int main (int argc, char* argv[]) {
  boost::mpl::for_each<Integrators>(Integrate());    

  return 0;

}


template<typename Int>
void Integrate::operator()(Int& integrator) {
  string typeStr(typeid(Int).name());
  int order=boost::mpl::order<Integrators,Int>::type::value;

  char dummy[10000];
  double tEnd, dt, dtPlot;

  // Beginn input
  ifstream is("input.asc");
  is >> rigidContacts;
  is.getline(dummy,10000);
  is >> tEnd;
  is.getline(dummy,10000);
  is >> dtPlot;
  is.getline(dummy,10000);
  is >> dt;
  is.getline(dummy,10000);
  is.close();

  System *sys = new System("TS_"+to_string(order));

  sys->initialize();

  sys->setProjectionTolerance(1e-15);
  sys->setGeneralizedRelativePositionTolerance(1e-6);
  sys->setGeneralizedRelativeVelocityTolerance(1e-6);
  sys->setGeneralizedImpulseTolerance(1e-6);
  sys->setGeneralizedRelativeAccelerationTolerance(1e-8);
  sys->setGeneralizedForceTolerance(1e-8);

  cout << "integrate using "<<typeStr<<" = TS_"<<order<<endl;

  integrator.setEndTime(tEnd);
  integrator.setPlotStepSize(dtPlot);
  integrator.setSystem(sys);
  integrator.integrate();

  cout << endl;
  cout << "finished using "<<typeStr<<" = TS_"<<order<<endl;

  delete sys;
}
