/*
 * Copyright (C) 2012-2015 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#include <sstream>

#include "gazebo/msgs/msgs.hh"
#include "gazebo/gui/GuiEvents.hh"

#include "gazebo/common/Console.hh"
#include "gazebo/common/MouseEvent.hh"
#include "gazebo/math/Quaternion.hh"

#include "gazebo/rendering/UserCamera.hh"

#include "gazebo/transport/Publisher.hh"

#include "gazebo/gui/CylinderMaker.hh"

using namespace gazebo;
using namespace gui;

unsigned int CylinderMaker::counter = 0;

CylinderMaker::CylinderMaker()
  : EntityMaker()
{
  this->state = 0;
  this->leftMousePressed = false;
  this->visualMsg = new msgs::Visual();
  this->visualMsg->mutable_geometry()->set_type(msgs::Geometry::CYLINDER);
  this->visualMsg->mutable_material()->mutable_script()->add_uri(
      "gazebo://media/materials/scripts/gazebo.material");
  this->visualMsg->mutable_material()->mutable_script()->set_name(
      "Gazebo/TurquoiseGlowOutline");
  msgs::Set(this->visualMsg->mutable_pose()->mutable_orientation(),
            math::Quaternion());
}

CylinderMaker::~CylinderMaker()
{
  this->camera.reset();
  delete this->visualMsg;
}

void CylinderMaker::Start(const rendering::UserCameraPtr _camera)
{
  this->camera = _camera;
  std::ostringstream stream;
  stream << "__GZ_USER_cylinder_" << counter++;
  this->visualMsg->set_name(stream.str());
  this->state = 1;
}

void CylinderMaker::Stop()
{
  msgs::Request *msg = msgs::CreateRequest("entity_delete",
                                           this->visualMsg->name());

  this->requestPub->Publish(*msg);
  delete msg;

  this->state = 0;
  gui::Events::moveMode(true);
}

bool CylinderMaker::IsActive() const
{
  return this->state > 0;
}

void CylinderMaker::OnMousePush(const common::MouseEvent &_event)
{
  if (this->state == 0)
    return;

  this->mousePushPos = _event.pressPos;
}

void CylinderMaker::OnMouseRelease(const common::MouseEvent &_event)
{
  if (this->state == 0)
    return;

  this->state++;
  this->mouseReleasePos = _event.pos;

  if (this->state == 3)
  {
    this->CreateTheEntity();
    this->Stop();
  }
}

/////////////////////////////////////////////////
void CylinderMaker::OnMouseMove(const common::MouseEvent &_event)
{
  if (this->state < 2)
    return;

  math::Vector3 norm;
  math::Vector3 p1, p2;

  norm.Set(1, 0, 0);

  math::Vector3 p(this->visualMsg->pose().position().x(),
                  this->visualMsg->pose().position().y(),
                  this->visualMsg->pose().position().z());

  double size = (this->mouseReleasePos.y - _event.pos.y) * 0.01;
  if (!_event.shift)
    size = rint(size);

  this->visualMsg->mutable_geometry()->mutable_cylinder()->set_length(size);

  p.z = size / 2.0;

  msgs::Set(this->visualMsg->mutable_pose()->mutable_position(), p);
  this->visPub->Publish(*this->visualMsg);
}

/////////////////////////////////////////////////
void CylinderMaker::OnMouseDrag(const common::MouseEvent &_event)
{
  if (this->state == 0)
    return;

  math::Vector3 norm;
  math::Vector3 p1, p2;

  norm.Set(0, 0, 1);

  if (!this->camera->GetWorldPointOnPlane(this->mousePushPos.x,
                                          this->mousePushPos.y,
                                          math::Plane(norm), p1))
  {
    gzerr << "Invalid mouse point\n";
    return;
  }

  p1.Round();

  if (!this->camera->GetWorldPointOnPlane(
        _event.pos.x, _event.pos.y, math::Plane(norm), p2))
  {
    gzerr << "Invalid mouse point\n";
    return;
  }

  p2 = this->GetSnappedPoint(p2);

  if (this->state == 1)
    msgs::Set(this->visualMsg->mutable_pose()->mutable_position(), p1);

  math::Vector3 p(this->visualMsg->pose().position().x(),
                  this->visualMsg->pose().position().y(),
                  this->visualMsg->pose().position().z());

  if (this->state == 1)
  {
    double dist = p1.Distance(p2);
    double rounded = rint(dist);
    if (fabs(dist - rounded) < 0.2)
      dist = rounded;

    this->visualMsg->mutable_geometry()->mutable_cylinder()->set_radius(dist);
    this->visualMsg->mutable_geometry()->mutable_cylinder()->set_length(0.01);
  }

  msgs::Set(this->visualMsg->mutable_pose()->mutable_position(), p);
  this->visPub->Publish(*this->visualMsg);
}

/////////////////////////////////////////////////
std::string CylinderMaker::GetSDFString()
{
  msgs::Model model;
  {
    std::ostringstream modelName;
    modelName << "unit_cylinder_" << counter;
    model.set_name(modelName.str());
  }
  msgs::Set(model.mutable_pose(), math::Pose(0, 0, 0.5, 0, 0, 0));
  msgs::AddCylinderLink(model, 1.0, 0.5, 1.0);
  model.mutable_link(0)->set_name("link");

  return "<sdf version='" + std::string(SDF_VERSION) + "'>"
         + msgs::ModelToSDF(model)->ToString("")
         + "</sdf>";
}

/////////////////////////////////////////////////
void CylinderMaker::CreateTheEntity()
{
  msgs::Factory msg;

  msg.set_sdf(this->GetSDFString());

  msgs::Request *requestMsg = msgs::CreateRequest("entity_delete",
      this->visualMsg->name());
  this->requestPub->Publish(*requestMsg);
  delete requestMsg;

  this->makerPub->Publish(msg);
  this->camera.reset();
}
