/*
 * Copyright 2014 Open Source Robotics Foundation
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

#include <gtest/gtest.h>
#include <map>
#include <boost/filesystem.hpp>
#include "sdf/sdf.hh"

#include "test_config.h"

const std::string SDF_TEST_FILE = std::string(PROJECT_SOURCE_PATH)
  + "/test/integration/urdf_joint_parameters.urdf";

/////////////////////////////////////////////////
TEST(SDFParser, JointAxisParameters)
{
  char *pathCStr = getenv("SDF_PATH");
  boost::filesystem::path path = PROJECT_SOURCE_PATH;
  path = path / "sdf" / SDF_VERSION;
  setenv("SDF_PATH", path.string().c_str(), 1);

  sdf::SDFPtr robot(new sdf::SDF());
  sdf::init(robot);
  ASSERT_TRUE(sdf::readFile(SDF_TEST_FILE, robot));
  if (pathCStr)
  {
    setenv("SDF_PATH", pathCStr, 1);
  }
  else
  {
    unsetenv("SDF_PATH");
  }

  sdf::ElementPtr model = robot->Root()->GetElement("model");

  ASSERT_TRUE(model->HasElement("joint"));
  unsigned int bitmask = 0;
  for (sdf::ElementPtr joint = model->GetElement("joint"); joint;
       joint = joint->GetNextElement("joint"))
  {
    std::string jointName = joint->Get<std::string>("name");

    double value = -1.0;
    if (jointName == "jointw0")
    {
      bitmask |= 0x1;
      value = 0.0;
    }
    else if (jointName == "joint01")
    {
      bitmask |= 0x2;
      value = 1.0;
    }
    else
    {
      continue;
    }
    ASSERT_TRUE(joint->HasElement("axis"));
    sdf::ElementPtr axis = joint->GetElement("axis");
    ASSERT_TRUE(axis->HasElement("dynamics"));
    sdf::ElementPtr dynamics = axis->GetElement("dynamics");
    ASSERT_TRUE(dynamics->HasElement("damping"));
    ASSERT_TRUE(dynamics->HasElement("friction"));
    EXPECT_DOUBLE_EQ(value, dynamics->Get<double>("damping"));
    EXPECT_DOUBLE_EQ(value, dynamics->Get<double>("friction"));
  }
  EXPECT_EQ(bitmask, 0x3u);
}
