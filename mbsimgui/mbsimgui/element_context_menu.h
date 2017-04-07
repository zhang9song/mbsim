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

#ifndef _ELEMENT_CONTEXT_MENU_H_
#define _ELEMENT_CONTEXT_MENU_H_

#include <QMenu>

namespace MBSimGUI {

  class Element;

  class EmbeddingContextMenu : public QMenu {
    Q_OBJECT

    public:
      EmbeddingContextMenu(Element *element, const QString &title="", QWidget * parent = 0);

    protected slots:
      void addScalarParameter();
      void addVectorParameter();
      void addMatrixParameter();
      void addStringParameter();
      void addImportParameter();

    protected:
      Element *element;
  };

  class ElementContextMenu : public QMenu {
    Q_OBJECT

    public:
      ElementContextMenu(Element *element, QWidget * parent = 0, bool removable=true);

    protected:
      Element *element;
  };

  class GroupContextMenu : public ElementContextMenu {
    public:
      GroupContextMenu(Element *group, QWidget * parent = 0, bool removable=true) : ElementContextMenu(element,parent,removable) { }
  };

  class DynamicSystemSolverContextMenu : public GroupContextMenu {
    public:
      DynamicSystemSolverContextMenu(Element *solver, QWidget * parent = 0) : GroupContextMenu(solver,parent,false) { }
  };

  class FrameContextMenu : public ElementContextMenu {
    public:
      FrameContextMenu(Element *frame, QWidget * parent = 0, bool removable=false) : ElementContextMenu(frame,parent,removable) { }
  };

  class FixedRelativeFrameContextMenu : public FrameContextMenu {
    public:
      FixedRelativeFrameContextMenu(Element *frame, QWidget * parent = 0) : FrameContextMenu(frame,parent,true) { }
  };

  class NodeFrameContextMenu : public FrameContextMenu {
    public:
      NodeFrameContextMenu(Element *frame, QWidget * parent = 0) : FrameContextMenu(frame,parent,true) { }
  };

  class ObjectContextMenu : public ElementContextMenu {
    public:
      ObjectContextMenu(Element *object, QWidget * parent = 0) : ElementContextMenu(element,parent,true) { }
  };

  class RigidBodyContextMenu : public ObjectContextMenu {
    public:
      RigidBodyContextMenu(Element *body, QWidget * parent = 0) : ObjectContextMenu(element,parent) { }
  };

  class FlexibleBodyFFRContextMenu : public ObjectContextMenu {
    public:
      FlexibleBodyFFRContextMenu(Element *body, QWidget * parent = 0) : ObjectContextMenu(element,parent) { }
  };

  class FixedRelativeFrameContextContextMenu : public QMenu {
    Q_OBJECT

    public:
      FixedRelativeFrameContextContextMenu(Element *contour, const QString &title="", QWidget * parent = 0);

      protected slots:
        void addFixedRelativeFrame();

    protected:
      Element *element;
  };

  class NodeFrameContextContextMenu : public QMenu {
    Q_OBJECT

    public:
      NodeFrameContextContextMenu(Element *contour, const QString &title="", QWidget * parent = 0);

      protected slots:
        void addNodeFrame();

    protected:
      Element *element;
  };

  class ContourContextContextMenu : public QMenu {
    Q_OBJECT

    public:
      ContourContextContextMenu(Element *contour, const QString &title="", QWidget * parent = 0);

    protected slots:
      void addPoint();
      void addLine();
      void addPlane();
      void addSphere();
      void addCircle();
      void addCuboid();
      void addLineSegment();
      void addPlanarContour();
      void addSpatialContour();

    protected:
      Element *element;
  };

  class GroupContextContextMenu : public QMenu {
    Q_OBJECT

    public:
      GroupContextContextMenu(Element *contour, const QString &title="", QWidget * parent = 0);

      protected slots:
        void addGroup();

    protected:
      Element *element;
  };

  class ObjectContextContextMenu : public QMenu {

    public:
      ObjectContextContextMenu(Element *object, const QString &title="", QWidget * parent = 0);

    protected:
      Element *element;
  };

  class BodyContextContextMenu : public QMenu {
    Q_OBJECT

    public:
      BodyContextContextMenu(Element *object, const QString &title="", QWidget * parent = 0);

      protected slots:
        void addRigidBody();
        void addFlexibleBodyFFR();

    protected:
      Element *element;
  };

  class ConstraintContextContextMenu : public QMenu {
    Q_OBJECT

    public:
      ConstraintContextContextMenu(Element *object, const QString &title="", QWidget * parent = 0);

    protected slots:
      void addGeneralizedGearConstraint();
      void addGeneralizedPositionConstraint();
      void addGeneralizedVelocityConstraint();
      void addGeneralizedAccelerationConstraint();
      void addJointConstraint();
      void addGeneralizedConnectionConstraint();

    protected:
      Element *element;
  };

  class LinkContextContextMenu : public QMenu {
    Q_OBJECT

    public:
      LinkContextContextMenu(Element *link, const QString &title="", QWidget * parent = 0);

      protected slots:
        void addSpringDamper();
      void addDirectionalSpringDamper();
      void addGeneralizedSpringDamper();
      void addKineticExcitation();
      void addJoint();
      void addElasticJoint();
      void addContact();
      void addSignal();
      void addGeneralizedFriction();
      void addGeneralizedGear();
      void addGeneralizedElasticConnection();

    protected:
      Element *element;
  };

  class ObserverContextContextMenu : public QMenu {
    Q_OBJECT

    public:
      ObserverContextContextMenu(Element *observer, const QString &title="", QWidget * parent = 0);

    protected slots:
      void addMechanicalLinkObserver();
      void addMechanicalConstraintObserver();
      void addContactObserver();
      void addFrameObserver();
      void addRigidBodyObserver();
      void addKinematicCoordinatesObserver();
      void addRelativeKinematicsObserver();

    protected:
      Element *element;
  };

  class SignalContextContextMenu : public QMenu {
    Q_OBJECT

    public:
      SignalContextContextMenu(Element *signal, const QString &title="", QWidget * parent = 0);

    protected slots:
      void addSensor();
      void addPIDController();
      void addUnarySignalOperation();
      void addBinarySignalOperation();
      void addExternSignalSource();
      void addExternSignalSink();

    protected:
      Element *element;
  };

  class SensorContextContextMenu : public QMenu {
    Q_OBJECT

    public:
      SensorContextContextMenu(Element *sensor, const QString &title="", QWidget * parent = 0);

    protected slots:
      void addGeneralizedPositionSensor();
      void addGeneralizedVelocitySensor();
      void addAbsolutePositionSensor();
      void addAbsoluteVelocitySensor();
      void addAbsoluteAngularPositionSensor();
      void addAbsoluteAngularVelocitySensor();
      void addFunctionSensor();

    protected:
      Element *element;
  };

}

#endif
