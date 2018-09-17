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
#include "element_property_dialog.h"
#include "basic_widgets.h"
#include "variable_widgets.h"
#include "special_widgets.h"
#include "kinematics_widgets.h"
#include "kinetics_widgets.h"
#include "function_widgets.h"
#include "kinematic_functions_widgets.h"
#include "ombv_widgets.h"
#include "extended_widgets.h"
#include "dynamic_system_solver.h"
#include "frame.h"
#include "contour.h"
#include "rigid_body.h"
#include "flexible_body_ffr.h"
#include "constraint.h"
#include "kinetic_excitation.h"
#include "spring_damper.h"
#include "joint.h"
#include "contact.h"
#include "observer.h"
#include "parameter.h"
#include "integrator.h"
#include "sensor.h"
#include "function_widget_factory.h"
#include "friction.h"
#include "gear.h"
#include "connection.h"
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QMessageBox>
#include <utility>
#include <mbxmlutilshelper/getinstallpath.h>
#include "mainwindow.h"
#include "import.h"
#include "import_widgets.h"
#include "dialogs.h"

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  extern MainWindow *mw;

  class ConnectRigidBodiesWidgetFactory : public WidgetFactory {
    public:
      ConnectRigidBodiesWidgetFactory(Element *parent_);
      QWidget* createWidget(int i=0) override;
      QString getName(int i=0) const override { return name[i]; }
      int getSize() const override { return name.size(); }
    protected:
      Element *parent;
      std::vector<QString> name;
  };

  ConnectRigidBodiesWidgetFactory::ConnectRigidBodiesWidgetFactory(Element *parent_) : parent(parent_) {
    name.emplace_back("1 rigid body");
    name.emplace_back("2 rigid bodies");
  }

  QWidget* ConnectRigidBodiesWidgetFactory::createWidget(int i) {
    return new ConnectElementsWidget<RigidBody>(i+1,parent);
  }

  ElementPropertyDialog::ElementPropertyDialog(Element *element, QWidget *parent, const Qt::WindowFlags& f) : EmbedItemPropertyDialog(element,parent,f) {
    addTab("General");
    name = new ExtWidget("Name",new TextWidget(item->getName()));
    name->setToolTip("Set the name of the element");
    addToTab("General", name);
    addTab("Plot");
    plotFeature = new ExtWidget("Plot features",new PlotFeatureWidget(getElement()->getPlotFeatureType()));
    addToTab("Plot", plotFeature);
  }

  Element* ElementPropertyDialog::getElement() const {
    return static_cast<Element*>(item);
  }

  DOMElement* ElementPropertyDialog::initializeUsingXML(DOMElement *parent) {
    static_cast<TextWidget*>(name->getWidget())->setText(item->getName());
    plotFeature->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* ElementPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    item->removeXMLElements();
    E(item->getXMLElement())->setAttribute("name",static_cast<TextWidget*>(name->getWidget())->getText().toStdString());
    plotFeature->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  void ElementPropertyDialog::showXMLHelp() {
    // generate url for current element
    string url=(MBXMLUtils::getInstallPath()/"share"/"mbxmlutils"/"doc").string();
    string ns=getElement()->getNameSpace().getNamespaceURI();
    replace(ns.begin(), ns.end(), ':', '_');
    replace(ns.begin(), ns.end(), '.', '_');
    replace(ns.begin(), ns.end(), '/', '_');
    url+="/"+ns+"/index.html#"+getElement()->getType().toStdString();
    // open in XML help dialog
    mw->xmlHelp(QString::fromStdString(url));
  }

  void ElementPropertyDialog::setName(const QString &str) {
    static_cast<TextWidget*>(name->getWidget())->setText(str);
  }

  void ElementPropertyDialog::setReadOnly(bool readOnly) {
    static_cast<TextWidget*>(name->getWidget())->setReadOnly(readOnly);
  }

  UnknownElementPropertyDialog::UnknownElementPropertyDialog(Element *element, QWidget *parent, const Qt::WindowFlags& f) : ElementPropertyDialog(element,parent,f) {
    name->setDisabled(true);
    plotFeature->setDisabled(true);
    editor = new ExtWidget("XML Editor",new XMLEditorWidget);
    addToTab("General", editor);
  }

  DOMElement* UnknownElementPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ElementPropertyDialog::initializeUsingXML(item->getXMLElement());
    editor->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* UnknownElementPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    item->setXMLElement(editor->writeXMLFile(item->getXMLElement(),ref));
    return nullptr;
  }

  FramePropertyDialog::FramePropertyDialog(Frame *frame, QWidget *parent, const Qt::WindowFlags& f) : ElementPropertyDialog(frame,parent,f) {
    addTab("Visualisation",1);
    visu = new ExtWidget("Enable openMBV",new FrameMBSOMBVWidget,true,true,MBSIM%"enableOpenMBV");
    visu->setToolTip("Set the visualisation parameters for the frame");
    addToTab("Visualisation", visu);
  }

  DOMElement* FramePropertyDialog::initializeUsingXML(DOMElement *parent) {
    ElementPropertyDialog::initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FramePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ElementPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    visu->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  InternalFramePropertyDialog::InternalFramePropertyDialog(InternalFrame *frame, QWidget *parent, const Qt::WindowFlags& f) : ElementPropertyDialog(frame,parent,f) {
    if(frame->getParent()->getEmbeded()) {
      buttonBox->button(QDialogButtonBox::Apply)->setDisabled(true);
      buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
    }
    addTab("Visualisation",1);
    visu = new ExtWidget("Enable openMBV",new FrameMBSOMBVWidget,true,true,frame->getXMLFrameName());
    visu->setToolTip("Set the visualisation parameters for the frame");
    addToTab("Visualisation", visu);
    setReadOnly(true);
    setName(frame->getName());
  }

  DOMElement* InternalFramePropertyDialog::initializeUsingXML(DOMElement *parent) {
    visu->initializeUsingXML(getElement()->getParent()->getXMLElement());
    static_cast<PlotFeatureWidget*>(plotFeature->getWidget())->initializeUsingXML2(getElement()->getParent()->getXMLElement());
    return parent;
  }

  DOMElement* InternalFramePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    item->removeXMLElements();
    DOMElement *ele = getElement()->getParent()->getXMLFrame();
    visu->writeXMLFile(getElement()->getParent()->getXMLElement(),ele);
    static_cast<PlotFeatureWidget*>(plotFeature->getWidget())->writeXMLFile2(getElement()->getParent()->getXMLElement(),ele);
    return nullptr;
  }

  FixedRelativeFramePropertyDialog::FixedRelativeFramePropertyDialog(FixedRelativeFrame *frame, QWidget *parent, const Qt::WindowFlags& f) : FramePropertyDialog(frame,parent,f) {
    addTab("Kinematics",1);

    refFrame = new ExtWidget("Frame of reference",new ParentFrameOfReferenceWidget(frame,frame),true,false,MBSIM%"frameOfReference");
    addToTab("Kinematics", refFrame);

    position = new ExtWidget("Relative position",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(3,lengthUnits()),vector<int>(3,4)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"relativePosition");
    addToTab("Kinematics", position);

    orientation = new ExtWidget("Relative orientation",new ChoiceWidget2(new RotMatWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIM%"relativeOrientation");
    addToTab("Kinematics", orientation);
  }

  DOMElement* FixedRelativeFramePropertyDialog::initializeUsingXML(DOMElement *parent) {
    FramePropertyDialog::initializeUsingXML(item->getXMLElement());
    refFrame->initializeUsingXML(item->getXMLElement());
    position->initializeUsingXML(item->getXMLElement());
    orientation->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FixedRelativeFramePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FramePropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    refFrame->writeXMLFile(item->getXMLElement(),nullptr);
    position->writeXMLFile(item->getXMLElement(),nullptr);
    orientation->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  NodeFramePropertyDialog::NodeFramePropertyDialog(NodeFrame *frame, QWidget *parent, const Qt::WindowFlags& f) : FramePropertyDialog(frame,parent,f) {

    nodeNumber = new ExtWidget("Node number",new ChoiceWidget2(new ScalarWidgetFactory("1"),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"nodeNumber");
    addToTab("General", nodeNumber);
  }

  DOMElement* NodeFramePropertyDialog::initializeUsingXML(DOMElement *parent) {
    FramePropertyDialog::initializeUsingXML(item->getXMLElement());
    nodeNumber->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* NodeFramePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FramePropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    nodeNumber->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  ContourPropertyDialog::ContourPropertyDialog(Contour *contour, QWidget * parent, const Qt::WindowFlags& f) : ElementPropertyDialog(contour,parent,f) {
    thickness = new ExtWidget("Thickness",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"thickness");
    addToTab("General", thickness);
  }

  DOMElement* ContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ElementPropertyDialog::initializeUsingXML(item->getXMLElement());
    thickness->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* ContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ElementPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    thickness->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  RigidContourPropertyDialog::RigidContourPropertyDialog(RigidContour *contour, QWidget * parent, const Qt::WindowFlags& f) : ContourPropertyDialog(contour,parent,f) {
    refFrame = new ExtWidget("Frame of reference",new ParentFrameOfReferenceWidget(contour,nullptr),true,false,MBSIM%"frameOfReference");
    addToTab("General", refFrame);
  }

  DOMElement* RigidContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    refFrame->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* RigidContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    refFrame->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  PointPropertyDialog::PointPropertyDialog(Point *point, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(point,parent,f) {
    addTab("Visualisation",1);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* PointPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* PointPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  LinePropertyDialog::LinePropertyDialog(Line *line, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(line,parent,f) {
    addTab("Visualisation",1);

    visu = new ExtWidget("Enable openMBV",new LineMBSOMBVWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* LinePropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* LinePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  PlanePropertyDialog::PlanePropertyDialog(Plane *plane, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(plane,parent,f) {
    addTab("Visualisation",1);

    visu = new ExtWidget("Enable openMBV",new PlaneMBSOMBVWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* PlanePropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* PlanePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  SpherePropertyDialog::SpherePropertyDialog(Sphere *sphere, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(sphere,parent,f) {
    addTab("Visualisation",1);

    radius = new ExtWidget("Radius",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"radius");
    addToTab("General", radius);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* SpherePropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    radius->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* SpherePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    radius->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  CirclePropertyDialog::CirclePropertyDialog(Circle *circle, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(circle,parent,f) {
    addTab("Visualisation",1);

    radius = new ExtWidget("Radius",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"radius");
    addToTab("General", radius);
    solid = new ExtWidget("Solid",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"solid");
    addToTab("General", solid);
    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* CirclePropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    radius->initializeUsingXML(item->getXMLElement());
    solid->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* CirclePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    radius->writeXMLFile(item->getXMLElement(),nullptr);
    solid->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  CuboidPropertyDialog::CuboidPropertyDialog(Cuboid *circle, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(circle,parent,f) {
    addTab("Visualisation",1);

    length = new ExtWidget("Length",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"length");
    addToTab("General", length);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* CuboidPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    length->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* CuboidPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    length->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  LineSegmentPropertyDialog::LineSegmentPropertyDialog(LineSegment *line, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(line,parent,f) {
    addTab("Visualisation",1);

    length = new ExtWidget("Length",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"length");
    addToTab("General", length);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* LineSegmentPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    length->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* LineSegmentPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    length->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  PlanarContourPropertyDialog::PlanarContourPropertyDialog(PlanarContour *contour, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    nodes = new ExtWidget("Nodes",new ChoiceWidget2(new VecSizeVarWidgetFactory(2),QBoxLayout::RightToLeft,5),true,false,MBSIM%"nodes");
    addToTab("General", nodes);

    contourFunction = new ExtWidget("Contour function",new ChoiceWidget2(new PlanarContourFunctionWidgetFactory(contour),QBoxLayout::TopToBottom,0),false,false,MBSIM%"contourFunction");
    addToTab("General", contourFunction);

    open = new ExtWidget("Open",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"open");
    addToTab("General", open);

    visu = new ExtWidget("Enable openMBV",new PlanarContourMBSOMBVWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* PlanarContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    nodes->initializeUsingXML(item->getXMLElement());
    contourFunction->initializeUsingXML(item->getXMLElement());
    open->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* PlanarContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    nodes->writeXMLFile(item->getXMLElement(),nullptr);
    contourFunction->writeXMLFile(item->getXMLElement(),nullptr);
    open->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  PlanarNurbsContourPropertyDialog::PlanarNurbsContourPropertyDialog(RigidContour *contour, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    vector<QString> list;
    list.emplace_back("\"equallySpaced\"");
    list.emplace_back("\"chordLength\"");
    list.emplace_back("\"none\"");
    interpolation = new ExtWidget("Interpolation",new TextChoiceWidget(list,1,true),true,false,MBSIM%"interpolation");
    addToTab("General", interpolation);

    controlPoints = new ExtWidget("Control points",new ChoiceWidget2(new MatRowsColsVarWidgetFactory(0,0),QBoxLayout::RightToLeft,5),false,false,MBSIM%"controlPoints");
    addToTab("General", controlPoints);

    numberOfControlPoints = new ExtWidget("Number of control points",new ChoiceWidget2(new ScalarWidgetFactory(0),QBoxLayout::RightToLeft,5),false,false,MBSIM%"numberOfControlPoints");
    addToTab("General", numberOfControlPoints);

    knotVector = new ExtWidget("Knot vector",new ChoiceWidget2(new VecSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIM%"knotVector");
    addToTab("General", knotVector);

    degree = new ExtWidget("Degree",new ChoiceWidget2(new ScalarWidgetFactory("3"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"degree");
    addToTab("General", degree);

    open = new ExtWidget("Open",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"open");
    addToTab("General", open);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* PlanarNurbsContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    interpolation->initializeUsingXML(item->getXMLElement());
    controlPoints->initializeUsingXML(item->getXMLElement());
    numberOfControlPoints->initializeUsingXML(item->getXMLElement());
    knotVector->initializeUsingXML(item->getXMLElement());
    degree->initializeUsingXML(item->getXMLElement());
    open->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* PlanarNurbsContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    interpolation->writeXMLFile(item->getXMLElement(),nullptr);
    controlPoints->writeXMLFile(item->getXMLElement(),nullptr);
    numberOfControlPoints->writeXMLFile(item->getXMLElement(),nullptr);
    knotVector->writeXMLFile(item->getXMLElement(),nullptr);
    degree->writeXMLFile(item->getXMLElement(),nullptr);
    open->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  SpatialContourPropertyDialog::SpatialContourPropertyDialog(SpatialContour *contour, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    etaNodes = new ExtWidget("Eta nodes",new ChoiceWidget2(new VecSizeVarWidgetFactory(2),QBoxLayout::RightToLeft,5),true,false,MBSIM%"etaNodes");
    addToTab("General", etaNodes);

    xiNodes = new ExtWidget("Xi nodes",new ChoiceWidget2(new VecSizeVarWidgetFactory(2),QBoxLayout::RightToLeft,5),true,false,MBSIM%"xiNodes");
    addToTab("General", xiNodes);

    contourFunction = new ExtWidget("Contour function",new ChoiceWidget2(new SpatialContourFunctionWidgetFactory(contour),QBoxLayout::TopToBottom,0),false,false,MBSIM%"contourFunction");
    addToTab("General", contourFunction);

    openEta = new ExtWidget("Open eta",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"openEta");
    addToTab("General", openEta);

    openXi = new ExtWidget("Open xi",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"openXi");
    addToTab("General", openXi);

    visu = new ExtWidget("Enable openMBV",new SpatialContourMBSOMBVWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* SpatialContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    etaNodes->initializeUsingXML(item->getXMLElement());
    xiNodes->initializeUsingXML(item->getXMLElement());
    contourFunction->initializeUsingXML(item->getXMLElement());
    openEta->initializeUsingXML(item->getXMLElement());
    openXi->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* SpatialContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    etaNodes->writeXMLFile(item->getXMLElement(),nullptr);
    xiNodes->writeXMLFile(item->getXMLElement(),nullptr);
    contourFunction->writeXMLFile(item->getXMLElement(),nullptr);
    openEta->writeXMLFile(item->getXMLElement(),nullptr);
    openXi->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  SpatialNurbsContourPropertyDialog::SpatialNurbsContourPropertyDialog(RigidContour *contour, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    vector<QString> list;
    list.emplace_back("\"equallySpaced\"");
    list.emplace_back("\"chordLength\"");
    list.emplace_back("\"none\"");
    interpolation = new ExtWidget("Interpolation",new TextChoiceWidget(list,1,true),true,false,MBSIM%"interpolation");
    addToTab("General", interpolation);

    controlPoints = new ExtWidget("Control points",new ChoiceWidget2(new MatRowsColsVarWidgetFactory(0,0),QBoxLayout::RightToLeft,5),false,false,MBSIM%"controlPoints");
    addToTab("General", controlPoints);

    numberOfEtaControlPoints = new ExtWidget("Number of eta control points",new ChoiceWidget2(new ScalarWidgetFactory(0),QBoxLayout::RightToLeft,5),false,false,MBSIM%"numberOfEtaControlPoints");
    addToTab("General", numberOfEtaControlPoints);

    numberOfXiControlPoints = new ExtWidget("Number of xi control points",new ChoiceWidget2(new ScalarWidgetFactory(0),QBoxLayout::RightToLeft,5),false,false,MBSIM%"numberOfXiControlPoints");
    addToTab("General", numberOfXiControlPoints);

    etaKnotVector = new ExtWidget("Eta knot vector",new ChoiceWidget2(new VecSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIM%"etaKnotVector");
    addToTab("General", etaKnotVector);

    xiKnotVector = new ExtWidget("Xi knot vector",new ChoiceWidget2(new VecSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIM%"xiKnotVector");
    addToTab("General", xiKnotVector);

    etaDegree = new ExtWidget("Eta degree",new ChoiceWidget2(new ScalarWidgetFactory("3"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"etaDegree");
    addToTab("General", etaDegree);

    xiDegree = new ExtWidget("Xi degree",new ChoiceWidget2(new ScalarWidgetFactory("3"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"xiDegree");
    addToTab("General", xiDegree);

    openEta = new ExtWidget("Open eta",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"openEta");
    addToTab("General", openEta);

    openXi = new ExtWidget("Open xi",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"openXi");
    addToTab("General", openXi);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* SpatialNurbsContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    interpolation->initializeUsingXML(item->getXMLElement());
    controlPoints->initializeUsingXML(item->getXMLElement());
    numberOfEtaControlPoints->initializeUsingXML(item->getXMLElement());
    numberOfXiControlPoints->initializeUsingXML(item->getXMLElement());
    etaKnotVector->initializeUsingXML(item->getXMLElement());
    xiKnotVector->initializeUsingXML(item->getXMLElement());
    etaDegree->initializeUsingXML(item->getXMLElement());
    xiDegree->initializeUsingXML(item->getXMLElement());
    openEta->initializeUsingXML(item->getXMLElement());
    openXi->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* SpatialNurbsContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    interpolation->writeXMLFile(item->getXMLElement(),nullptr);
    controlPoints->writeXMLFile(item->getXMLElement(),nullptr);
    numberOfEtaControlPoints->writeXMLFile(item->getXMLElement(),nullptr);
    numberOfXiControlPoints->writeXMLFile(item->getXMLElement(),nullptr);
    etaKnotVector->writeXMLFile(item->getXMLElement(),nullptr);
    xiKnotVector->writeXMLFile(item->getXMLElement(),nullptr);
    etaDegree->writeXMLFile(item->getXMLElement(),nullptr);
    xiDegree->writeXMLFile(item->getXMLElement(),nullptr);
    openEta->writeXMLFile(item->getXMLElement(),nullptr);
    openXi->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  FlexiblePlanarNurbsContourPropertyDialog::FlexiblePlanarNurbsContourPropertyDialog(Contour *contour, QWidget *parent, const Qt::WindowFlags& f) : ContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    interpolation = new ExtWidget("Interpolation",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"interpolation");
    addToTab("General", interpolation);

    indices = new ExtWidget("Indices",new ChoiceWidget2(new VecSizeVarWidgetFactory(0),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"indices");
    addToTab("General", indices);

    knotVector = new ExtWidget("Knot vector",new ChoiceWidget2(new VecSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"knotVector");
    addToTab("General", knotVector);

    degree = new ExtWidget("Degree",new ChoiceWidget2(new ScalarWidgetFactory("3"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"degree");
    addToTab("General", degree);

    open = new ExtWidget("Open",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"open");
    addToTab("General", open);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIMFLEX%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* FlexiblePlanarNurbsContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    interpolation->initializeUsingXML(item->getXMLElement());
    indices->initializeUsingXML(item->getXMLElement());
    knotVector->initializeUsingXML(item->getXMLElement());
    degree->initializeUsingXML(item->getXMLElement());
    open->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FlexiblePlanarNurbsContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    interpolation->writeXMLFile(item->getXMLElement(),nullptr);
    indices->writeXMLFile(item->getXMLElement(),nullptr);
    knotVector->writeXMLFile(item->getXMLElement(),nullptr);
    degree->writeXMLFile(item->getXMLElement(),nullptr);
    open->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  FlexibleSpatialNurbsContourPropertyDialog::FlexibleSpatialNurbsContourPropertyDialog(Contour *contour, QWidget *parent, const Qt::WindowFlags& f) : ContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    interpolation = new ExtWidget("Interpolation",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"interpolation");
    addToTab("General", interpolation);

    indices = new ExtWidget("Indices",new ChoiceWidget2(new MatRowsColsVarWidgetFactory(0,0),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"indices");
    addToTab("General", indices);

    etaKnotVector = new ExtWidget("Eta knot vector",new ChoiceWidget2(new VecSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"etaKnotVector");
    addToTab("General", etaKnotVector);

    xiKnotVector = new ExtWidget("Xi knot vector",new ChoiceWidget2(new VecSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"xiKnotVector");
    addToTab("General", xiKnotVector);

    etaDegree = new ExtWidget("Eta degree",new ChoiceWidget2(new ScalarWidgetFactory("3"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"etaDegree");
    addToTab("General", etaDegree);

    xiDegree = new ExtWidget("Xi degree",new ChoiceWidget2(new ScalarWidgetFactory("3"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"xiDegree");
    addToTab("General", xiDegree);

    openEta = new ExtWidget("Open eta",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"openEta");
    addToTab("General", openEta);

    openXi = new ExtWidget("Open xi",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"openXi");
    addToTab("General", openXi);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIMFLEX%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* FlexibleSpatialNurbsContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    interpolation->initializeUsingXML(item->getXMLElement());
    indices->initializeUsingXML(item->getXMLElement());
    etaKnotVector->initializeUsingXML(item->getXMLElement());
    xiKnotVector->initializeUsingXML(item->getXMLElement());
    etaDegree->initializeUsingXML(item->getXMLElement());
    xiDegree->initializeUsingXML(item->getXMLElement());
    openEta->initializeUsingXML(item->getXMLElement());
    openXi->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FlexibleSpatialNurbsContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    interpolation->writeXMLFile(item->getXMLElement(),nullptr);
    indices->writeXMLFile(item->getXMLElement(),nullptr);
    etaKnotVector->writeXMLFile(item->getXMLElement(),nullptr);
    xiKnotVector->writeXMLFile(item->getXMLElement(),nullptr);
    etaDegree->writeXMLFile(item->getXMLElement(),nullptr);
    xiDegree->writeXMLFile(item->getXMLElement(),nullptr);
    openEta->writeXMLFile(item->getXMLElement(),nullptr);
    openXi->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  FCLContourPropertyDialog::FCLContourPropertyDialog(RigidContour *contour, QWidget *parent, const Qt::WindowFlags& f) : RigidContourPropertyDialog(contour,parent,f) {
    computeLocalAABB = new ExtWidget("Compute local AABB",new ChoiceWidget2(new BoolWidgetFactory("true"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"computeLocalAABB");
    addToTab("General", computeLocalAABB);
  }

  DOMElement* FCLContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    computeLocalAABB->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FCLContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    computeLocalAABB->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  FCLBoxPropertyDialog::FCLBoxPropertyDialog(RigidContour *contour, QWidget *parent, const Qt::WindowFlags& f) : FCLContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    length = new ExtWidget("Length",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(3,lengthUnits()),vector<int>(3,4)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"length");
    addToTab("General", length);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* FCLBoxPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FCLContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    length->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FCLBoxPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FCLContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    length->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  FCLSpherePropertyDialog::FCLSpherePropertyDialog(RigidContour *contour, QWidget *parent, const Qt::WindowFlags& f) : FCLContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    radius = new ExtWidget("Radius",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"radius");
    addToTab("General", radius);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* FCLSpherePropertyDialog::initializeUsingXML(DOMElement *parent) {
    FCLContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    radius->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FCLSpherePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FCLContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    radius->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  FCLPlanePropertyDialog::FCLPlanePropertyDialog(RigidContour *contour, QWidget *parent, const Qt::WindowFlags& f) : FCLContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    vector<QString> n = getVec<QString>(3,"0");
    n[0] = "1";
    normal = new ExtWidget("Normal",new ChoiceWidget2(new VecWidgetFactory(n,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"normal");
    addToTab("General", normal);

    offset = new ExtWidget("Offset",new ChoiceWidget2(new ScalarWidgetFactory(0),QBoxLayout::RightToLeft,5),false,false,MBSIM%"offset");
    addToTab("General", offset);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* FCLPlanePropertyDialog::initializeUsingXML(DOMElement *parent) {
    FCLContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    normal->initializeUsingXML(item->getXMLElement());
    offset->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FCLPlanePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FCLContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    normal->writeXMLFile(item->getXMLElement(),nullptr);
    offset->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  FCLMeshPropertyDialog::FCLMeshPropertyDialog(RigidContour *contour, QWidget *parent, const Qt::WindowFlags& f) : FCLContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    vertices = new ExtWidget("Vertices",new ChoiceWidget2(new MatRowsVarWidgetFactory(1,3,vector<QStringList>(3,lengthUnits()),vector<int>(3,4)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"vertices");
    addToTab("General", vertices);

    triangles = new ExtWidget("Triangles",new ChoiceWidget2(new MatRowsVarWidgetFactory(1,3,vector<QStringList>(3,QStringList()),vector<int>(3,2)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"triangles");
    addToTab("General", triangles);

    vector<QString> list;
    list.emplace_back("\"AABB\"");
    list.emplace_back("\"KDOP16\"");
    list.emplace_back("\"KDOP18\"");
    list.emplace_back("\"KDOP24\"");
    list.emplace_back("\"kIOS\"");
    list.emplace_back("\"OBB\"");
    list.emplace_back("\"OBBRSS\"");
    list.emplace_back("\"RSS\"");
    collisionStructure = new ExtWidget("Collision structure",new TextChoiceWidget(list,0,true),true,false,MBSIM%"collisionStructure");
    addToTab("General", collisionStructure);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVColoreBodyWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* FCLMeshPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FCLContourPropertyDialog::initializeUsingXML(item->getXMLElement());
    vertices->initializeUsingXML(item->getXMLElement());
    triangles->initializeUsingXML(item->getXMLElement());
    collisionStructure->initializeUsingXML(item->getXMLElement());
    visu->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FCLMeshPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FCLContourPropertyDialog::writeXMLFile(item->getXMLElement(),nullptr);
    vertices->writeXMLFile(item->getXMLElement(),nullptr);
    triangles->writeXMLFile(item->getXMLElement(),nullptr);
    collisionStructure->writeXMLFile(item->getXMLElement(),nullptr);
    visu->writeXMLFile(item->getXMLElement(),nullptr);
    return nullptr;
  }

  GroupPropertyDialog::GroupPropertyDialog(Group *group, QWidget *parent, const Qt::WindowFlags& f, bool kinematics) : ElementPropertyDialog(group,parent,f), frameOfReference(nullptr) {
    if(kinematics) {
      addTab("Kinematics",1);

      frameOfReference = new ExtWidget("Frame of reference",new ParentFrameOfReferenceWidget(group,nullptr),true,false,MBSIM%"frameOfReference");
      addToTab("Kinematics", frameOfReference);
    }
  }

  DOMElement* GroupPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ElementPropertyDialog::initializeUsingXML(item->getXMLElement());
    if(frameOfReference)
      frameOfReference->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GroupPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ElementPropertyDialog::writeXMLFile(parent,ref?ref:getElement()->getXMLFrames());
    if(frameOfReference)
      frameOfReference->writeXMLFile(item->getXMLElement(),ref?ref:getElement()->getXMLFrames());
    return nullptr;
  }

  DynamicSystemSolverPropertyDialog::DynamicSystemSolverPropertyDialog(DynamicSystemSolver *solver, QWidget *parent, const Qt::WindowFlags& f) : GroupPropertyDialog(solver,parent,f,false) {
    addTab("Environment",1);
    addTab("Solver parameters",2);
    addTab("Extra");

    environment = new ExtWidget("Acceleration of gravity",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(3,accelerationUnits())),QBoxLayout::RightToLeft,5),false,false,MBSIM%"accelerationOfGravity");
    addToTab("Environment", environment);

    vector<QString> list;
    list.emplace_back("\"fixedpoint\"");
    list.emplace_back("\"GaussSeidel\"");
    list.emplace_back("\"direct\"");
    list.emplace_back("\"rootfinding\"");
    constraintSolver = new ExtWidget("Constraint solver",new TextChoiceWidget(list,0,true),true,false,MBSIM%"constraintSolver");
    addToTab("Solver parameters", constraintSolver);

    impactSolver = new ExtWidget("Impact solver",new TextChoiceWidget(list,0,true),true,false,MBSIM%"impactSolver");
    addToTab("Solver parameters", impactSolver);

    maxIter = new ExtWidget("Maximum iterations",new ChoiceWidget2(new ScalarWidgetFactory("10000"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"maximumIterations");
    addToTab("Solver parameters", maxIter);

    numericalJacobian = new ExtWidget("Numerical jacobian",new ChoiceWidget2(new BoolWidgetFactory("false"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"numericalJacobian");
    addToTab("Solver parameters", numericalJacobian);

    list.clear();
    list.emplace_back("\"LUDecomposition\"");
    list.emplace_back("\"LevenbergMarquardt\"");
    list.emplace_back("\"pseudoinverse\"");
    linearAlgebra = new ExtWidget("Linear algebra",new TextChoiceWidget(list,0,true),true,false,MBSIM%"linearAlgebra");
    addToTab("Solver parameters", linearAlgebra);

    projection = new ExtWidget("Projection tolerance",new ChoiceWidget2(new ScalarWidgetFactory("1e-15"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"projectionTolerance");
    addToTab("Solver parameters", projection);

    gTol = new ExtWidget("Generalized relative position tolerance",new ChoiceWidget2(new ScalarWidgetFactory("1e-8"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"generalizedRelativePositionTolerance");
    addToTab("Solver parameters", gTol);

    gdTol = new ExtWidget("Generalized relative velocity tolerance",new ChoiceWidget2(new ScalarWidgetFactory("1e-10"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"generalizedRelativeVelocityTolerance");
    addToTab("Solver parameters", gdTol);

    gddTol = new ExtWidget("Generalized relative acceleration tolerance",new ChoiceWidget2(new ScalarWidgetFactory("1e-12"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"generalizedRelativeAccelerationTolerance");
    addToTab("Solver parameters", gddTol);

    laTol = new ExtWidget("Generalized force tolerance",new ChoiceWidget2(new ScalarWidgetFactory("1e-12"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"generalizedForceTolerance");
    addToTab("Solver parameters", laTol);

    LaTol = new ExtWidget("Generalized impulse tolerance",new ChoiceWidget2(new ScalarWidgetFactory("1e-10"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"generalizedImpulseTolerance");
    addToTab("Solver parameters", LaTol);

    gCorr = new ExtWidget("Generalized relative position correction value",new ChoiceWidget2(new ScalarWidgetFactory("1e-14"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"generalizedRelativePositionCorrectionValue");
    addToTab("Solver parameters", gCorr);

    gdCorr = new ExtWidget("Generalized relative velocity correction value",new ChoiceWidget2(new ScalarWidgetFactory("1e-16"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"generalizedRelativeVelocityCorrectionValue");
    addToTab("Solver parameters", gdCorr);

    inverseKinetics = new ExtWidget("Inverse kinetics",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"inverseKinetics");
    addToTab("Extra", inverseKinetics);

    initialProjection = new ExtWidget("Initial projection",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"initialProjection");
    addToTab("Extra", initialProjection);

    useConstraintSolverForSmoothMotion = new ExtWidget("Use constraint solver for smooth motion",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"useConstraintSolverForSmoothMotion");
    addToTab("Extra", useConstraintSolverForSmoothMotion);

    useConstraintSolverForPlot = new ExtWidget("Use constraint solver for plot",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"useConstraintSolverForPlot");
    addToTab("Extra", useConstraintSolverForPlot);
  }

  DOMElement* DynamicSystemSolverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    GroupPropertyDialog::initializeUsingXML(item->getXMLElement());
    environment->initializeUsingXML(static_cast<DynamicSystemSolver*>(item)->getXMLEnvironments()->getFirstElementChild());
    constraintSolver->initializeUsingXML(item->getXMLElement());
    impactSolver->initializeUsingXML(item->getXMLElement());
    maxIter->initializeUsingXML(item->getXMLElement());
    numericalJacobian->initializeUsingXML(item->getXMLElement());
    linearAlgebra->initializeUsingXML(item->getXMLElement());
    projection->initializeUsingXML(item->getXMLElement());
    gTol->initializeUsingXML(item->getXMLElement());
    gdTol->initializeUsingXML(item->getXMLElement());
    gddTol->initializeUsingXML(item->getXMLElement());
    laTol->initializeUsingXML(item->getXMLElement());
    LaTol->initializeUsingXML(item->getXMLElement());
    gCorr->initializeUsingXML(item->getXMLElement());
    gdCorr->initializeUsingXML(item->getXMLElement());
    inverseKinetics->initializeUsingXML(item->getXMLElement());
    initialProjection->initializeUsingXML(item->getXMLElement());
    useConstraintSolverForSmoothMotion->initializeUsingXML(item->getXMLElement());
    useConstraintSolverForPlot->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* DynamicSystemSolverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    GroupPropertyDialog::writeXMLFile(parent,getElement()->getXMLFrames());
    environment->writeXMLFile(static_cast<DynamicSystemSolver*>(item)->getXMLEnvironments()->getFirstElementChild());
    constraintSolver->writeXMLFile(item->getXMLElement());
    impactSolver->writeXMLFile(item->getXMLElement());
    maxIter->writeXMLFile(item->getXMLElement());
    numericalJacobian->writeXMLFile(item->getXMLElement());
    linearAlgebra->writeXMLFile(item->getXMLElement());
    projection->writeXMLFile(item->getXMLElement());
    gTol->writeXMLFile(item->getXMLElement());
    gdTol->writeXMLFile(item->getXMLElement());
    gddTol->writeXMLFile(item->getXMLElement());
    laTol->writeXMLFile(item->getXMLElement());
    LaTol->writeXMLFile(item->getXMLElement());
    gCorr->writeXMLFile(item->getXMLElement());
    gdCorr->writeXMLFile(item->getXMLElement());
    inverseKinetics->writeXMLFile(item->getXMLElement());
    initialProjection->writeXMLFile(item->getXMLElement());
    useConstraintSolverForSmoothMotion->writeXMLFile(item->getXMLElement());
    useConstraintSolverForPlot->writeXMLFile(item->getXMLElement());
    return nullptr;
  }

  ObjectPropertyDialog::ObjectPropertyDialog(Object *object, QWidget *parent, const Qt::WindowFlags& f) : ElementPropertyDialog(object,parent,f) {
    addTab("Initial conditions",1);

    q0 = new ExtWidget("Generalized initial position",new ChoiceWidget2(new VecSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIM%"generalizedInitialPosition");
    addToTab("Initial conditions", q0);

    u0 = new ExtWidget("Generalized initial velocity",new ChoiceWidget2(new VecSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIM%"generalizedInitialVelocity");
    addToTab("Initial conditions", u0);

    connect(q0, SIGNAL(widgetChanged()), this, SLOT(updateWidget()));
    connect(u0, SIGNAL(widgetChanged()), this, SLOT(updateWidget()));
  }

  DOMElement* ObjectPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ElementPropertyDialog::initializeUsingXML(item->getXMLElement());
    q0->initializeUsingXML(item->getXMLElement());
    u0->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* ObjectPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ElementPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    q0->writeXMLFile(item->getXMLElement(),ref);
    u0->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  BodyPropertyDialog::BodyPropertyDialog(Body *body, QWidget *parent, const Qt::WindowFlags& f) : ObjectPropertyDialog(body,parent,f) {
    addTab("Kinematics",1);

    R = new ExtWidget("Frame of reference",new ElementOfReferenceWidget<Frame>(body,body->getParent()->getFrame(0)),true,false,MBSIM%"frameOfReference");
    addToTab("Kinematics",R);
  }

  DOMElement* BodyPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObjectPropertyDialog::initializeUsingXML(item->getXMLElement());
    R->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* BodyPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObjectPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    R->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  RigidBodyPropertyDialog::RigidBodyPropertyDialog(RigidBody *body, QWidget *parent, const Qt::WindowFlags& f) : BodyPropertyDialog(body,parent,f) {
    addTab("Visualisation",3);

    K = new ExtWidget("Frame for kinematics",new LocalFrameOfReferenceWidget(body,nullptr),true,false,MBSIM%"frameForKinematics");
    addToTab("Kinematics",K);

    mass = new ExtWidget("Mass",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,massUnits()),vector<int>(2,2)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"mass");
    addToTab("General",mass);

    inertia = new ExtWidget("Inertia tensor",new ChoiceWidget2(new SymMatWidgetFactory(getEye<QString>(3,3,"0.01","0"),vector<QStringList>(3,inertiaUnits()),vector<int>(3,2)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"inertiaTensor");
    addToTab("General",inertia);

    frameForInertiaTensor = new ExtWidget("Frame for inertia tensor",new LocalFrameOfReferenceWidget(body,nullptr),true,false,MBSIM%"frameForInertiaTensor");
    addToTab("General",frameForInertiaTensor);

    translation = new ExtWidget("Translation",new ChoiceWidget2(new TranslationWidgetFactory4(body),QBoxLayout::TopToBottom,3),true,false,"");
    addToTab("Kinematics", translation);
    connect(translation,SIGNAL(widgetChanged()),this,SLOT(updateWidget()));

    rotation = new ExtWidget("Rotation",new ChoiceWidget2(new RotationWidgetFactory4(body),QBoxLayout::TopToBottom,3),true,false,"");
    addToTab("Kinematics", rotation);
    connect(rotation,SIGNAL(widgetChanged()),this,SLOT(updateWidget()));

    translationDependentRotation = new ExtWidget("Translation dependent rotation",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"translationDependentRotation");
    addToTab("Kinematics", translationDependentRotation);

    coordinateTransformationForRotation = new ExtWidget("Coordinate transformation for rotation",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"coordinateTransformationForRotation");
    addToTab("Kinematics", coordinateTransformationForRotation);

    bodyFixedRepresentationOfAngularVelocity = new ExtWidget("Body-fixed representation of angular velocity",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"bodyFixedRepresentationOfAngularVelocity");
    addToTab("Kinematics", bodyFixedRepresentationOfAngularVelocity);

    ombv = new ExtWidget("OpenMBV Body",new ChoiceWidget2(new OMBVRigidBodyWidgetFactory,QBoxLayout::TopToBottom,0),true,true,MBSIM%"openMBVRigidBody");
    addToTab("Visualisation", ombv);

    ombvFrameRef=new ExtWidget("OpenMBV frame of reference",new LocalFrameOfReferenceWidget(body),true,false,MBSIM%"openMBVFrameOfReference");
    addToTab("Visualisation", ombvFrameRef);
  }

  DOMElement* RigidBodyPropertyDialog::initializeUsingXML(DOMElement *parent) {
    BodyPropertyDialog::initializeUsingXML(item->getXMLElement());
    K->initializeUsingXML(item->getXMLElement());
    mass->initializeUsingXML(item->getXMLElement());
    inertia->initializeUsingXML(item->getXMLElement());
    frameForInertiaTensor->initializeUsingXML(item->getXMLElement());
    translation->initializeUsingXML(item->getXMLElement());
    rotation->initializeUsingXML(item->getXMLElement());
    translationDependentRotation->initializeUsingXML(item->getXMLElement());
    coordinateTransformationForRotation->initializeUsingXML(item->getXMLElement());
    bodyFixedRepresentationOfAngularVelocity->initializeUsingXML(item->getXMLElement());
    ombv->initializeUsingXML(item->getXMLElement());
    ombvFrameRef->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* RigidBodyPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    BodyPropertyDialog::writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    K->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    mass->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    inertia->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    frameForInertiaTensor->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    translation->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    rotation->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    translationDependentRotation->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    coordinateTransformationForRotation->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    bodyFixedRepresentationOfAngularVelocity->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    DOMElement *ele =getElement()->getXMLContours()->getNextElementSibling();
    ombv->writeXMLFile(item->getXMLElement(),ele);
    ombvFrameRef->writeXMLFile(item->getXMLElement(),ele);
    return nullptr;
  }

  int RigidBodyPropertyDialog::getqRelSize() const {
    int nqT=0, nqR=0;
    if(translation->isActive()) {
      if(static_cast<ChoiceWidget2*>(translation->getWidget())->getIndex()!=1) {
        auto *trans = dynamic_cast<FunctionWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(translation->getWidget())->getWidget())->getWidget());
        if(trans)
          nqT = trans->getArg1Size();
      }
    }
    if(rotation->isActive()) {
      if(static_cast<ChoiceWidget2*>(rotation->getWidget())->getIndex()!=1) {
        auto *rot = dynamic_cast<FunctionWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(rotation->getWidget())->getWidget())->getWidget());
        if(rot)
          nqR = rot->getArg1Size();
      }
    }
    int nq = nqT + nqR;
    return nq;
  }

  int RigidBodyPropertyDialog::getuRelSize() const {
    return getqRelSize();
  }

  void RigidBodyPropertyDialog::resizeGeneralizedPosition() {
    int size =  static_cast<RigidBody*>(item)->isConstrained() ? 0 : getqRelSize();
    q0->resize_(size,1);
  }

  void RigidBodyPropertyDialog::resizeGeneralizedVelocity() {
    int size =  static_cast<RigidBody*>(item)->isConstrained() ? 0 : getuRelSize();
    u0->resize_(size,1);
  }

  FlexibleBodyFFRPropertyDialog::FlexibleBodyFFRPropertyDialog(FlexibleBodyFFR *body, QWidget *parent, const Qt::WindowFlags& f) : BodyPropertyDialog(body,parent,f) {
    addTab("Visualisation",3);
    addTab("Nodal data");

    mass = new ExtWidget("Mass",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,massUnits()),vector<int>(2,2)),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"mass");
    addToTab("General",mass);

    rdm = new ExtWidget("Position integral",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(3,QStringList()),vector<int>(3,0)),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"positionIntegral");
    addToTab("General", rdm);

    rrdm = new ExtWidget("Position position integral",new ChoiceWidget2(new SymMatWidgetFactory(getEye<QString>(3,3,"0","0"),vector<QStringList>(3,inertiaUnits()),vector<int>(3,2)),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"positionPositionIntegral");
    addToTab("General",rrdm);

    Pdm = new ExtWidget("Shape function integral",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"shapeFunctionIntegral");
    addToTab("General",Pdm);

    rPdm = new ExtWidget("Position shape function integral",new ChoiceWidget2(new OneDimMatArrayWidgetFactory(MBSIMFLEX%"positionShapeFunctionIntegral",3,3,1),QBoxLayout::TopToBottom,3),true,false,"");
    addToTab("General",rPdm);

    PPdm = new ExtWidget("Shape function shape function integral",new ChoiceWidget2(new TwoDimMatArrayWidgetFactory(MBSIMFLEX%"shapeFunctionShapeFunctionIntegral",3,1,1),QBoxLayout::TopToBottom,3),true,false,"");
    addToTab("General",PPdm);

    Ke = new ExtWidget("Stiffness matrix",new ChoiceWidget2(new SymMatWidgetFactory(getMat<QString>(1,1,"0")),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"stiffnessMatrix");
    addToTab("General",Ke);

    De = new ExtWidget("Damping matrix",new ChoiceWidget2(new SymMatWidgetFactory(getMat<QString>(1,1,"0")),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"dampingMatrix");
    addToTab("General",De);

    beta = new ExtWidget("Proportional damping",new ChoiceWidget2(new VecWidgetFactory(2),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"proportionalDamping");
    addToTab("General", beta);

    Knl1 = new ExtWidget("Nonlinear stiffness matrix of first order",new ChoiceWidget2(new OneDimMatArrayWidgetFactory(MBSIMFLEX%"nonlinearStiffnessMatrixOfFirstOrder"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("General",Knl1);

    Knl2 = new ExtWidget("Nonlinear stiffness matrix of second order",new ChoiceWidget2(new TwoDimMatArrayWidgetFactory(MBSIMFLEX%"nonlinearStiffnessMatrixOfSecondOrder"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("General",Knl2);

    ksigma0 = new ExtWidget("Initial stress integral",new ChoiceWidget2(new VecWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"initialStressIntegral");
    addToTab("General", ksigma0);

    ksigma1 = new ExtWidget("Nonlinear initial stress integral",new ChoiceWidget2(new MatWidgetFactory(1,1),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nonlinearInitialStressIntegral");
    addToTab("General", ksigma1);

    K0t = new ExtWidget("Geometric stiffness matrix due to acceleration",new ChoiceWidget2(new OneDimMatArrayWidgetFactory(MBSIMFLEX%"geometricStiffnessMatrixDueToAcceleration"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("General",K0t);

    K0r = new ExtWidget("Geometric stiffness matrix due to angular acceleration",new ChoiceWidget2(new OneDimMatArrayWidgetFactory(MBSIMFLEX%"geometricStiffnessMatrixDueToAngularAcceleration"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("General",K0r);

    K0om = new ExtWidget("Geometric stiffness matrix due to angular velocity",new ChoiceWidget2(new OneDimMatArrayWidgetFactory(MBSIMFLEX%"geometricStiffnessMatrixDueToAngularVelocity"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("General",K0om);

    r = new ExtWidget("Nodal relative position",new ChoiceWidget2(new OneDimVecArrayWidgetFactory(MBSIMFLEX%"nodalRelativePosition",1,3,true),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("Nodal data", r);

    A = new ExtWidget("Nodal relative orientation",new ChoiceWidget2(new OneDimMatArrayWidgetFactory(MBSIMFLEX%"nodalRelativeOrientation"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("Nodal data", A);

    Phi = new ExtWidget("Nodal shape matrix of translation",new ChoiceWidget2(new OneDimMatArrayWidgetFactory(MBSIMFLEX%"nodalShapeMatrixOfTranslation"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("Nodal data", Phi);

    Psi = new ExtWidget("Nodal shape matrix of rotation",new ChoiceWidget2(new OneDimMatArrayWidgetFactory(MBSIMFLEX%"nodalShapeMatrixOfRotation"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("Nodal data", Psi);

    sigmahel = new ExtWidget("Nodal stress matrix",new ChoiceWidget2(new OneDimMatArrayWidgetFactory(MBSIMFLEX%"nodalStressMatrix"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("Nodal data", sigmahel);

    sigmahen = new ExtWidget("Nodal nonlinear stress matrix",new ChoiceWidget2(new TwoDimMatArrayWidgetFactory(MBSIMFLEX%"nodalNonlinearStressMatrix"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("Nodal data", sigmahen);

    sigma0 = new ExtWidget("Nodal initial stress",new ChoiceWidget2(new OneDimVecArrayWidgetFactory(MBSIMFLEX%"nodalInitialStress"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("Nodal data", sigma0);

    K0F = new ExtWidget("Nodal geometric stiffness matrix due to force",new ChoiceWidget2(new TwoDimMatArrayWidgetFactory(MBSIMFLEX%"nodalGeometricStiffnessMatrixDueToForce"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("Nodal data", K0F);

    K0M = new ExtWidget("Nodal geometric stiffness matrix due to moment",new ChoiceWidget2(new TwoDimMatArrayWidgetFactory(MBSIMFLEX%"nodalGeometricStiffnessMatrixDueToMoment"),QBoxLayout::RightToLeft,3),true,false,"");
    addToTab("Nodal data", K0M);

    translation = new ExtWidget("Translation",new ChoiceWidget2(new TranslationWidgetFactory4(body,MBSIMFLEX),QBoxLayout::TopToBottom,3),true,false,"");
    addToTab("Kinematics", translation);
    connect(translation,SIGNAL(widgetChanged()),this,SLOT(updateWidget()));

    rotation = new ExtWidget("Rotation",new ChoiceWidget2(new RotationWidgetFactory4(body,MBSIMFLEX),QBoxLayout::TopToBottom,3),true,false,"");
    addToTab("Kinematics", rotation);
    connect(rotation,SIGNAL(widgetChanged()),this,SLOT(updateWidget()));

    translationDependentRotation = new ExtWidget("Translation dependent rotation",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"translationDependentRotation");
    addToTab("Kinematics", translationDependentRotation);

    coordinateTransformationForRotation = new ExtWidget("Coordinate transformation for rotation",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"coordinateTransformationForRotation");
    addToTab("Kinematics", coordinateTransformationForRotation);

    ombv = new ExtWidget("OpenMBV Body",new ChoiceWidget2(new OMBVFlexibleBodyWidgetFactory,QBoxLayout::TopToBottom,0),true,true,MBSIMFLEX%"openMBVFlexibleBody");
    addToTab("Visualisation", ombv);

    ombvNodes = new ExtWidget("OpenMBV nodes",new ChoiceWidget2(new VecSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"openMBVNodes");
    addToTab("Visualisation", ombvNodes);

    vector<QString> list;
    list.emplace_back("\"none\"");
    list.emplace_back("\"xDisplacement\"");
    list.emplace_back("\"yDisplacement\"");
    list.emplace_back("\"zDisplacement\"");
    list.emplace_back("\"totalDisplacement\"");
    list.emplace_back("\"xxStress\"");
    list.emplace_back("\"yyStress\"");
    list.emplace_back("\"zzStress\"");
    list.emplace_back("\"xyStress\"");
    list.emplace_back("\"yzStress\"");
    list.emplace_back("\"zxStress\"");
    list.emplace_back("\"equivalentStress\"");
    ombvColorRepresentation = new ExtWidget("OpenMBV color representation",new TextChoiceWidget(list,0,true),true,false,MBSIMFLEX%"openMBVColorRepresentation");
    addToTab("Visualisation", ombvColorRepresentation);

    QPushButton *import = new QPushButton("Import");
    static_cast<QGridLayout*>(QWidget::layout())->addWidget(import,1,0);
    connect(import,SIGNAL(clicked()),this,SLOT(import()));

    connect(Pdm->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
//    connect(Knl1,SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
  }

  FlexibleBodyFFRPropertyDialog::~FlexibleBodyFFRPropertyDialog() {
    delete dialog;
  }

  void FlexibleBodyFFRPropertyDialog::updateWidget() {
    BodyPropertyDialog::updateWidget();
    int size = static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(Pdm->getWidget())->getWidget())->cols();
    if(static_cast<ChoiceWidget2*>(rPdm->getWidget())->getIndex()==0)
      rPdm->resize_(3,size);
    else
      rPdm->resize_(9,size);
    if(static_cast<ChoiceWidget2*>(PPdm->getWidget())->getIndex()==0)
      PPdm->resize_(size,size);
    else
      PPdm->resize_(9*size,size);
    Ke->resize_(size,size);
    De->resize_(size,size);
    if(Knl1->isActive()) {
      if(static_cast<ChoiceWidget2*>(Knl1->getWidget())->getIndex()==0)
        static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(Knl1->getWidget())->getWidget())->resize_(size,size,size);
      else
        Knl1->resize_(size*size,size);
    }
    if(Knl2->isActive()) {
      if(static_cast<ChoiceWidget2*>(Knl2->getWidget())->getIndex()==0)
        static_cast<TwoDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(Knl2->getWidget())->getWidget())->resize_(size,size,size,size);
      else
        Knl2->resize_(size*size*size,size);
    }
    ksigma0->resize_(size,1);
    ksigma1->resize_(size,size);
    if(K0t->isActive()) {
      if(static_cast<ChoiceWidget2*>(K0t->getWidget())->getIndex()==0)
        static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(K0t->getWidget())->getWidget())->resize_(3,size,size);
      else
        K0t->resize_(3*size,size);
    }
    if(K0r->isActive()) {
      if(static_cast<ChoiceWidget2*>(K0r->getWidget())->getIndex()==0)
        static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(K0r->getWidget())->getWidget())->resize_(3,size,size);
      else
        K0r->resize_(3*size,size);
    }
    if(K0om->isActive()) {
      if(static_cast<ChoiceWidget2*>(K0om->getWidget())->getIndex()==0)
        static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(K0om->getWidget())->getWidget())->resize_(3,size,size);
      else
        K0om->resize_(3*size,size);
    }
    if(r->isActive()) {
      int rsize;
      if(static_cast<ChoiceWidget2*>(r->getWidget())->getIndex()==0)
        rsize = static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(r->getWidget())->getWidget())->getArray().size();
      else
        rsize = static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(r->getWidget())->getWidget())->getWidget())->rows()/3;
      if(A->isActive()) {
        if(static_cast<ChoiceWidget2*>(A->getWidget())->getIndex()==0)
          static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(A->getWidget())->getWidget())->resize_(rsize,3,3);
        else
          A->resize_(3*rsize,3);
      }
      if(Phi->isActive()) {
        if(static_cast<ChoiceWidget2*>(Phi->getWidget())->getIndex()==0)
          static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(Phi->getWidget())->getWidget())->resize_(rsize,3,size);
        else
          Phi->resize_(3*rsize,size);
      }
      if(Psi->isActive()) {
        if(static_cast<ChoiceWidget2*>(Psi->getWidget())->getIndex()==0)
          static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(Psi->getWidget())->getWidget())->resize_(rsize,3,size);
        else
          Psi->resize_(3*rsize,size);
      }
      if(sigmahel->isActive()) {
        if(static_cast<ChoiceWidget2*>(sigmahel->getWidget())->getIndex()==0)
          static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(sigmahel->getWidget())->getWidget())->resize_(rsize,6,size);
        else
          sigmahel->resize_(6*rsize,size);
      }
      if(sigmahen->isActive()) {
        if(static_cast<ChoiceWidget2*>(sigmahen->getWidget())->getIndex()==0)
          static_cast<TwoDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(sigmahen->getWidget())->getWidget())->resize_(rsize,size,6,size);
        else
          sigmahen->resize_(6*rsize*size,size);
      }
      if(sigma0->isActive()) {
        if(static_cast<ChoiceWidget2*>(sigma0->getWidget())->getIndex()==0)
          static_cast<OneDimVecArrayWidget*>(static_cast<ChoiceWidget2*>(sigma0->getWidget())->getWidget())->resize_(rsize,6,1);
        else
          sigma0->resize_(6*rsize,1);
      }
      if(K0F->isActive()) {
        if(static_cast<ChoiceWidget2*>(K0F->getWidget())->getIndex()==0)
          static_cast<TwoDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(K0F->getWidget())->getWidget())->resize_(rsize,size,size,size);
        else
          K0F->resize_(size*rsize*size,size);
      }
      if(K0M->isActive()) {
        if(static_cast<ChoiceWidget2*>(K0M->getWidget())->getIndex()==0)
          static_cast<TwoDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(K0M->getWidget())->getWidget())->resize_(rsize,size,size,size);
        else
          K0M->resize_(size*rsize*size,size);
      }
    }
  }

  int FlexibleBodyFFRPropertyDialog::getqRelSize() const {
    int nqT=0, nqR=0, nqE=0;
    if(translation->isActive()) {
      if(static_cast<ChoiceWidget2*>(translation->getWidget())->getIndex()!=1) {
        auto *trans = dynamic_cast<FunctionWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(translation->getWidget())->getWidget())->getWidget());
        if(trans)
          nqT = trans->getArg1Size();
      }
    }
    if(rotation->isActive()) {
      if(static_cast<ChoiceWidget2*>(rotation->getWidget())->getIndex()!=1) {
        auto *rot = dynamic_cast<FunctionWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(rotation->getWidget())->getWidget())->getWidget());
        if(rot)
          nqR = rot->getArg1Size();
      }
    }
    if(Pdm->isActive())
      nqE = static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(Pdm->getWidget())->getWidget())->cols();
    int nq = nqT + nqR + nqE;
    return nq;
  }

  int FlexibleBodyFFRPropertyDialog::getuRelSize() const {
    return getqRelSize();
  }

  void FlexibleBodyFFRPropertyDialog::resizeGeneralizedPosition() {
    int size =  getqRelSize();
    q0->resize_(size,1);
  }

  void FlexibleBodyFFRPropertyDialog::resizeGeneralizedVelocity() {
    int size =  getuRelSize();
    u0->resize_(size,1);
  }

  DOMElement* FlexibleBodyFFRPropertyDialog::initializeUsingXML(DOMElement *parent) {
    BodyPropertyDialog::initializeUsingXML(item->getXMLElement());
    mass->initializeUsingXML(item->getXMLElement());
    rdm->initializeUsingXML(item->getXMLElement());
    rrdm->initializeUsingXML(item->getXMLElement());
    Pdm->initializeUsingXML(item->getXMLElement());
    rPdm->initializeUsingXML(item->getXMLElement());
    PPdm->initializeUsingXML(item->getXMLElement());
    Ke->initializeUsingXML(item->getXMLElement());
    De->initializeUsingXML(item->getXMLElement());
    beta->initializeUsingXML(item->getXMLElement());
    Knl1->initializeUsingXML(item->getXMLElement());
    Knl2->initializeUsingXML(item->getXMLElement());
    ksigma0->initializeUsingXML(item->getXMLElement());
    ksigma1->initializeUsingXML(item->getXMLElement());
    K0t->initializeUsingXML(item->getXMLElement());
    K0r->initializeUsingXML(item->getXMLElement());
    K0om->initializeUsingXML(item->getXMLElement());
    r->initializeUsingXML(item->getXMLElement());
    A->initializeUsingXML(item->getXMLElement());
    Phi->initializeUsingXML(item->getXMLElement());
    Psi->initializeUsingXML(item->getXMLElement());
    sigmahel->initializeUsingXML(item->getXMLElement());
    sigmahen->initializeUsingXML(item->getXMLElement());
    sigma0->initializeUsingXML(item->getXMLElement());
    K0F->initializeUsingXML(item->getXMLElement());
    K0M->initializeUsingXML(item->getXMLElement());
    translation->initializeUsingXML(item->getXMLElement());
    rotation->initializeUsingXML(item->getXMLElement());
    translationDependentRotation->initializeUsingXML(item->getXMLElement());
    coordinateTransformationForRotation->initializeUsingXML(item->getXMLElement());
    ombv->initializeUsingXML(item->getXMLElement());
    ombvNodes->initializeUsingXML(item->getXMLElement());
    ombvColorRepresentation->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FlexibleBodyFFRPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    BodyPropertyDialog::writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    mass->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    rdm->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    rrdm->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    Pdm->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    rPdm->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    PPdm->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    Ke->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    De->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    beta->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    Knl1->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    Knl2->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    ksigma0->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    ksigma1->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    K0t->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    K0r->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    K0om->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    r->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    A->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    Phi->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    Psi->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    sigmahel->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    sigmahen->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    sigma0->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    K0F->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    K0M->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    translation->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    rotation->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    translationDependentRotation->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    coordinateTransformationForRotation->writeXMLFile(item->getXMLElement(),getElement()->getXMLFrames());
    DOMElement *ele =getElement()->getXMLContours()->getNextElementSibling();
    ombv->writeXMLFile(item->getXMLElement(),ele);
    ombvNodes->writeXMLFile(item->getXMLElement(),ele);
    ombvColorRepresentation->writeXMLFile(item->getXMLElement(),ele);
    return nullptr;
  }

  void FlexibleBodyFFRPropertyDialog::import() {
    if(not dialog) dialog = new ImportDialog;
    int status = dialog->exec();
    if(not status) return;
    ImportWidget *importWidget = dialog->getImportWidget();
    QString dir = mw->getProjectPath()+"/";
    ImportFEMData import;
    if(importWidget->getNumberOfModesChecked())
      import.setNumberOfModes(importWidget->getNumberOfModes());
    try {
      import.read((dir+importWidget->getResultFile().remove("."+QFileInfo(importWidget->getResultFile()).suffix())).toStdString());
    }
    catch(exception &err) {
      QMessageBox::warning(nullptr, "Import error", QString::fromStdString(err.what()));
      return;
    }
    if(importWidget->getMassChecked()) {
      static_cast<ChoiceWidget2*>(mass->getWidget())->setIndex(0);
      static_cast<ScalarWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(mass->getWidget())->getWidget())->getWidget())->setValue(QString::number(import.getm()));
    }
    if(importWidget->getrdmChecked()) {
      static_cast<ChoiceWidget2*>(rdm->getWidget())->setIndex(0);
      static_cast<BasicVecWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(rdm->getWidget())->getWidget())->getWidget())->setVec(VecToQvector<fmatvec::Vec3>(import.getrdm()));
    }
    if(importWidget->getrrdmChecked()) {
      static_cast<ChoiceWidget2*>(rrdm->getWidget())->setIndex(0);
      static_cast<BasicMatWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(rrdm->getWidget())->getWidget())->getWidget())->setMat(MatToQvector<fmatvec::SymMat3>(import.getrrdm()));
    }
    if(importWidget->getNumberOfModes()) {
      if(importWidget->getPdmChecked()) {
        Pdm->setActive(true);
        if(importWidget->getFilePdmChecked()) {
          static_cast<ChoiceWidget2*>(Pdm->getWidget())->setIndex(1);
          dump((dir+importWidget->getFilenamePdm()).toStdString().c_str(),import.getPdm());
          static_cast<FromFileWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(Pdm->getWidget())->getWidget())->getWidget())->setFile(importWidget->getFilenamePdm());
        }
        else {
          static_cast<ChoiceWidget2*>(Pdm->getWidget())->setIndex(2);
          static_cast<ExpressionWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(Pdm->getWidget())->getWidget())->getWidget())->setValue(QString::fromStdString(toString(import.getPdm())));
        }
      }
      if(importWidget->getrPdmChecked()) {
        rPdm->setActive(true);
        static_cast<ChoiceWidget2*>(rPdm->getWidget())->setIndex(1);
        if(importWidget->getFilerPdmChecked()) {
          static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(rPdm->getWidget())->getWidget())->setIndex(1);
          dump((dir+importWidget->getFilenamerPdm()).toStdString().c_str(),import.getrPdm());
          static_cast<FromFileWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(rPdm->getWidget())->getWidget())->getWidget())->getWidget())->setFile(importWidget->getFilenamerPdm());
        }
        else {
          static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(rPdm->getWidget())->getWidget())->setIndex(2);
          static_cast<ExpressionWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(rPdm->getWidget())->getWidget())->getWidget())->getWidget())->setValue(QString::fromStdString(toString(import.getrPdm())));
        }
      }
      if(importWidget->getPPdmChecked()) {
        PPdm->setActive(true);
        static_cast<ChoiceWidget2*>(PPdm->getWidget())->setIndex(1);
        if(importWidget->getFilePPdmChecked()) {
          static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(PPdm->getWidget())->getWidget())->setIndex(1);
          dump((dir+importWidget->getFilenamePPdm()).toStdString().c_str(),import.getPPdm());
          static_cast<FromFileWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(PPdm->getWidget())->getWidget())->getWidget())->getWidget())->setFile(importWidget->getFilenamePPdm());
        }
        else {
          static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(PPdm->getWidget())->getWidget())->setIndex(2);
          static_cast<ExpressionWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(PPdm->getWidget())->getWidget())->getWidget())->getWidget())->setValue(QString::fromStdString(toString(import.getPPdm())));
        }
      }
      if(importWidget->getKeChecked()) {
        Ke->setActive(true);
        if(importWidget->getFileKeChecked()) {
          static_cast<ChoiceWidget2*>(Ke->getWidget())->setIndex(1);
          dump((dir+importWidget->getFilenameKe()).toStdString().c_str(),import.getKe());
          static_cast<FromFileWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(Ke->getWidget())->getWidget())->getWidget())->setFile(importWidget->getFilenameKe());
        }
        else {
          static_cast<ChoiceWidget2*>(Ke->getWidget())->setIndex(2);
          static_cast<ExpressionWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(Ke->getWidget())->getWidget())->getWidget())->setValue(QString::fromStdString(toString(import.getKe())));
        }
      }
      if(importWidget->getPhiChecked()) {
        Phi->setActive(true);
        static_cast<ChoiceWidget2*>(Phi->getWidget())->setIndex(1);
        if(importWidget->getFilePhiChecked()) {
          static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(Phi->getWidget())->getWidget())->setIndex(1);
          dump((dir+importWidget->getFilenamePhi()).toStdString().c_str(),import.getPhi());
          static_cast<FromFileWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(Phi->getWidget())->getWidget())->getWidget())->getWidget())->setFile(importWidget->getFilenamePhi());
        }
        else {
          static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(Phi->getWidget())->getWidget())->setIndex(2);
          static_cast<ExpressionWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(Phi->getWidget())->getWidget())->getWidget())->getWidget())->setValue(QString::fromStdString(toString(import.getPhi())));
        }
      }
      if(importWidget->getSrChecked()) {
        sigmahel->setActive(true);
        static_cast<ChoiceWidget2*>(sigmahel->getWidget())->setIndex(1);
        if(importWidget->getFileSrChecked()) {
          static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(sigmahel->getWidget())->getWidget())->setIndex(1);
          dump((dir+importWidget->getFilenameSr()).toStdString().c_str(),import.getSr());
          static_cast<FromFileWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(sigmahel->getWidget())->getWidget())->getWidget())->getWidget())->setFile(importWidget->getFilenameSr());
        }
        else {
          static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(sigmahel->getWidget())->getWidget())->setIndex(2);
          static_cast<ExpressionWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(sigmahel->getWidget())->getWidget())->getWidget())->getWidget())->setValue(QString::fromStdString(toString(import.getSr())));
        }
      }
    }
    else {
      Pdm->setActive(false);
      rPdm->setActive(false);
      PPdm->setActive(false);
      Ke->setActive(false);
      Phi->setActive(false);
      sigmahel->setActive(false);
    }
    if(importWidget->getu0Checked()) {
      r->setActive(true);
      static_cast<ChoiceWidget2*>(r->getWidget())->setIndex(1);
      if(importWidget->getFileu0Checked()) {
        static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(r->getWidget())->getWidget())->setIndex(1);
        dump((dir+importWidget->getFilenameu0()).toStdString().c_str(),import.getu0());
        static_cast<FromFileWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(r->getWidget())->getWidget())->getWidget())->getWidget())->setFile(importWidget->getFilenameu0());
      }
      else {
        static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(r->getWidget())->getWidget())->setIndex(2);
        static_cast<ExpressionWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(r->getWidget())->getWidget())->getWidget())->getWidget())->setValue(QString::fromStdString(toString(import.getu0())));
      }
    }
    if(importWidget->getVisuChecked()) {
      ombv->setActive(true);
      static_cast<ChoiceWidget2*>(ombv->getWidget())->setIndex(importWidget->getVisu()+1);
    }
    if(importWidget->getVisuChecked() and importWidget->getNodesChecked()) {
      if(importWidget->getNodesChecked()) {
        ombvNodes->setActive(true);
        if(importWidget->getFileNodesChecked()) {
          static_cast<ChoiceWidget2*>(ombvNodes->getWidget())->setIndex(1);
          dump((dir+importWidget->getFilenameNodes()).toStdString().c_str(),import.getNodes());
          static_cast<FromFileWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(ombvNodes->getWidget())->getWidget())->getWidget())->setFile(importWidget->getFilenameNodes());
        }
        else {
          static_cast<ChoiceWidget2*>(ombvNodes->getWidget())->setIndex(2);
          static_cast<ExpressionWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(ombvNodes->getWidget())->getWidget())->getWidget())->setValue(QString::fromStdString(toString(import.getNodes())));
        }
      }
    }
    if(importWidget->getVisuChecked() and importWidget->getIndicesChecked()) {
      DynamicIndexedFaceSetWidget *body = static_cast<DynamicIndexedFaceSetWidget*>(static_cast<ChoiceWidget2*>(ombv->getWidget())->getWidget());
      if(importWidget->getIndicesChecked()) {
        if(importWidget->getFileIndicesChecked()) {
          static_cast<ChoiceWidget2*>(body->getIndices()->getWidget())->setIndex(1);
          dump((dir+importWidget->getFilenameIndices()).toStdString().c_str(),import.getIndices());
          static_cast<FromFileWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(body->getIndices()->getWidget())->getWidget())->getWidget())->setFile(importWidget->getFilenameIndices());
        }
        else {
          static_cast<ChoiceWidget2*>(body->getIndices()->getWidget())->setIndex(2);
          static_cast<ExpressionWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(body->getIndices()->getWidget())->getWidget())->getWidget())->setValue(QString::fromStdString(toString(import.getIndices())));
        }
      }
    }
  }

  ConstraintPropertyDialog::ConstraintPropertyDialog(Constraint *constraint, QWidget *parent, const Qt::WindowFlags& f) : ElementPropertyDialog(constraint,parent,f) {
  }

  MechanicalConstraintPropertyDialog::MechanicalConstraintPropertyDialog(MechanicalConstraint *constraint, QWidget *parent, const Qt::WindowFlags& f) : ConstraintPropertyDialog(constraint,parent,f) {
  }

  GeneralizedConstraintPropertyDialog::GeneralizedConstraintPropertyDialog(GeneralizedConstraint *constraint, QWidget *parent, const Qt::WindowFlags& f) : MechanicalConstraintPropertyDialog(constraint,parent,f) {

    addTab("Visualisation",2);

    support = new ExtWidget("Support frame",new ElementOfReferenceWidget<Frame>(constraint,nullptr),true,false,MBSIM%"supportFrame");
    addToTab("General",support);
  }

  DOMElement* GeneralizedConstraintPropertyDialog::initializeUsingXML(DOMElement *parent) {
    MechanicalConstraintPropertyDialog::initializeUsingXML(item->getXMLElement());
    support->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedConstraintPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    MechanicalConstraintPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    support->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  GeneralizedGearConstraintPropertyDialog::GeneralizedGearConstraintPropertyDialog(GeneralizedGearConstraint *constraint, QWidget *parent, const Qt::WindowFlags& f) : GeneralizedConstraintPropertyDialog(constraint,parent,f) {

    dependentBody = new ExtWidget("Dependent rigid body",new ElementOfReferenceWidget<RigidBody>(constraint,nullptr),false,false,MBSIM%"dependentRigidBody");
    addToTab("General", dependentBody);

    independentBodies = new ExtWidget("Independent rigid bodies",new ListWidget(new ElementOfReferenceWidgetFactory<RigidBody>(MBSIM%"independentRigidBody",constraint,true),"Independent body",0,2),false,false,"");
    addToTab("General",independentBodies);
  }

  DOMElement* GeneralizedGearConstraintPropertyDialog::initializeUsingXML(DOMElement *parent) {
    GeneralizedConstraintPropertyDialog::initializeUsingXML(item->getXMLElement());
    dependentBody->initializeUsingXML(item->getXMLElement());
    independentBodies->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedGearConstraintPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    GeneralizedConstraintPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    dependentBody->writeXMLFile(item->getXMLElement(),ref);
    independentBodies->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  GeneralizedDualConstraintPropertyDialog::GeneralizedDualConstraintPropertyDialog(GeneralizedDualConstraint *constraint, QWidget *parent, const Qt::WindowFlags& f) : GeneralizedConstraintPropertyDialog(constraint,parent,f) {

    dependentBody = new ExtWidget("Dependent rigid body",new ElementOfReferenceWidget<RigidBody>(constraint,nullptr),false,false,MBSIM%"dependentRigidBody");
    addToTab("General", dependentBody);

    independentBody = new ExtWidget("Independent rigid body",new ElementOfReferenceWidget<RigidBody>(constraint,nullptr),true,false,MBSIM%"independentRigidBody");
    addToTab("General", independentBody);
  }

  DOMElement* GeneralizedDualConstraintPropertyDialog::initializeUsingXML(DOMElement *parent) {
    GeneralizedConstraintPropertyDialog::initializeUsingXML(item->getXMLElement());
    dependentBody->initializeUsingXML(item->getXMLElement());
    independentBody->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedDualConstraintPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    GeneralizedConstraintPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    dependentBody->writeXMLFile(item->getXMLElement(),ref);
    independentBody->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  GeneralizedPositionConstraintPropertyDialog::GeneralizedPositionConstraintPropertyDialog(GeneralizedPositionConstraint *constraint, QWidget *parent, const Qt::WindowFlags& f) : GeneralizedDualConstraintPropertyDialog(constraint,parent,f) {

    constraintFunction = new ExtWidget("Constraint function",new ChoiceWidget2(new FunctionWidgetFactory2(constraint),QBoxLayout::TopToBottom,0),false,false,MBSIM%"constraintFunction");
    addToTab("General", constraintFunction);
    connect(constraintFunction->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
  }

  void GeneralizedPositionConstraintPropertyDialog::updateWidget() {
//    RigidBody *refBody = static_cast<RigidBodyOfReferenceWidget*>(dependentBody->getWidget())->getSelectedBody();
//    int size = refBody?refBody->getqRelSize():0;
//    constraintFunction->resize_(size,1);
  }

  DOMElement* GeneralizedPositionConstraintPropertyDialog::initializeUsingXML(DOMElement *parent) {
    GeneralizedDualConstraintPropertyDialog::initializeUsingXML(item->getXMLElement());
    constraintFunction->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedPositionConstraintPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    GeneralizedDualConstraintPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    constraintFunction->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  GeneralizedVelocityConstraintPropertyDialog::GeneralizedVelocityConstraintPropertyDialog(GeneralizedVelocityConstraint *constraint, QWidget *parent, const Qt::WindowFlags& f) : GeneralizedDualConstraintPropertyDialog(constraint,parent,f) {
    addTab("Initial conditions",1);

    constraintFunction = new ExtWidget("Constraint function",new ChoiceWidget2(new ConstraintWidgetFactory(constraint),QBoxLayout::TopToBottom,3),false,false,"");
    addToTab("General", constraintFunction);
    connect(constraintFunction->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));

    x0 = new ExtWidget("Initial state",new ChoiceWidget2(new VecWidgetFactory(0,vector<QStringList>(3,QStringList())),QBoxLayout::RightToLeft,5),true,false,MBSIM%"initialState");
    addToTab("Initial conditions", x0);
  }

  void GeneralizedVelocityConstraintPropertyDialog::updateWidget() {
    cout << "GeneralizedVelocityConstraintPropertyDialog::updateWidget() not yet implemented" << endl;
  }

  DOMElement* GeneralizedVelocityConstraintPropertyDialog::initializeUsingXML(DOMElement *parent) {
    GeneralizedDualConstraintPropertyDialog::initializeUsingXML(item->getXMLElement());
    x0->initializeUsingXML(item->getXMLElement());
    constraintFunction->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedVelocityConstraintPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    GeneralizedDualConstraintPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    x0->writeXMLFile(item->getXMLElement(),ref);
    constraintFunction->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  GeneralizedAccelerationConstraintPropertyDialog::GeneralizedAccelerationConstraintPropertyDialog(GeneralizedAccelerationConstraint *constraint, QWidget *parent, const Qt::WindowFlags& f) : GeneralizedDualConstraintPropertyDialog(constraint,parent,f) {
    addTab("Initial conditions",1);

    constraintFunction = new ExtWidget("Constraint function",new ChoiceWidget2(new ConstraintWidgetFactory(constraint),QBoxLayout::TopToBottom,3),false,false,"");
    addToTab("General", constraintFunction);
    connect(constraintFunction->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));

    x0 = new ExtWidget("Initial state",new ChoiceWidget2(new VecWidgetFactory(0,vector<QStringList>(3,QStringList())),QBoxLayout::RightToLeft,5),true,false,MBSIM%"initialState");
    addToTab("Initial conditions", x0);
  }

  void GeneralizedAccelerationConstraintPropertyDialog::updateWidget() {
//    RigidBody *refBody = static_cast<RigidBodyOfReferenceWidget*>(dependentBody->getWidget())->getSelectedBody();
//    int size = refBody?(refBody->getqRelSize()+refBody->getuRelSize()):0;
//    static_cast<ChoiceWidget2*>(constraintFunction->getWidget())->resize_(size,1);
//    if(x0_ && x0_->size() != size)
//      x0_->resize_(size);
//    static_cast<FunctionWidget*>(static_cast<ChoiceWidget2*>(constraintFunction->getWidget())->getWidget())->setArg1Size(size);
  }

  DOMElement* GeneralizedAccelerationConstraintPropertyDialog::initializeUsingXML(DOMElement *parent) {
    GeneralizedDualConstraintPropertyDialog::initializeUsingXML(item->getXMLElement());
    x0->initializeUsingXML(item->getXMLElement());
    constraintFunction->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedAccelerationConstraintPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    GeneralizedDualConstraintPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    x0->writeXMLFile(item->getXMLElement(),ref);
    constraintFunction->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  JointConstraintPropertyDialog::JointConstraintPropertyDialog(JointConstraint *constraint, QWidget *parent, const Qt::WindowFlags& f) : MechanicalConstraintPropertyDialog(constraint,parent,f) {

    addTab("Kinetics",1);
    addTab("Visualisation",2);
    addTab("Initial conditions",2);

    dependentBodiesFirstSide = new ExtWidget("Dependent bodies on first side",new ListWidget(new ElementOfReferenceWidgetFactory<RigidBody>(MBSIM%"dependentRigidBodyOnFirstSide",constraint),"Body",0,2),false,false,"");
    addToTab("General",dependentBodiesFirstSide);
    connect(dependentBodiesFirstSide->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));

    dependentBodiesSecondSide = new ExtWidget("Dependent bodies on second side",new ListWidget(new ElementOfReferenceWidgetFactory<RigidBody>(MBSIM%"dependentRigidBodyOnSecondSide",constraint),"Body",0,2),false,false,"");
    addToTab("General",dependentBodiesSecondSide);
    connect(dependentBodiesSecondSide->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));

    independentBody = new ExtWidget("Independent rigid body",new ElementOfReferenceWidget<RigidBody>(constraint,nullptr),false,false,MBSIM%"independentRigidBody");
    addToTab("General", independentBody);

    connections = new ExtWidget("Connections",new ConnectElementsWidget<Frame>(2,constraint),false,false,MBSIM%"connect");
    addToTab("Kinetics", connections);

    refFrameID = new ExtWidget("Frame of reference ID",new SpinBoxWidget(1,1,2),true,false,MBSIM%"frameOfReferenceID");
    addToTab("Kinetics", refFrameID);

    force = new ExtWidget("Force direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"forceDirection");
    addToTab("Kinetics", force);

    moment = new ExtWidget("Moment direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"momentDirection");
    addToTab("Kinetics", moment);

    q0 = new ExtWidget("Initial guess",new ChoiceWidget2(new VecSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIM%"initialGuess");
    addToTab("Initial conditions", q0);
  }

  void JointConstraintPropertyDialog::updateWidget() {
//    int size = 0;
//    ListWidget *list = static_cast<ListWidget*>(dependentBodiesFirstSide->getWidget());
//    for(int i=0; i<list->getSize(); i++) {
//      RigidBody *body = static_cast<RigidBodyOfReferenceWidget*>(list->getWidget(i))->getSelectedBody();
//      if(body)
//        size += body->getqRelSize();
//    }
//    list = static_cast<ListWidget*>(dependentBodiesSecondSide->getWidget());
//    for(int i=0; i<list->getSize(); i++) {
//      RigidBody *body = static_cast<RigidBodyOfReferenceWidget*>(list->getWidget(i))->getSelectedBody();
//      if(body)
//        size += body->getqRelSize();
//    }
//    if(q0_->size() != size)
//      q0_->resize_(size);
  }

  DOMElement* JointConstraintPropertyDialog::initializeUsingXML(DOMElement *parent) {
    MechanicalConstraintPropertyDialog::initializeUsingXML(item->getXMLElement());
    dependentBodiesFirstSide->initializeUsingXML(item->getXMLElement());
    dependentBodiesSecondSide->initializeUsingXML(item->getXMLElement());
    independentBody->initializeUsingXML(item->getXMLElement());
    connections->initializeUsingXML(item->getXMLElement());
    refFrameID->initializeUsingXML(item->getXMLElement());
    force->initializeUsingXML(item->getXMLElement());
    moment->initializeUsingXML(item->getXMLElement());
    q0->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* JointConstraintPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    MechanicalConstraintPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    dependentBodiesFirstSide->writeXMLFile(item->getXMLElement(),ref);
    dependentBodiesSecondSide->writeXMLFile(item->getXMLElement(),ref);
    independentBody->writeXMLFile(item->getXMLElement(),ref);
    connections->writeXMLFile(item->getXMLElement(),ref);
    refFrameID->writeXMLFile(item->getXMLElement(),ref);
    force->writeXMLFile(item->getXMLElement(),ref);
    moment->writeXMLFile(item->getXMLElement(),ref);
    q0->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  GeneralizedConnectionConstraintPropertyDialog::GeneralizedConnectionConstraintPropertyDialog(GeneralizedConnectionConstraint *constraint, QWidget *parent, const Qt::WindowFlags& f) : GeneralizedDualConstraintPropertyDialog(constraint,parent,f) {
  }

  LinkPropertyDialog::LinkPropertyDialog(Link *link, QWidget *parent, const Qt::WindowFlags& f) : ElementPropertyDialog(link,parent,f) {
  }

  MechanicalLinkPropertyDialog::MechanicalLinkPropertyDialog(MechanicalLink *link, QWidget *parent, const Qt::WindowFlags& f) : LinkPropertyDialog(link,parent,f) {
  }

  FrameLinkPropertyDialog::FrameLinkPropertyDialog(FrameLink *link, QWidget *parent, const Qt::WindowFlags& f) : MechanicalLinkPropertyDialog(link,parent,f) {
    addTab("Kinetics",1);
    addTab("Visualisation",2);

    connections = new ExtWidget("Connections",new ConnectElementsWidget<Frame>(2,link),false,false,MBSIM%"connect");
    addToTab("Kinetics", connections);
  }

  DOMElement* FrameLinkPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ElementPropertyDialog::initializeUsingXML(item->getXMLElement());
    connections->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FrameLinkPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ElementPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    connections->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  FixedFrameLinkPropertyDialog::FixedFrameLinkPropertyDialog(FixedFrameLink *link, QWidget *parent, const Qt::WindowFlags& f) : FrameLinkPropertyDialog(link,parent,f) {
  }

  FloatingFrameLinkPropertyDialog::FloatingFrameLinkPropertyDialog(FloatingFrameLink *link, QWidget *parent, const Qt::WindowFlags& f) : FrameLinkPropertyDialog(link,parent,f) {
    refFrameID = new ExtWidget("Frame of reference ID",new SpinBoxWidget(1,1,2),true,false,MBSIM%"frameOfReferenceID");
    addToTab("Kinetics", refFrameID);
  }

  DOMElement* FloatingFrameLinkPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FrameLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    refFrameID->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FloatingFrameLinkPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FrameLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    refFrameID->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  RigidBodyLinkPropertyDialog::RigidBodyLinkPropertyDialog(RigidBodyLink *link, QWidget *parent, const Qt::WindowFlags& f) : MechanicalLinkPropertyDialog(link,parent,f) {
    addTab("Visualisation",2);

    support = new ExtWidget("Support frame",new ElementOfReferenceWidget<Frame>(link,nullptr),true,false,MBSIM%"supportFrame");
    addToTab("General",support);
  }

  DOMElement* RigidBodyLinkPropertyDialog::initializeUsingXML(DOMElement *parent) {
    MechanicalLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    support->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* RigidBodyLinkPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    MechanicalLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    support->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  DualRigidBodyLinkPropertyDialog::DualRigidBodyLinkPropertyDialog(DualRigidBodyLink *link, QWidget *parent, const Qt::WindowFlags& f) : RigidBodyLinkPropertyDialog(link,parent,f) {
    addTab("Kinetics",1);

    connections = new ExtWidget("Connections",new ChoiceWidget2(new ConnectRigidBodiesWidgetFactory(link),QBoxLayout::RightToLeft,5),false,false,MBSIM%"connect");
    addToTab("Kinetics",connections);
  }

  DOMElement* DualRigidBodyLinkPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidBodyLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    connections->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* DualRigidBodyLinkPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidBodyLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    connections->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  KineticExcitationPropertyDialog::KineticExcitationPropertyDialog(KineticExcitation *kineticExcitation, QWidget *parent, const Qt::WindowFlags& wf) : FloatingFrameLinkPropertyDialog(kineticExcitation,parent,wf) {

    static_cast<ConnectElementsWidget<Frame>*>(connections->getWidget())->setDefaultElement("../Frame[I]");

    forceDirection = new ExtWidget("Force direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"forceDirection");
    addToTab("Kinetics",forceDirection);

    forceFunction = new ExtWidget("Force function",new ChoiceWidget2(new FunctionWidgetFactory2(kineticExcitation),QBoxLayout::TopToBottom,0),true,false,MBSIM%"forceFunction");
    addToTab("Kinetics",forceFunction);

    momentDirection = new ExtWidget("Moment direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"momentDirection");
    addToTab("Kinetics",momentDirection);

    momentFunction = new ExtWidget("Moment function",new ChoiceWidget2(new FunctionWidgetFactory2(kineticExcitation),QBoxLayout::TopToBottom,0),true,false,MBSIM%"momentFunction");
    addToTab("Kinetics",momentFunction);

    arrow = new ExtWidget("Enable openMBV",new InteractionArrowMBSOMBVWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation",arrow);

    connect(forceDirection->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
    connect(forceFunction->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
    connect(momentDirection->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
    connect(momentFunction->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
  }

  void KineticExcitationPropertyDialog::updateWidget() {
    if(forceDirection->isActive()) {
      int size = static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(forceDirection->getWidget())->getWidget())->cols();
      forceFunction->resize_(size,1);
    }
    if(momentDirection->isActive()) {
      int size = static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(momentDirection->getWidget())->getWidget())->cols();
      momentFunction->resize_(size,1);
    }
  }

  DOMElement* KineticExcitationPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FloatingFrameLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    forceDirection->initializeUsingXML(item->getXMLElement());
    forceFunction->initializeUsingXML(item->getXMLElement());
    momentDirection->initializeUsingXML(item->getXMLElement());
    momentFunction->initializeUsingXML(item->getXMLElement());
    arrow->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* KineticExcitationPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FloatingFrameLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    forceDirection->writeXMLFile(item->getXMLElement(),ref);
    forceFunction->writeXMLFile(item->getXMLElement(),ref);
    momentDirection->writeXMLFile(item->getXMLElement(),ref);
    momentFunction->writeXMLFile(item->getXMLElement(),ref);
    arrow->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  SpringDamperPropertyDialog::SpringDamperPropertyDialog(SpringDamper *springDamper, QWidget *parent, const Qt::WindowFlags& f) : FixedFrameLinkPropertyDialog(springDamper,parent,f) {

    forceFunction = new ExtWidget("Force function",new ChoiceWidget2(new SpringDamperWidgetFactory(springDamper),QBoxLayout::TopToBottom,0),false,false,MBSIM%"forceFunction");
    addToTab("Kinetics", forceFunction);

    unloadedLength = new ExtWidget("Unloaded length",new ChoiceWidget2(new ScalarWidgetFactory("1"),QBoxLayout::RightToLeft,5),false,false,MBSIM%"unloadedLength");
    addToTab("General",unloadedLength);

    coilSpring = new ExtWidget("Enable openMBV",new CoilSpringMBSOMBVWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", coilSpring);
  }

  DOMElement* SpringDamperPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FixedFrameLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    forceFunction->initializeUsingXML(item->getXMLElement());
    unloadedLength->initializeUsingXML(item->getXMLElement());
    coilSpring->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* SpringDamperPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FixedFrameLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    forceFunction->writeXMLFile(item->getXMLElement(),ref);
    unloadedLength->writeXMLFile(item->getXMLElement(),ref);
    coilSpring->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  DirectionalSpringDamperPropertyDialog::DirectionalSpringDamperPropertyDialog(DirectionalSpringDamper *springDamper, QWidget *parent, const Qt::WindowFlags& f) : FloatingFrameLinkPropertyDialog(springDamper,parent,f) {

    forceDirection = new ExtWidget("Force direction",new ChoiceWidget2(new VecWidgetFactory(3),QBoxLayout::RightToLeft,5),true,false,MBSIM%"forceDirection");
    addToTab("Kinetics",forceDirection);

    forceFunction = new ExtWidget("Force function",new ChoiceWidget2(new SpringDamperWidgetFactory(springDamper),QBoxLayout::TopToBottom,0),false,false,MBSIM%"forceFunction");
    addToTab("Kinetics", forceFunction);

    unloadedLength = new ExtWidget("Unloaded length",new ChoiceWidget2(new ScalarWidgetFactory("1"),QBoxLayout::RightToLeft,5),false,false,MBSIM%"unloadedLength");
    addToTab("General",unloadedLength);

    coilSpring = new ExtWidget("Enable openMBV",new CoilSpringMBSOMBVWidget,true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", coilSpring);
  }

  DOMElement* DirectionalSpringDamperPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FloatingFrameLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    forceDirection->initializeUsingXML(item->getXMLElement());
    forceFunction->initializeUsingXML(item->getXMLElement());
    unloadedLength->initializeUsingXML(item->getXMLElement());
    coilSpring->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* DirectionalSpringDamperPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FloatingFrameLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    forceDirection->writeXMLFile(item->getXMLElement(),ref);
    forceFunction->writeXMLFile(item->getXMLElement(),ref);
    unloadedLength->writeXMLFile(item->getXMLElement(),ref);
    coilSpring->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  IsotropicRotationalSpringDamperPropertyDialog::IsotropicRotationalSpringDamperPropertyDialog(IsotropicRotationalSpringDamper *springDamper, QWidget *parent, const Qt::WindowFlags& f) : FixedFrameLinkPropertyDialog(springDamper,parent,f) {

    elasticMomentFunction = new ExtWidget("Elastic moment function",new ChoiceWidget2(new FunctionWidgetFactory2(springDamper),QBoxLayout::TopToBottom,0),false,false,MBSIM%"elasticMomentFunction");
    addToTab("Kinetics", elasticMomentFunction);

    dissipativeMomentFunction = new ExtWidget("Dissipative moment function",new ChoiceWidget2(new FunctionWidgetFactory2(springDamper),QBoxLayout::TopToBottom,0),false,false,MBSIM%"dissipativeMomentFunction");
    addToTab("Kinetics", dissipativeMomentFunction);
  }

  DOMElement* IsotropicRotationalSpringDamperPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FixedFrameLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    elasticMomentFunction->initializeUsingXML(item->getXMLElement());
    dissipativeMomentFunction->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* IsotropicRotationalSpringDamperPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FixedFrameLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    elasticMomentFunction->writeXMLFile(item->getXMLElement(),ref);
    dissipativeMomentFunction->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  JointPropertyDialog::JointPropertyDialog(Joint *joint, QWidget *parent, const Qt::WindowFlags& f) : FloatingFrameLinkPropertyDialog(joint,parent,f) {

    addTab("Extra");

    forceDirection = new ExtWidget("Force direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"forceDirection");
    addToTab("Kinetics",forceDirection);

    forceLaw = new ExtWidget("Force law",new ChoiceWidget2(new GeneralizedForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"forceLaw");
    addToTab("Kinetics",forceLaw);

    momentDirection = new ExtWidget("Moment direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"momentDirection");
    addToTab("Kinetics",momentDirection);

    momentLaw = new ExtWidget("Moment law",new ChoiceWidget2(new GeneralizedForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"momentLaw");
    addToTab("Kinetics",momentLaw);

    integrate = new ExtWidget("Integrate generalized relative velocity of rotation",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"integrateGeneralizedRelativeVelocityOfRotation");
    addToTab("Extra",integrate);
  }

  DOMElement* JointPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FloatingFrameLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    forceDirection->initializeUsingXML(item->getXMLElement());
    forceLaw->initializeUsingXML(item->getXMLElement());
    momentDirection->initializeUsingXML(item->getXMLElement());
    momentLaw->initializeUsingXML(item->getXMLElement());
    integrate->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* JointPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FloatingFrameLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    forceDirection->writeXMLFile(item->getXMLElement(),ref);
    forceLaw->writeXMLFile(item->getXMLElement(),ref);
    momentDirection->writeXMLFile(item->getXMLElement(),ref);
    momentLaw->writeXMLFile(item->getXMLElement(),ref);
    integrate->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  ElasticJointPropertyDialog::ElasticJointPropertyDialog(ElasticJoint *joint, QWidget *parent, const Qt::WindowFlags& f) : FloatingFrameLinkPropertyDialog(joint,parent,f) {

    addTab("Extra");

    forceDirection = new ExtWidget("Force direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"forceDirection");
    addToTab("Kinetics", forceDirection);

    momentDirection = new ExtWidget("Moment direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"momentDirection");
    addToTab("Kinetics", momentDirection);

    function = new ExtWidget("Generalized force function",new ChoiceWidget2(new SpringDamperWidgetFactory(joint),QBoxLayout::TopToBottom,0),true,false,MBSIM%"generalizedForceFunction");
    addToTab("Kinetics", function);

    integrate = new ExtWidget("Integrate generalized relative velocity of rotation",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"integrateGeneralizedRelativeVelocityOfRotation");
    addToTab("Extra", integrate);

    connect(forceDirection->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
    connect(momentDirection->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
    connect(function->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
  }

  void ElasticJointPropertyDialog::updateWidget() {
    int size = 0;
    if(forceDirection->isActive())
      size += static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(forceDirection->getWidget())->getWidget())->cols();
    if(momentDirection->isActive())
      size += static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(momentDirection->getWidget())->getWidget())->cols();
    function->resize_(size,1);
  }

  DOMElement* ElasticJointPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FloatingFrameLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    forceDirection->initializeUsingXML(item->getXMLElement());
    momentDirection->initializeUsingXML(item->getXMLElement());
    function->initializeUsingXML(item->getXMLElement());
    integrate->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* ElasticJointPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FloatingFrameLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    forceDirection->writeXMLFile(item->getXMLElement(),ref);
    momentDirection->writeXMLFile(item->getXMLElement(),ref);
    function->writeXMLFile(item->getXMLElement(),ref);
    integrate->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  GeneralizedSpringDamperPropertyDialog::GeneralizedSpringDamperPropertyDialog(DualRigidBodyLink *springDamper, QWidget *parent, const Qt::WindowFlags& f) : DualRigidBodyLinkPropertyDialog(springDamper,parent,f) {

    function = new ExtWidget("Generalized force function",new ChoiceWidget2(new SpringDamperWidgetFactory(springDamper),QBoxLayout::TopToBottom,0),false,false,MBSIM%"generalizedForceFunction");
    addToTab("Kinetics", function);

    unloadedLength = new ExtWidget("Generalized Unloaded length",new ChoiceWidget2(new ScalarWidgetFactory("0"),QBoxLayout::RightToLeft,5),false,false,MBSIM%"generalizedUnloadedLength");
    addToTab("General",unloadedLength);
  }

  DOMElement* GeneralizedSpringDamperPropertyDialog::initializeUsingXML(DOMElement *parent) {
    DualRigidBodyLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    function->initializeUsingXML(item->getXMLElement());
    unloadedLength->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedSpringDamperPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    DualRigidBodyLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    function->writeXMLFile(item->getXMLElement(),ref);
    unloadedLength->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  GeneralizedFrictionPropertyDialog::GeneralizedFrictionPropertyDialog(DualRigidBodyLink *friction, QWidget *parent, const Qt::WindowFlags& f) : DualRigidBodyLinkPropertyDialog(friction,parent,f) {

    function = new ExtWidget("Generalized friction force law",new ChoiceWidget2(new FrictionForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"generalizedFrictionForceLaw");
    addToTab("Kinetics", function);

    normalForce = new ExtWidget("Generalized normal force function",new ChoiceWidget2(new FunctionWidgetFactory2(friction),QBoxLayout::TopToBottom,0),true,false,MBSIM%"generalizedNormalForceFunction");
    addToTab("Kinetics",normalForce);
  }

  DOMElement* GeneralizedFrictionPropertyDialog::initializeUsingXML(DOMElement *parent) {
    DualRigidBodyLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    function->initializeUsingXML(item->getXMLElement());
    normalForce->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedFrictionPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    DualRigidBodyLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    function->writeXMLFile(item->getXMLElement(),ref);
    normalForce->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }


  GeneralizedGearPropertyDialog::GeneralizedGearPropertyDialog(RigidBodyLink *link, QWidget *parent, const Qt::WindowFlags& f) : RigidBodyLinkPropertyDialog(link,parent,f) {
    addTab("Kinetics",1);
    addTab("Visualisation",2);

    gearOutput = new ExtWidget("Gear output",new ElementOfReferenceWidget<RigidBody>(link,nullptr),false,false,MBSIM%"gearOutput");
    addToTab("General",gearOutput);

    gearInput = new ExtWidget("Gear inputs",new ListWidget(new ElementOfReferenceWidgetFactory<RigidBody>(MBSIM%"gearInput",link,true),"Gear input",0,2),false,false,"");
    addToTab("General",gearInput);

    function = new ExtWidget("Generalized force law",new ChoiceWidget2(new GeneralizedForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"generalizedForceLaw");
    addToTab("Kinetics",function);
  }

  DOMElement* GeneralizedGearPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidBodyLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    gearOutput->initializeUsingXML(item->getXMLElement());
    gearInput->initializeUsingXML(item->getXMLElement());
    function->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedGearPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidBodyLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    gearOutput->writeXMLFile(item->getXMLElement(),ref);
    gearInput->writeXMLFile(item->getXMLElement(),ref);
    function->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  GeneralizedElasticConnectionPropertyDialog::GeneralizedElasticConnectionPropertyDialog(DualRigidBodyLink *connection, QWidget *parent, const Qt::WindowFlags& f) : DualRigidBodyLinkPropertyDialog(connection,parent,f) {

    function = new ExtWidget("Generalized force function",new ChoiceWidget2(new SpringDamperWidgetFactory(connection,true),QBoxLayout::TopToBottom,0),false,false,MBSIM%"generalizedForceFunction");
    addToTab("Kinetics", function);

    connect(function,SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
    connect(connections->getWidget(),SIGNAL(widgetChanged()),this,SLOT(updateWidget()));
  }

  void GeneralizedElasticConnectionPropertyDialog::updateWidget() {
  }

  DOMElement* GeneralizedElasticConnectionPropertyDialog::initializeUsingXML(DOMElement *parent) {
    DualRigidBodyLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    function->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedElasticConnectionPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    DualRigidBodyLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    function->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  GeneralizedElasticStructurePropertyDialog::GeneralizedElasticStructurePropertyDialog(RigidBodyLink *link, QWidget *parent, const Qt::WindowFlags& f) : RigidBodyLinkPropertyDialog(link,parent,f) {
    addTab("Kinetics",1);

    rigidBody = new ExtWidget("Rigid bodies",new ListWidget(new ElementOfReferenceWidgetFactory<RigidBody>(MBSIM%"rigidBody",link),"Rigid body",0,2),false,false,"");
    addToTab("General",rigidBody);

    function = new ExtWidget("Generalized force function",new ChoiceWidget2(new SpringDamperWidgetFactory(link,true),QBoxLayout::TopToBottom,0),false,false,MBSIM%"generalizedForceFunction");
    addToTab("Kinetics", function);
  }

  DOMElement* GeneralizedElasticStructurePropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidBodyLinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    rigidBody->initializeUsingXML(item->getXMLElement());
    function->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedElasticStructurePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidBodyLinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    rigidBody->writeXMLFile(item->getXMLElement(),ref);
    function->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  ContactPropertyDialog::ContactPropertyDialog(Contact *contact, QWidget *parent, const Qt::WindowFlags& f) : LinkPropertyDialog(contact,parent,f) {

    addTab("Kinetics",1);
    addTab("Extra");

    //connections = new ExtWidget("Connections",new ConnectContoursWidget(2,contact),false,false,MBSIM%"connect");
    connections = new ExtWidget("Connections",new ConnectElementsWidget<Contour>(2,contact),false,false,MBSIM%"connect");
    addToTab("Kinetics", connections);

    contactForceLaw = new ExtWidget("Normal force law",new ChoiceWidget2(new GeneralizedForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"normalForceLaw");
    addToTab("Kinetics", contactForceLaw);

    contactImpactLaw = new ExtWidget("Normal impact law",new ChoiceWidget2(new GeneralizedImpactLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"normalImpactLaw");
    addToTab("Kinetics", contactImpactLaw);

    frictionForceLaw = new ExtWidget("Tangential force law",new ChoiceWidget2(new FrictionForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"tangentialForceLaw");
    addToTab("Kinetics", frictionForceLaw);

    frictionImpactLaw = new ExtWidget("Tangential impact law",new ChoiceWidget2(new FrictionImpactLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"tangentialImpactLaw");
    addToTab("Kinetics", frictionImpactLaw);

    searchAllContactPoints = new ExtWidget("Search all contact points",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"searchAllContactPoints");
    addToTab("Extra", searchAllContactPoints);

    initialGuess = new ExtWidget("Initial guess",new ChoiceWidget2(new MatRowsColsVarWidgetFactory(0,0),QBoxLayout::RightToLeft,5),true,false,MBSIM%"initialGuess");
    addToTab("Extra", initialGuess);

    tolerance = new ExtWidget("Tolerance",new ChoiceWidget2(new ScalarWidgetFactory("1e-10"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"tolerance");
    addToTab("Extra", tolerance);

    maxNumContacts = new ExtWidget("Maximum number of contacts",new ChoiceWidget2(new ScalarWidgetFactory("1"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"maximumNumberOfContacts");
    addToTab("Extra", maxNumContacts);
  }

  DOMElement* ContactPropertyDialog::initializeUsingXML(DOMElement *parent) {
    LinkPropertyDialog::initializeUsingXML(item->getXMLElement());
    connections->initializeUsingXML(item->getXMLElement());
    contactForceLaw->initializeUsingXML(item->getXMLElement());
    contactImpactLaw->initializeUsingXML(item->getXMLElement());
    frictionForceLaw->initializeUsingXML(item->getXMLElement());
    frictionImpactLaw->initializeUsingXML(item->getXMLElement());
    searchAllContactPoints->initializeUsingXML(item->getXMLElement());
    initialGuess->initializeUsingXML(item->getXMLElement());
    tolerance->initializeUsingXML(item->getXMLElement());
    maxNumContacts->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* ContactPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    LinkPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    connections->writeXMLFile(item->getXMLElement(),ref);
    contactForceLaw->writeXMLFile(item->getXMLElement(),ref);
    contactImpactLaw->writeXMLFile(item->getXMLElement(),ref);
    frictionForceLaw->writeXMLFile(item->getXMLElement(),ref);
    frictionImpactLaw->writeXMLFile(item->getXMLElement(),ref);
    searchAllContactPoints->writeXMLFile(item->getXMLElement(),ref);
    initialGuess->writeXMLFile(item->getXMLElement(),ref);
    tolerance->writeXMLFile(item->getXMLElement(),ref);
    maxNumContacts->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  ObserverPropertyDialog::ObserverPropertyDialog(Observer *observer, QWidget * parent, const Qt::WindowFlags& f) : ElementPropertyDialog(observer,parent,f) {
  }

  KinematicCoordinatesObserverPropertyDialog::KinematicCoordinatesObserverPropertyDialog(KinematicCoordinatesObserver *observer, QWidget *parent, const Qt::WindowFlags& f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    frame = new ExtWidget("Frame",new ElementOfReferenceWidget<Frame>(observer,nullptr),false,false,MBSIM%"frame");
    addToTab("General", frame);

    frameOfReference = new ExtWidget("Frame of reference",new ElementOfReferenceWidget<Frame>(observer,nullptr),true,false,MBSIM%"frameOfReference");
    addToTab("General", frameOfReference);

    position = new ExtWidget("Enable openMBV position",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVPosition");
    addToTab("Visualisation",position);

    velocity = new ExtWidget("Enable openMBV velocity",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVVelocity");
    addToTab("Visualisation",velocity);

    acceleration = new ExtWidget("Enable openMBV acceleration",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVAcceleration");
    addToTab("Visualisation",acceleration);
  }

  DOMElement* KinematicCoordinatesObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(item->getXMLElement());
    frame->initializeUsingXML(item->getXMLElement());
    frameOfReference->initializeUsingXML(item->getXMLElement());
    position->initializeUsingXML(item->getXMLElement());
    velocity->initializeUsingXML(item->getXMLElement());
    acceleration->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* KinematicCoordinatesObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    frame->writeXMLFile(item->getXMLElement(),ref);
    frameOfReference->writeXMLFile(item->getXMLElement(),ref);
    position->writeXMLFile(item->getXMLElement(),ref);
    velocity->writeXMLFile(item->getXMLElement(),ref);
    acceleration->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  RelativeKinematicsObserverPropertyDialog::RelativeKinematicsObserverPropertyDialog(RelativeKinematicsObserver *observer, QWidget *parent, const Qt::WindowFlags& f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    frame = new ExtWidget("Frame",new ElementOfReferenceWidget<Frame>(observer,nullptr),false,false,MBSIM%"frame");
    addToTab("General", frame);

    refFrame = new ExtWidget("Frame of reference",new ElementOfReferenceWidget<Frame>(observer,nullptr),true,false,MBSIM%"frameOfReference");
    addToTab("General", refFrame);

    position = new ExtWidget("Enable openMBV position",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVPosition");
    addToTab("Visualisation",position);

    velocity = new ExtWidget("Enable openMBV velocity",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVVelocity");
    addToTab("Visualisation",velocity);

    angularVelocity = new ExtWidget("Enable openMBV angular velocity",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVAngularVelocity");
    addToTab("Visualisation",angularVelocity);

    acceleration = new ExtWidget("Enable openMBV acceleration",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVAcceleration");
    addToTab("Visualisation",acceleration);

    angularAcceleration = new ExtWidget("Enable openMBV angular acceleration",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVAngularAcceleration");
    addToTab("Visualisation",angularAcceleration);
  }

  DOMElement* RelativeKinematicsObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(item->getXMLElement());
    frame->initializeUsingXML(item->getXMLElement());
    refFrame->initializeUsingXML(item->getXMLElement());
    position->initializeUsingXML(item->getXMLElement());
    velocity->initializeUsingXML(item->getXMLElement());
    angularVelocity->initializeUsingXML(item->getXMLElement());
    acceleration->initializeUsingXML(item->getXMLElement());
    angularAcceleration->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* RelativeKinematicsObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    frame->writeXMLFile(item->getXMLElement(),ref);
    refFrame->writeXMLFile(item->getXMLElement(),ref);
    position->writeXMLFile(item->getXMLElement(),ref);
    velocity->writeXMLFile(item->getXMLElement(),ref);
    angularVelocity->writeXMLFile(item->getXMLElement(),ref);
    acceleration->writeXMLFile(item->getXMLElement(),ref);
    angularAcceleration->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  MechanicalLinkObserverPropertyDialog::MechanicalLinkObserverPropertyDialog(MechanicalLinkObserver *observer, QWidget *parent, const Qt::WindowFlags& f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    link = new ExtWidget("Mechanical link",new ElementOfReferenceWidget<Link>(observer,nullptr),false,false,MBSIM%"mechanicalLink");
    addToTab("General", link);

    forceArrow = new ExtWidget("Enable openMBV force",new InteractionArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVForce");
    addToTab("Visualisation",forceArrow);

    momentArrow = new ExtWidget("Enable openMBV moment",new InteractionArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVMoment");
    addToTab("Visualisation",momentArrow);
  }

  DOMElement* MechanicalLinkObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(item->getXMLElement());
    link->initializeUsingXML(item->getXMLElement());
    forceArrow->initializeUsingXML(item->getXMLElement());
    momentArrow->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* MechanicalLinkObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    link->writeXMLFile(item->getXMLElement(),ref);
    forceArrow->writeXMLFile(item->getXMLElement(),ref);
    momentArrow->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  MechanicalConstraintObserverPropertyDialog::MechanicalConstraintObserverPropertyDialog(MechanicalConstraintObserver *observer, QWidget *parent, const Qt::WindowFlags& f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    constraint = new ExtWidget("Mechanical constraint",new ElementOfReferenceWidget<Constraint>(observer,nullptr),false,false,MBSIM%"mechanicalConstraint");
    addToTab("General", constraint);

    forceArrow = new ExtWidget("Enable openMBV force",new InteractionArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVForce");
    addToTab("Visualisation",forceArrow);

    momentArrow = new ExtWidget("Enable openMBV moment",new InteractionArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVMoment");
    addToTab("Visualisation",momentArrow);
  }

  DOMElement* MechanicalConstraintObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(item->getXMLElement());
    constraint->initializeUsingXML(item->getXMLElement());
    forceArrow->initializeUsingXML(item->getXMLElement());
    momentArrow->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* MechanicalConstraintObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    constraint->writeXMLFile(item->getXMLElement(),ref);
    forceArrow->writeXMLFile(item->getXMLElement(),ref);
    momentArrow->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  ContactObserverPropertyDialog::ContactObserverPropertyDialog(ContactObserver *observer, QWidget *parent, const Qt::WindowFlags& f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    link = new ExtWidget("Mechanical link",new ElementOfReferenceWidget<Link>(observer,nullptr),false,false,MBSIM%"contact");
    addToTab("General", link);

    forceArrow = new ExtWidget("Enable openMBV force",new InteractionArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVForce");
    addToTab("Visualisation",forceArrow);

    momentArrow = new ExtWidget("Enable openMBV force",new InteractionArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVMoment");
    addToTab("Visualisation",momentArrow);

    contactPoints = new ExtWidget("Enable openMBV contact points",new FrameMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVContactPoints");
    addToTab("Visualisation",contactPoints);

    normalForceArrow = new ExtWidget("Enable openMBV normal force",new InteractionArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVNormalForce");
    addToTab("Visualisation",normalForceArrow);

    frictionArrow = new ExtWidget("Enable openMBV tangential force",new FrictionArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVTangentialForce");
    addToTab("Visualisation",frictionArrow);
  }

  DOMElement* ContactObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(item->getXMLElement());
    link->initializeUsingXML(item->getXMLElement());
    forceArrow->initializeUsingXML(item->getXMLElement());
    momentArrow->initializeUsingXML(item->getXMLElement());
    contactPoints->initializeUsingXML(item->getXMLElement());
    normalForceArrow->initializeUsingXML(item->getXMLElement());
    frictionArrow->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* ContactObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    link->writeXMLFile(item->getXMLElement(),ref);
    forceArrow->writeXMLFile(item->getXMLElement(),ref);
    momentArrow->writeXMLFile(item->getXMLElement(),ref);
    contactPoints->writeXMLFile(item->getXMLElement(),ref);
    normalForceArrow->writeXMLFile(item->getXMLElement(),ref);
    frictionArrow->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  FrameObserverPropertyDialog::FrameObserverPropertyDialog(FrameObserver *observer, QWidget *parent, const Qt::WindowFlags& f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    frame = new ExtWidget("Frame",new ElementOfReferenceWidget<Frame>(observer,nullptr),false,false,MBSIM%"frame");
    addToTab("General", frame);

    position = new ExtWidget("Enable openMBV position",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVPosition");
    addToTab("Visualisation",position);

    velocity = new ExtWidget("Enable openMBV velocity",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVVelocity");
    addToTab("Visualisation",velocity);

    angularVelocity = new ExtWidget("Enable openMBV angular velocity",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVAngularVelocity");
    addToTab("Visualisation",angularVelocity);

    acceleration = new ExtWidget("Enable openMBV acceleration",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVAcceleration");
    addToTab("Visualisation",acceleration);

    angularAcceleration = new ExtWidget("Enable openMBV angular acceleration",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVAngularAcceleration");
    addToTab("Visualisation",angularAcceleration);
  }

  DOMElement* FrameObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(item->getXMLElement());
    frame->initializeUsingXML(item->getXMLElement());
    position->initializeUsingXML(item->getXMLElement());
    velocity->initializeUsingXML(item->getXMLElement());
    angularVelocity->initializeUsingXML(item->getXMLElement());
    acceleration->initializeUsingXML(item->getXMLElement());
    angularAcceleration->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FrameObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    frame->writeXMLFile(item->getXMLElement(),ref);
    position->writeXMLFile(item->getXMLElement(),ref);
    velocity->writeXMLFile(item->getXMLElement(),ref);
    angularVelocity->writeXMLFile(item->getXMLElement(),ref);
    acceleration->writeXMLFile(item->getXMLElement(),ref);
    angularAcceleration->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  RigidBodyObserverPropertyDialog::RigidBodyObserverPropertyDialog(RigidBodyObserver *observer, QWidget *parent, const Qt::WindowFlags& f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    body = new ExtWidget("Rigid body",new ElementOfReferenceWidget<RigidBody>(observer,nullptr),false,false,MBSIM%"rigidBody");
    addToTab("General", body);

    frameOfReference = new ExtWidget("Frame of reference",new ElementOfReferenceWidget<Frame>(observer,nullptr),true,false,MBSIM%"frameOfReference");
    addToTab("General", frameOfReference);

    weight = new ExtWidget("Enable openMBV weight",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVWeight");
    addToTab("Visualisation",weight);

    jointForce = new ExtWidget("Enable openMBV joint force",new InteractionArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVJointForce");
    addToTab("Visualisation",jointForce);

    jointMoment = new ExtWidget("Enable openMBV joint moment",new InteractionArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVJointMoment");
    addToTab("Visualisation",jointMoment);

    axisOfRotation = new ExtWidget("Enable openMBV axis of rotation",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVAxisOfRotation");
    addToTab("Visualisation",axisOfRotation);

    momentum = new ExtWidget("Enable openMBV momentum",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVMomentum");
    addToTab("Visualisation",momentum);

    angularMomentum = new ExtWidget("Enable openMBV angular momentum",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVAngularMomentum");
    addToTab("Visualisation",angularMomentum);

    derivativeOfMomentum = new ExtWidget("Enable openMBV derivative of momentum",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVDerivativeOfMomentum");
    addToTab("Visualisation",derivativeOfMomentum);

    derivativeOfAngularMomentum = new ExtWidget("Enable openMBV derivative of angular momentum",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVDerivativeOfAngularMomentum");
    addToTab("Visualisation",derivativeOfAngularMomentum);
  }

  DOMElement* RigidBodyObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(item->getXMLElement());
    body->initializeUsingXML(item->getXMLElement());
    frameOfReference->initializeUsingXML(item->getXMLElement());
    weight->initializeUsingXML(item->getXMLElement());
    jointForce->initializeUsingXML(item->getXMLElement());
    jointMoment->initializeUsingXML(item->getXMLElement());
    axisOfRotation->initializeUsingXML(item->getXMLElement());
    momentum->initializeUsingXML(item->getXMLElement());
    angularMomentum->initializeUsingXML(item->getXMLElement());
    derivativeOfMomentum->initializeUsingXML(item->getXMLElement());
    derivativeOfAngularMomentum->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* RigidBodyObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    body->writeXMLFile(item->getXMLElement(),ref);
    frameOfReference->writeXMLFile(item->getXMLElement(),ref);
    weight->writeXMLFile(item->getXMLElement(),ref);
    jointForce->writeXMLFile(item->getXMLElement(),ref);
    jointMoment->writeXMLFile(item->getXMLElement(),ref);
    axisOfRotation->writeXMLFile(item->getXMLElement(),ref);
    momentum->writeXMLFile(item->getXMLElement(),ref);
    angularMomentum->writeXMLFile(item->getXMLElement(),ref);
    derivativeOfMomentum->writeXMLFile(item->getXMLElement(),ref);
    derivativeOfAngularMomentum->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  RigidBodySystemObserverPropertyDialog::RigidBodySystemObserverPropertyDialog(RigidBodySystemObserver *observer, QWidget *parent, const Qt::WindowFlags& f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    bodies = new ExtWidget("Rigid bodies",new ListWidget(new ElementOfReferenceWidgetFactory<RigidBody>(MBSIM%"rigidBody",observer),"Rigid body",0,2),false,false,"");
    addToTab("General", bodies);

    frameOfReference = new ExtWidget("Frame of reference",new ElementOfReferenceWidget<Frame>(observer,nullptr),true,false,MBSIM%"frameOfReference");
    addToTab("General", frameOfReference);

    position = new ExtWidget("Enable openMBV position",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVPosition");
    addToTab("Visualisation",position);

    velocity = new ExtWidget("Enable openMBV velocity",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVVelocity");
    addToTab("Visualisation",velocity);

    acceleration = new ExtWidget("Enable openMBV acceleration",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVAcceleration");
    addToTab("Visualisation",acceleration);

    weight = new ExtWidget("Enable openMBV weight",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVWeight");
    addToTab("Visualisation",weight);

    momentum = new ExtWidget("Enable openMBV momentum",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVMomentum");
    addToTab("Visualisation",momentum);

    angularMomentum = new ExtWidget("Enable openMBV angular momentum",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVAngularMomentum");
    addToTab("Visualisation",angularMomentum);

    derivativeOfMomentum = new ExtWidget("Enable openMBV derivative of momentum",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVDerivativeOfMomentum");
    addToTab("Visualisation",derivativeOfMomentum);

    derivativeOfAngularMomentum = new ExtWidget("Enable openMBV derivative of angular momentum",new ArrowMBSOMBVWidget,true,false,MBSIM%"enableOpenMBVDerivativeOfAngularMomentum");
    addToTab("Visualisation",derivativeOfAngularMomentum);

  }

  DOMElement* RigidBodySystemObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(item->getXMLElement());
    bodies->initializeUsingXML(item->getXMLElement());
    frameOfReference->initializeUsingXML(item->getXMLElement());
    position->initializeUsingXML(item->getXMLElement());
    velocity->initializeUsingXML(item->getXMLElement());
    acceleration->initializeUsingXML(item->getXMLElement());
    weight->initializeUsingXML(item->getXMLElement());
    momentum->initializeUsingXML(item->getXMLElement());
    angularMomentum->initializeUsingXML(item->getXMLElement());
    derivativeOfMomentum->initializeUsingXML(item->getXMLElement());
    derivativeOfAngularMomentum->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* RigidBodySystemObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    bodies->writeXMLFile(item->getXMLElement(),ref);
    frameOfReference->writeXMLFile(item->getXMLElement(),ref);
    position->writeXMLFile(item->getXMLElement(),ref);
    velocity->writeXMLFile(item->getXMLElement(),ref);
    acceleration->writeXMLFile(item->getXMLElement(),ref);
    weight->writeXMLFile(item->getXMLElement(),ref);
    momentum->writeXMLFile(item->getXMLElement(),ref);
    angularMomentum->writeXMLFile(item->getXMLElement(),ref);
    derivativeOfMomentum->writeXMLFile(item->getXMLElement(),ref);
    derivativeOfAngularMomentum->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  SignalPropertyDialog::SignalPropertyDialog(Signal *signal, QWidget * parent, const Qt::WindowFlags& f) : LinkPropertyDialog(signal,parent,f) {
  }

  SensorPropertyDialog::SensorPropertyDialog(Sensor *sensor, QWidget * parent, const Qt::WindowFlags& f) : SignalPropertyDialog(sensor,parent,f) {
  }

  ObjectSensorPropertyDialog::ObjectSensorPropertyDialog(ObjectSensor *sensor, QWidget * parent, const Qt::WindowFlags& f) : SensorPropertyDialog(sensor,parent,f) {
    object = new ExtWidget("Object of reference",new ElementOfReferenceWidget<Object>(sensor,nullptr),false,false,MBSIMCONTROL%"object");
    addToTab("General", object);
  }

  DOMElement* ObjectSensorPropertyDialog::initializeUsingXML(DOMElement *parent) {
    SignalPropertyDialog::initializeUsingXML(item->getXMLElement());
    object->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* ObjectSensorPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    SignalPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    object->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  GeneralizedPositionSensorPropertyDialog::GeneralizedPositionSensorPropertyDialog(GeneralizedPositionSensor *sensor, QWidget * parent, const Qt::WindowFlags& f) : ObjectSensorPropertyDialog(sensor,parent,f) {
  }

  GeneralizedVelocitySensorPropertyDialog::GeneralizedVelocitySensorPropertyDialog(GeneralizedVelocitySensor *sensor, QWidget * parent, const Qt::WindowFlags& f) : ObjectSensorPropertyDialog(sensor,parent,f) {
  }

  FrameSensorPropertyDialog::FrameSensorPropertyDialog(FrameSensor *sensor, QWidget * parent, const Qt::WindowFlags& f) : SensorPropertyDialog(sensor,parent,f) {
    frame = new ExtWidget("Frame of reference",new ElementOfReferenceWidget<Frame>(sensor,nullptr),false,false,MBSIMCONTROL%"frame");
    addToTab("General", frame);
  }

  DOMElement* FrameSensorPropertyDialog::initializeUsingXML(DOMElement *parent) {
    SensorPropertyDialog::initializeUsingXML(item->getXMLElement());
    frame->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FrameSensorPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    SensorPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    frame->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  PositionSensorPropertyDialog::PositionSensorPropertyDialog(PositionSensor *sensor, QWidget * parent, const Qt::WindowFlags& f) : FrameSensorPropertyDialog(sensor,parent,f) {
  }

  OrientationSensorPropertyDialog::OrientationSensorPropertyDialog(OrientationSensor *sensor, QWidget * parent, const Qt::WindowFlags& f) : FrameSensorPropertyDialog(sensor,parent,f) {
  }

  VelocitySensorPropertyDialog::VelocitySensorPropertyDialog(VelocitySensor *sensor, QWidget * parent, const Qt::WindowFlags& f) : FrameSensorPropertyDialog(sensor,parent,f) {
  }

  AngularVelocitySensorPropertyDialog::AngularVelocitySensorPropertyDialog(AngularVelocitySensor *sensor, QWidget * parent, const Qt::WindowFlags& f) : FrameSensorPropertyDialog(sensor,parent,f) {
  }

  FunctionSensorPropertyDialog::FunctionSensorPropertyDialog(FunctionSensor *sensor, QWidget * parent, const Qt::WindowFlags& f) : SensorPropertyDialog(sensor,parent,f) {
    function = new ExtWidget("Function",new ChoiceWidget2(new FunctionWidgetFactory2(sensor,false),QBoxLayout::TopToBottom,0),false,false,MBSIMCONTROL%"function");
    addToTab("General", function);
  }

  DOMElement* FunctionSensorPropertyDialog::initializeUsingXML(DOMElement *parent) {
    SensorPropertyDialog::initializeUsingXML(item->getXMLElement());
    function->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* FunctionSensorPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    SensorPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    function->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  MultiplexerPropertyDialog::MultiplexerPropertyDialog(Multiplexer *signal, QWidget * parent, const Qt::WindowFlags& f) : SignalPropertyDialog(signal,parent,f) {
    inputSignal = new ExtWidget("Input signal",new ListWidget(new ElementOfReferenceWidgetFactory<Signal>(MBSIMCONTROL%"inputSignal",signal),"Signal",0,2),false,false,"");
    addToTab("General", inputSignal);
  }

  DOMElement* MultiplexerPropertyDialog::initializeUsingXML(DOMElement *parent) {
    SignalPropertyDialog::initializeUsingXML(item->getXMLElement());
    inputSignal->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* MultiplexerPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    SignalPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    inputSignal->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  DemultiplexerPropertyDialog::DemultiplexerPropertyDialog(Demultiplexer *signal, QWidget * parent, const Qt::WindowFlags& f) : SignalPropertyDialog(signal,parent,f) {
    inputSignal = new ExtWidget("Input signal",new ElementOfReferenceWidget<Signal>(signal,nullptr),false,false,MBSIMCONTROL%"inputSignal");
    addToTab("General", inputSignal);
    indices = new ExtWidget("Indices",new ChoiceWidget2(new VecSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIMCONTROL%"indices");
    addToTab("General", indices);
  }

  DOMElement* DemultiplexerPropertyDialog::initializeUsingXML(DOMElement *parent) {
    SignalPropertyDialog::initializeUsingXML(item->getXMLElement());
    inputSignal->initializeUsingXML(item->getXMLElement());
    indices->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* DemultiplexerPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    SignalPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    inputSignal->writeXMLFile(item->getXMLElement(),ref);
    indices->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  LinearTransferSystemPropertyDialog::LinearTransferSystemPropertyDialog(LinearTransferSystem *signal, QWidget * parent, const Qt::WindowFlags& f) : SignalPropertyDialog(signal,parent,f) {
    inputSignal = new ExtWidget("Input signal",new ElementOfReferenceWidget<Signal>(signal,nullptr),false,false,MBSIMCONTROL%"inputSignal");
    addToTab("General", inputSignal);

    A = new ExtWidget("System matrix",new ChoiceWidget2(new SqrMatSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIMCONTROL%"systemMatrix");
    addToTab("General", A);

    B = new ExtWidget("Input matrix",new ChoiceWidget2(new MatWidgetFactory(1,1),QBoxLayout::RightToLeft,5),true,false,MBSIMCONTROL%"inputMatrix");
    addToTab("General", B);

    C = new ExtWidget("Output matrix",new ChoiceWidget2(new MatWidgetFactory(1,1),QBoxLayout::RightToLeft,5),true,false,MBSIMCONTROL%"outputMatrix");
    addToTab("General", C);

    D = new ExtWidget("Feedthrough matrix",new ChoiceWidget2(new SqrMatSizeVarWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIMCONTROL%"feedthroughMatrix");
    addToTab("General", D);
  }

  void LinearTransferSystemPropertyDialog::updateWidget() {
    int sizeA = A->isActive()?static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(A->getWidget())->getWidget())->rows():0;
    int sizeD = D->isActive()?static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(D->getWidget())->getWidget())->rows():0;
    B->resize_(sizeA,sizeD);
    C->resize_(sizeD,sizeA);
  }

  DOMElement* LinearTransferSystemPropertyDialog::initializeUsingXML(DOMElement *parent) {
    SignalPropertyDialog::initializeUsingXML(item->getXMLElement());
    inputSignal->initializeUsingXML(item->getXMLElement());
    A->initializeUsingXML(item->getXMLElement());
    B->initializeUsingXML(item->getXMLElement());
    C->initializeUsingXML(item->getXMLElement());
    D->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* LinearTransferSystemPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    SignalPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    inputSignal->writeXMLFile(item->getXMLElement(),ref);
    A->writeXMLFile(item->getXMLElement(),ref);
    B->writeXMLFile(item->getXMLElement(),ref);
    C->writeXMLFile(item->getXMLElement(),ref);
    D->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  SignalOperationPropertyDialog::SignalOperationPropertyDialog(SignalOperation *signal, QWidget * parent, const Qt::WindowFlags& f) : SignalPropertyDialog(signal,parent,f) {
    inputSignal = new ExtWidget("Input signal",new ListWidget(new ElementOfReferenceWidgetFactory<Signal>(MBSIMCONTROL%"inputSignal",signal),"Signal",1,2,false,1,2),false,false,"");
    addToTab("General", inputSignal);

    function = new ExtWidget("Function",new ChoiceWidget2(new SymbolicFunctionWidgetFactory3(signal,QStringList("x"),1,false),QBoxLayout::TopToBottom,0),false,false,MBSIMCONTROL%"function");
    addToTab("General", function);

    connect(inputSignal,SIGNAL(widgetChanged()),this,SLOT(updateFunctionFactory()));
  }

  void SignalOperationPropertyDialog::updateWidget() {
    function->updateWidget();
  }

  void SignalOperationPropertyDialog::updateFunctionFactory(bool defineWidget) {
    if(static_cast<ListWidget*>(inputSignal->getWidget())->getSize()==1)
      static_cast<ChoiceWidget2*>(function->getWidget())->setWidgetFactory(new SymbolicFunctionWidgetFactory3(getElement(),QStringList("x"),1,false));
    else
      static_cast<ChoiceWidget2*>(function->getWidget())->setWidgetFactory(new SymbolicFunctionWidgetFactory2(getElement(),QStringList("x")<<"y",1,false));
    if(defineWidget)
      static_cast<ChoiceWidget2*>(function->getWidget())->defineWidget(0);
  }

  DOMElement* SignalOperationPropertyDialog::initializeUsingXML(DOMElement *parent) {
    SignalPropertyDialog::initializeUsingXML(item->getXMLElement());
    inputSignal->initializeUsingXML(item->getXMLElement());
    updateFunctionFactory(false);
    function->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* SignalOperationPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    SignalPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    inputSignal->writeXMLFile(item->getXMLElement(),ref);
    function->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  ExternSignalSourcePropertyDialog::ExternSignalSourcePropertyDialog(ExternSignalSource *signal, QWidget * parent, const Qt::WindowFlags& f) : SignalPropertyDialog(signal,parent,f) {
    sourceSize = new ExtWidget("Length of input vector",new SpinBoxWidget(1,1,1000),false,false,MBSIMCONTROL%"sourceSize");
    addToTab("General", sourceSize);
  }

  DOMElement* ExternSignalSourcePropertyDialog::initializeUsingXML(DOMElement *parent) {
    SignalPropertyDialog::initializeUsingXML(item->getXMLElement());
    sourceSize->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* ExternSignalSourcePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    SignalPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    sourceSize->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

  ExternSignalSinkPropertyDialog::ExternSignalSinkPropertyDialog(ExternSignalSink *signal, QWidget * parent, const Qt::WindowFlags& f) : SignalPropertyDialog(signal,parent,f) {
    inputSignal = new ExtWidget("Input signal",new ElementOfReferenceWidget<Signal>(signal,nullptr),false,false,MBSIMCONTROL%"inputSignal");
    addToTab("General", inputSignal);
  }

  DOMElement* ExternSignalSinkPropertyDialog::initializeUsingXML(DOMElement *parent) {
    SignalPropertyDialog::initializeUsingXML(item->getXMLElement());
    inputSignal->initializeUsingXML(item->getXMLElement());
    return parent;
  }

  DOMElement* ExternSignalSinkPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    SignalPropertyDialog::writeXMLFile(item->getXMLElement(),ref);
    inputSignal->writeXMLFile(item->getXMLElement(),ref);
    return nullptr;
  }

}
