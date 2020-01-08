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
#include "function_widget_factory.h"
#include "function_widgets.h"
#include "kinematic_functions_widgets.h"
#include "extended_widgets.h"
#include "variable_widgets.h"
#include "basic_widgets.h"
#include "rigid_body.h"
#include "function.h"
#include <iostream>

using namespace std;

namespace MBSimGUI {

  Function1ArgWidgetFactory::Function1ArgWidgetFactory(Element *element_, const QString &argName_, int argDim_, bool fixedArgDim_, int retDim_, bool fixedRetDim_, QWidget *parent_, int index_) : element(element_), argName(argName_), argDim(argDim_), fixedArgDim(fixedArgDim_), retDim(retDim_), fixedRetDim(fixedRetDim_), parent(parent_), index(index_) {
    name.emplace_back("Absolute value function");
    name.emplace_back("Bidirectional function");
    name.emplace_back("Composite function");
    name.emplace_back("Constant function");
    name.emplace_back("Continued function");
    name.emplace_back("Fourier function");
    name.emplace_back("Identity function");
    name.emplace_back("Linear function");
    name.emplace_back("Modulo function");
    name.emplace_back("Piecewise defined function");
    name.emplace_back("Piecewise polynom function");
    name.emplace_back("Polynom function");
    name.emplace_back("Quadratic function");
    name.emplace_back("Signal function");
    name.emplace_back("Signum function");
    name.emplace_back("Sinusoidal function");
    name.emplace_back("Symbolic function");
    name.emplace_back("Tabular function");
    name.emplace_back("Vector valued function");
    xmlName.push_back(MBSIM%"AbsoluteValueFunction");
    xmlName.push_back(MBSIM%"BidirectionalFunction");
    xmlName.push_back(MBSIM%"CompositeFunction");
    xmlName.push_back(MBSIM%"ConstantFunction");
    xmlName.push_back(MBSIM%"ContinuedFunction");
    xmlName.push_back(MBSIM%"FourierFunction");
    xmlName.push_back(MBSIM%"IdentityFunction");
    xmlName.push_back(MBSIM%"LinearFunction");
    xmlName.push_back(MBSIM%"ModuloFunction");
    xmlName.push_back(MBSIM%"PiecewiseDefinedFunction");
    xmlName.push_back(MBSIM%"PiecewisePolynomFunction");
    xmlName.push_back(MBSIM%"PolynomFunction");
    xmlName.push_back(MBSIM%"QuadraticFunction");
    xmlName.push_back(MBSIMCONTROL%"SignalFunction");
    xmlName.push_back(MBSIM%"SignumFunction");
    xmlName.push_back(MBSIM%"SinusoidalFunction");
    xmlName.push_back(MBSIM%"SymbolicFunction");
    xmlName.push_back(MBSIM%"TabularFunction");
    xmlName.push_back(MBSIM%"VectorValuedFunction");
  }

  QWidget* Function1ArgWidgetFactory::createWidget(int i) {
    if(i==0)
      return new AbsoluteValueFunctionWidget;
    if(i==1)
      return new BidirectionalFunctionWidget(element,argName,argDim,fixedArgDim,retDim,fixedRetDim,parent);
    if(i==2) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new CompositeFunctionWidget(new Function1ArgWidgetFactory(dummy,"x",1,false,retDim,fixedRetDim,parent,16),new Function2ArgWidgetFactory(dummy,QStringList("x")<<"y",vector<int>(2,1),false,retDim,fixedRetDim,parent),new Function1ArgWidgetFactory(dummy,argName,argDim,true,1,fixedRetDim,parent),16,0,3);
    }
    if(i==3)
      return new ConstantFunctionWidget;
    if(i==4) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new ContinuedFunctionWidget(new Function1ArgWidgetFactory(dummy,argName,argDim,fixedArgDim,retDim,fixedRetDim,parent),new Function1ArgWidgetFactory(dummy,argName,argDim,fixedArgDim,retDim,fixedRetDim,parent));
    }
    if(i==5)
      return new FourierFunctionWidget;
    if(i==6)
      return new IdentityFunctionWidget;
    if(i==7)
      return new LinearFunctionWidget;
    if(i==8)
      return new ModuloFunctionWidget;
    if(i==9) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new PiecewiseDefinedFunctionWidget(new Function1ArgWidgetFactory(dummy,argName,argDim,fixedArgDim,retDim,fixedRetDim,parent));
    }
    if(i==10)
      return new PiecewisePolynomFunctionWidget(retDim,fixedRetDim);
    if(i==11)
      return new PolynomFunctionWidget;
    if(i==12)
      return new QuadraticFunctionWidget;
    if(i==13)
      return new SignalFunctionWidget(element,parent);
    if(i==14)
      return new SignumFunctionWidget;
    if(i==15)
      return new SinusoidalFunctionWidget;
    if(i==16)
      return new SymbolicFunctionWidget(QStringList(argName),vector<int>(1,argDim),fixedArgDim,retDim,fixedRetDim);
    if(i==17)
      return new TabularFunctionWidget(retDim,fixedRetDim);
    if(i==18) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new VectorValuedFunctionWidget(new Function1ArgWidgetFactory(dummy,argName,argDim,fixedArgDim,0,true,parent),retDim,fixedRetDim);
    }
    return nullptr;
  }

  Function2ArgWidgetFactory::Function2ArgWidgetFactory(Element *element_, const QStringList &argName_, const vector<int> argDim_, bool fixedArgDim_, int retDim_, bool fixedRetDim_, QWidget *parent_) : element(element_), argName(argName_), argDim(argDim_), fixedArgDim(fixedArgDim_), retDim(retDim_), fixedRetDim(fixedRetDim_), parent(parent_) {
    name.emplace_back("Symbolic function");
    name.emplace_back("Two dimensional piecewise polynom function");
    name.emplace_back("Two dimensional tabular function");
    xmlName.push_back(MBSIM%"SymbolicFunction");
    xmlName.push_back(MBSIM%"TwoDimensionalPiecewisePolynomFunction");
    xmlName.push_back(MBSIM%"TwoDimensionalTabularFunction");
  }

  QWidget* Function2ArgWidgetFactory::createWidget(int i) {
    if(i==0)
      return new SymbolicFunctionWidget(argName,argDim,fixedArgDim,retDim,fixedRetDim);
    if(i==1)
      return new TwoDimensionalPiecewisePolynomFunctionWidget;
    if(i==2)
      return new TwoDimensionalTabularFunctionWidget;
    return nullptr;
  }

  StateDependentTranslationWidgetFactory::StateDependentTranslationWidgetFactory(Element *element_, QWidget *parent_) : element(element_), parent(parent_) {
    name.emplace_back("Composite function");
    name.emplace_back("Linear translation");
    name.emplace_back("Piecewise defined function");
    name.emplace_back("Piecewise polynom function");
    name.emplace_back("Symbolic function");
    name.emplace_back("Translation along x axis");
    name.emplace_back("Translation along y axis");
    name.emplace_back("Translation along z axis");
    name.emplace_back("Translation along axes x and y");
    name.emplace_back("Translation along axes y and z");
    name.emplace_back("Translation along axes x and z");
    name.emplace_back("Translation along axes x,y and z");
    name.emplace_back("Translation along fixed axis");
    xmlName.push_back(MBSIM%"CompositeFunction");
    xmlName.push_back(MBSIM%"LinearTranslation");
    xmlName.push_back(MBSIM%"PiecewiseDefinedFunction");
    xmlName.push_back(MBSIM%"PiecewisePolynomFunction");
    xmlName.push_back(MBSIM%"SymbolicFunction");
    xmlName.push_back(MBSIM%"TranslationAlongXAxis");
    xmlName.push_back(MBSIM%"TranslationAlongYAxis");
    xmlName.push_back(MBSIM%"TranslationAlongZAxis");
    xmlName.push_back(MBSIM%"TranslationAlongAxesXY");
    xmlName.push_back(MBSIM%"TranslationAlongAxesYZ");
    xmlName.push_back(MBSIM%"TranslationAlongAxesXZ");
    xmlName.push_back(MBSIM%"TranslationAlongAxesXYZ");
    xmlName.push_back(MBSIM%"TranslationAlongFixedAxis");
  }

  QWidget* StateDependentTranslationWidgetFactory::createWidget(int i) {
    if(i==0) {
      auto *dummy1 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy1->setParent(element);
      auto *dummy2 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy2->setParent(dummy1);
      return new CompositeFunctionWidget(new StateDependentTranslationWidgetFactory(element,parent),0,new Function1ArgWidgetFactory(dummy2,"q",1,false,1,true,parent));
    }
    if(i==1)
      return new LinearTranslationWidget(3,1);
    if(i==2) {
      auto *dummy1 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy1->setParent(element);
      auto *dummy2 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy2->setParent(dummy1);
      return new PiecewiseDefinedFunctionWidget(new Function1ArgWidgetFactory(dummy2,"q",1,false,3,true,parent));
    }
    if(i==3)
      return new PiecewisePolynomFunctionWidget(3,true);
    if(i==4)
      return new SymbolicFunctionWidget(QStringList("q"),vector<int>(1,1),false,3,true);
    if(i==5)
      return new TranslationAlongXAxisWidget;
    if(i==6)
      return new TranslationAlongYAxisWidget;
    if(i==7)
      return new TranslationAlongZAxisWidget;
    if(i==8)
      return new TranslationAlongAxesXYWidget;
    if(i==9)
      return new TranslationAlongAxesYZWidget;
    if(i==10)
      return new TranslationAlongAxesXZWidget;
    if(i==11)
      return new TranslationAlongAxesXYZWidget;
    if(i==12)
      return new TranslationAlongFixedAxisWidget;
    return nullptr;
  }

  TimeDependentTranslationWidgetFactory::TimeDependentTranslationWidgetFactory(Element *element_, QWidget *parent_) : element(element_), parent(parent_) {
    name.emplace_back("Composite function");
    name.emplace_back("Piecewise defined function");
    name.emplace_back("Piecewise polynom function");
    name.emplace_back("Symbolic function");
    name.emplace_back("Tabular function");
    name.emplace_back("Vector valued function");
    xmlName.push_back(MBSIM%"CompositeFunction");
    xmlName.push_back(MBSIM%"PiecewiseDefinedFunction");
    xmlName.push_back(MBSIM%"PiecewisePolynomFunction");
    xmlName.push_back(MBSIM%"SymbolicFunction");
    xmlName.push_back(MBSIM%"TabularFunction");
    xmlName.push_back(MBSIM%"VectorValuedFunction");
  }

  QWidget* TimeDependentTranslationWidgetFactory::createWidget(int i) {
    if(i==0) {
      auto *dummy1 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy1->setParent(element);
      auto *dummy2 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy2->setParent(dummy1);
      return new CompositeFunctionWidget(new StateDependentTranslationWidgetFactory(element,parent),0,new Function1ArgWidgetFactory(dummy2,"t",0,true,1,true,parent));
    }
    if(i==1) {
      auto *dummy1 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy1->setParent(element);
      auto *dummy2 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy2->setParent(dummy1);
      return new PiecewiseDefinedFunctionWidget(new Function1ArgWidgetFactory(dummy2,"t",0,true,3,true,parent));
    }
    if(i==2)
      return new PiecewisePolynomFunctionWidget(3,true);
    if(i==3)
      return new SymbolicFunctionWidget(QStringList("t"),vector<int>(1,0),true,3,true);
    if(i==4)
      return new TabularFunctionWidget(3,true);
    if(i==5) {
      auto *dummy1 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy1->setParent(element);
      auto *dummy2 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy2->setParent(dummy1);
      return new VectorValuedFunctionWidget(new Function1ArgWidgetFactory(dummy2,"t",0,true,3,true,parent),3,true);
    }
    return nullptr;
  }

  StateDependentRotationWidgetFactory::StateDependentRotationWidgetFactory(Element *element_, QWidget *parent_) : element(element_), parent(parent_) {
    name.emplace_back("Composite function");
    name.emplace_back("Rotation about x axis");
    name.emplace_back("Rotation about y axis");
    name.emplace_back("Rotation about z axis");
    name.emplace_back("Rotation about axes x and y");
    name.emplace_back("Rotation about axes y and z");
    name.emplace_back("Rotation about axes x and z");
    name.emplace_back("Rotation about axes x,y and z");
    name.emplace_back("Rotation about axes z,x and z");
    name.emplace_back("Rotation about axes z,y and x");
    name.emplace_back("Rotation about fixed axis");
    name.emplace_back("Symbolic function");
    xmlName.push_back(MBSIM%"CompositeFunction");
    xmlName.push_back(MBSIM%"RotationAboutXAxis");
    xmlName.push_back(MBSIM%"RotationAboutYAxis");
    xmlName.push_back(MBSIM%"RotationAboutZAxis");
    xmlName.push_back(MBSIM%"RotationAboutAxesXY");
    xmlName.push_back(MBSIM%"RotationAboutAxesYZ");
    xmlName.push_back(MBSIM%"RotationAboutAxesXZ");
    xmlName.push_back(MBSIM%"RotationAboutAxesXYZ");
    xmlName.push_back(MBSIM%"RotationAboutAxesZXZ");
    xmlName.push_back(MBSIM%"RotationAboutAxesZYX");
    xmlName.push_back(MBSIM%"RotationAboutFixedAxis");
    xmlName.push_back(MBSIM%"SymbolicFunction");
  }

  QWidget* StateDependentRotationWidgetFactory::createWidget(int i) {
    if(i==0) {
      auto *dummy1 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy1->setParent(element);
      auto *dummy2 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy2->setParent(dummy1);
      return new CompositeFunctionWidget(new StateDependentRotationWidgetFactory(element,parent),0,new Function1ArgWidgetFactory(dummy2,"q",1,false,1,true,parent));
    }
    if(i==1)
      return new RotationAboutXAxisWidget;
    if(i==2)
      return new RotationAboutYAxisWidget;
    if(i==3)
      return new RotationAboutZAxisWidget;
    if(i==4)
      return new RotationAboutAxesXYWidget;
    if(i==5)
      return new RotationAboutAxesYZWidget;
    if(i==6)
      return new RotationAboutAxesXZWidget;
    if(i==7)
      return new RotationAboutAxesXYZWidget;
    if(i==8)
      return new RotationAboutAxesZXZWidget;
    if(i==9)
      return new RotationAboutAxesZYXWidget;
    if(i==10)
      return new RotationAboutFixedAxisWidget;
    if(i==11)
      return new SymbolicFunctionWidget(QStringList("q"),vector<int>(1,3),false,3,true);
    return nullptr;
  }

  TimeDependentRotationWidgetFactory::TimeDependentRotationWidgetFactory(Element *element_, QWidget *parent_) : parent(parent_) {
    name.emplace_back("Composite function");
    name.emplace_back("Symbolic function");
    xmlName.push_back(MBSIM%"CompositeFunction");
    xmlName.push_back(MBSIM%"SymbolicFunction");
  }

  QWidget* TimeDependentRotationWidgetFactory::createWidget(int i) {
    if(i==0) {
      auto *dummy1 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy1->setParent(element);
      auto *dummy2 = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy2->setParent(dummy1);
      return new CompositeFunctionWidget(new StateDependentRotationWidgetFactory(element,parent),0,new Function1ArgWidgetFactory(dummy2,"t",0,true,1,true,parent));
    }
    if(i==1)
      return new SymbolicFunctionWidget(QStringList("t"),vector<int>(1,0),true,3,true);
    return nullptr;
  }

  TranslationWidgetFactory::TranslationWidgetFactory(Element *element_, const MBXMLUtils::NamespaceURI &uri, QWidget *parent_) : element(element_), parent(parent_) {
    name.emplace_back("General translation");
    name.emplace_back("State dependent translation");
    name.emplace_back("Time dependent translation");
    xmlName.push_back(uri%"generalTranslation");
    xmlName.push_back(uri%"stateDependentTranslation");
    xmlName.push_back(uri%"timeDependentTranslation");
  }

  QWidget* TranslationWidgetFactory::createWidget(int i) {
    if(i==0) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      vector<int> argDim(2);
      argDim[0] = 1;
      argDim[1] = 0;
      return new ChoiceWidget2(new Function2ArgWidgetFactory(dummy,QStringList("q")<<"t",argDim,false,3,true,parent));
    }
    if(i==1)
      return new ChoiceWidget2(new StateDependentTranslationWidgetFactory(element,parent),QBoxLayout::TopToBottom,0);
    if(i==2)
      return new ChoiceWidget2(new TimeDependentTranslationWidgetFactory(element,parent),QBoxLayout::TopToBottom,0);
    return nullptr;
  }

  RotationWidgetFactory::RotationWidgetFactory(Element *element_, const MBXMLUtils::NamespaceURI &uri, QWidget *parent_) : element(element_), parent(parent_) {
    name.emplace_back("State dependent rotation");
    name.emplace_back("Time dependent rotation");
    xmlName.push_back(uri%"stateDependentRotation");
    xmlName.push_back(uri%"timeDependentRotation");
  }

  QWidget* RotationWidgetFactory::createWidget(int i) {
    if(i==0)
      return new ChoiceWidget2(new StateDependentRotationWidgetFactory(element,parent),QBoxLayout::TopToBottom,0);
    if(i==1)
      return new ChoiceWidget2(new TimeDependentRotationWidgetFactory(element,parent),QBoxLayout::TopToBottom,0);
    return nullptr;
  }

  CompositeFunctionWidgetFactory::CompositeFunctionWidgetFactory(WidgetFactory *factory_) : factory(factory_) {
    name.emplace_back("1 inner function");
    name.emplace_back("2 inner functions");
    xmlName.push_back(MBSIM%"innerFunction");
    xmlName.push_back(MBSIM%"innerFunctions");
  }

  QWidget* CompositeFunctionWidgetFactory::createWidget(int i) {
    if(i==0) {
      return new ExtWidget("Inner function",new ChoiceWidget2(factory,QBoxLayout::TopToBottom,0),false,false,MBSIM%"innerFunction");
    }
    if(i==1)
      return new ExtWidget("Inner functions",new ListWidget(new ChoiceWidgetFactory(factory,1),"Function",2,0,true),false,false,MBSIM%"innerFunctions");
    return nullptr;
  }

  TabularFunctionWidgetFactory::TabularFunctionWidgetFactory(int retDim_, bool fixedRetDim_) : retDim(retDim_), fixedRetDim(fixedRetDim_) {
    name.emplace_back("x and y");
    name.emplace_back("xy");
    xmlName.push_back(MBSIM%"x");
    xmlName.push_back(MBSIM%"xy");
  }

  QWidget* TabularFunctionWidgetFactory::createWidget(int i) {
    if(i==0) {
      auto *widgetContainer = new ContainerWidget;
      widgetContainer->addWidget(new ExtWidget("x",new ChoiceWidget2(new VecSizeVarWidgetFactory(3,1,100,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,0),false,true),QBoxLayout::RightToLeft,5),false,false,MBSIM%"x"));
      if(fixedRetDim)
        widgetContainer->addWidget(new ExtWidget("y",new ChoiceWidget2(new MatWidgetFactory(getEye<QString>(3,retDim,"0","0"),vector<QStringList>(3,noUnitUnits()),std::vector<int>(3,0),true),QBoxLayout::RightToLeft,5),false,false,MBSIM%"y"));
      else
        widgetContainer->addWidget(new ExtWidget("y",new ChoiceWidget2(new MatColsVarWidgetFactory(3,retDim,vector<QStringList>(3,noUnitUnits()),std::vector<int>(3,0),true),QBoxLayout::RightToLeft,5),false,false,MBSIM%"y"));
      return widgetContainer;
    }
    if(i==1)
      return new ExtWidget("xy",new ChoiceWidget2(new MatRowsVarWidgetFactory(3,1+retDim,vector<QStringList>(3,noUnitUnits()),std::vector<int>(3,0),true),QBoxLayout::RightToLeft,5),false,false,MBSIM%"xy");
    return nullptr;
  }

  TwoDimensionalTabularFunctionWidgetFactory::TwoDimensionalTabularFunctionWidgetFactory() {
    name.emplace_back("x,y and z");
    name.emplace_back("xyz");
    xmlName.push_back(MBSIM%"x");
    xmlName.push_back(MBSIM%"xyz");
  }

  QWidget* TwoDimensionalTabularFunctionWidgetFactory::createWidget(int i) {
    if(i==0) {
      auto *widgetContainer = new ContainerWidget;
      widgetContainer->addWidget(new ExtWidget("x",new ChoiceWidget2(new VecSizeVarWidgetFactory(3,1,100,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,0),false,true),QBoxLayout::RightToLeft,5),false,false,MBSIM%"x"));
      widgetContainer->addWidget(new ExtWidget("y",new ChoiceWidget2(new VecSizeVarWidgetFactory(3,1,100,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,0),false,true),QBoxLayout::RightToLeft,5),false,false,MBSIM%"y"));
      widgetContainer->addWidget(new ExtWidget("z",new ChoiceWidget2(new MatWidgetFactory(3,3,vector<QStringList>(3,noUnitUnits()),vector<int>(3,0),true),QBoxLayout::RightToLeft,5),false,false,MBSIM%"z"));
      return widgetContainer;
    }
    if(i==1)
      return new ExtWidget("xyz",new ChoiceWidget2(new MatRowsColsVarWidgetFactory(3,3,true),QBoxLayout::RightToLeft,5),false,false,MBSIM%"xyz");
    return nullptr;
  }

  FourierFunctionWidgetFactory::FourierFunctionWidgetFactory() {
    name.emplace_back("a and b");
    name.emplace_back("ab");
    xmlName.push_back(MBSIM%"a");
    xmlName.push_back(MBSIM%"ab");
  }

  QWidget* FourierFunctionWidgetFactory::createWidget(int i) {
    if(i==0) {
      auto *widgetContainer = new ContainerWidget;
      widgetContainer->addWidget(new ExtWidget("a",new ChoiceWidget2(new VecSizeVarWidgetFactory(3),QBoxLayout::RightToLeft,5),false,false,MBSIM%"a"));
      widgetContainer->addWidget(new ExtWidget("b",new ChoiceWidget2(new VecWidgetFactory(3),QBoxLayout::RightToLeft,5),false,false,MBSIM%"b"));
      return widgetContainer;
    }
    if(i==1)
      return new ExtWidget("ab",new ChoiceWidget2(new MatRowsVarWidgetFactory(3,2),QBoxLayout::RightToLeft,5),false,false,MBSIM%"ab");
    return nullptr;
  }

  ConstraintWidgetFactory::ConstraintWidgetFactory(Element *element_, QWidget *parent_) : element(element_), parent(parent_) {
    name.emplace_back("General constraint function");
    name.emplace_back("State dependent constraint function");
    name.emplace_back("Time dependent constraint function");
    xmlName.push_back(MBSIM%"generalConstraintFunction");
    xmlName.push_back(MBSIM%"stateDependentConstraintFunction");
    xmlName.push_back(MBSIM%"timeDependentConstraintFunction");
  }

  QWidget* ConstraintWidgetFactory::createWidget(int i) {
    if(i==0) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      vector<int> argDim(2);
      argDim[0] = 1;
      argDim[1] = 0;
      return new ChoiceWidget2(new Function2ArgWidgetFactory(dummy,QStringList("q")<<"t",argDim,false,1,true,parent));
    }
    if(i==1) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new ChoiceWidget2(new Function1ArgWidgetFactory(dummy,"q",1,false,1,true,parent),QBoxLayout::TopToBottom,0);
    }
    if(i==2) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new ChoiceWidget2(new Function1ArgWidgetFactory(dummy,"t",0,true,1,true,parent),QBoxLayout::TopToBottom,0);
    }
    return nullptr;
  }

  SpringDamperWidgetFactory::SpringDamperWidgetFactory(Element *element_, bool varSize_, QWidget *parent_) : element(element_), varSize(varSize_), parent(parent_) {
    name.emplace_back("Linear elastic function");
    name.emplace_back("Linear spring damper force");
    name.emplace_back("Nonlinear spring damper force");
    name.emplace_back("Symbolic function");
    xmlName.push_back(MBSIM%"LinearElasticFunction");
    xmlName.push_back(MBSIM%"LinearSpringDamperForce");
    xmlName.push_back(MBSIM%"NonlinearSpringDamperForce");
    xmlName.push_back(MBSIM%"SymbolicFunction");
  }

  QWidget* SpringDamperWidgetFactory::createWidget(int i) {
    if(i==0)
      return new LinearElasticFunctionWidget(varSize);
    if(i==1)
      return new LinearSpringDamperForceWidget;
    if(i==2)
      return new NonlinearSpringDamperForceWidget(element,parent);
    if(i==3)
      return new SymbolicFunctionWidget(QStringList("g")<<"gd",vector<int>(1,0),true,0,true);
    return nullptr;
  }

  PlanarContourFunctionWidgetFactory::PlanarContourFunctionWidgetFactory(Element *element_, QWidget *parent_, const QString &var_) : element(element_), parent(parent_), var(var_) {
    name.emplace_back("Composite function");
    name.emplace_back("Continued function");
    name.emplace_back("Piecewise defined function");
    name.emplace_back("Piecewise polynom function");
    name.emplace_back("Polar contour function");
    name.emplace_back("Symbolic function");
    name.emplace_back("Vector valued function");
    xmlName.push_back(MBSIM%"CompositeFunction");
    xmlName.push_back(MBSIM%"ContinuedFunction");
    xmlName.push_back(MBSIM%"PiecewiseDefinedFunction");
    xmlName.push_back(MBSIM%"PiecewisePolynomFunction");
    xmlName.push_back(MBSIM%"PolarContourFunction");
    xmlName.push_back(MBSIM%"SymbolicFunction");
    xmlName.push_back(MBSIM%"VectorValuedFunction");
  }

  QWidget* PlanarContourFunctionWidgetFactory::createWidget(int i) {
    if(i==0) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new CompositeFunctionWidget(new PlanarContourFunctionWidgetFactory(element,parent,"phi"),0,new Function1ArgWidgetFactory(dummy,var,0,true,0,true,parent));
    }
    if(i==1) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new ContinuedFunctionWidget(new PlanarContourFunctionWidgetFactory(element,parent,var),new Function1ArgWidgetFactory(dummy,var,0,true,0,true,parent));
    }
    if(i==2) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new PiecewiseDefinedFunctionWidget(new PlanarContourFunctionWidgetFactory(element,parent,var));
    }
    if(i==3)
      return new PiecewisePolynomFunctionWidget(3,true);
    if(i==4)
      return new PolarContourFunctionWidget(element,parent);
    if(i==5)
      return new SymbolicFunctionWidget(QStringList(var),vector<int>(1,0),true,3,true);
    if(i==6) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new VectorValuedFunctionWidget(new Function1ArgWidgetFactory(dummy,var,0,true,0,true,parent),3,true);
    }
    return nullptr;
  }

  SpatialContourFunctionWidgetFactory::SpatialContourFunctionWidgetFactory(Element *element_, QWidget *parent_, const QString &var_) : element(element_), parent(parent_), var(var_) {
    name.emplace_back("Composite function");
    name.emplace_back("Continued function");
    name.emplace_back("Symbolic function");
    name.emplace_back("Vector valued function");
    xmlName.push_back(MBSIM%"CompositeFunction");
    xmlName.push_back(MBSIM%"ContinuedFunction");
    xmlName.push_back(MBSIM%"SymbolicFunction");
    xmlName.push_back(MBSIM%"VectorValuedFunction");
  }

  QWidget* SpatialContourFunctionWidgetFactory::createWidget(int i) {
    if(i==0) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new CompositeFunctionWidget(new SpatialContourFunctionWidgetFactory(element,parent,"phi"),0,new Function1ArgWidgetFactory(dummy,var,2,true,2,true,parent));
    }
    if(i==1) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new ContinuedFunctionWidget(new SpatialContourFunctionWidgetFactory(element,parent,var),new Function1ArgWidgetFactory(dummy,var,2,true,2,true,parent));
    }
    if(i==2)
      return new SymbolicFunctionWidget(QStringList(var),vector<int>(1,2),true,3,true);
    if(i==3) {
      auto *dummy = new Function; // Workaround for correct XML path. TODO: provide a consistent concept
      dummy->setParent(element);
      return new VectorValuedFunctionWidget(new Function1ArgWidgetFactory(dummy,var,0,true,0,true,parent),3,true);
    }
    return nullptr;
  }

  GravityFunctionWidgetFactory::GravityFunctionWidgetFactory() {
    name.emplace_back("Constant function");
    name.emplace_back("Gravity function");
    name.emplace_back("Symbolic function");
    xmlName.push_back(MBSIM%"ConstantFunction");
    xmlName.push_back(MBSIMPHYSICS%"GravityFunction");
    xmlName.push_back(MBSIM%"SymbolicFunction");
  }

  QWidget* GravityFunctionWidgetFactory::createWidget(int i) {
    if(i==0)
      return new ConstantFunctionWidget;
    if(i==1)
      return new GravityFunctionWidget;
    if(i==2)
      return new SymbolicFunctionWidget(QStringList("g"),vector<int>(1,0),true,0,true);
    return nullptr;
  }
}
