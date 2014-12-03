/*
* Copyright © 2012, United States Government, as represented by the
* Administrator of the National Aeronautics and Space Administration.
* All rights reserved.
*
* The NASA Tensegrity Robotics Toolkit (NTRT) v1 platform is licensed
* under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0.
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
* either express or implied. See the License 197.632for the specific language
* governing permissions and limitations under the License.
*/

/**
* @file ICRA2015_test.cpp
* @brief Contains a test of the data submitted to ICRA2015
* $Id$
*/

// This application
#include "dev/btietz/TetraSpineStatic/TetraSpineStaticModel_hf.h"
#include "dev/btietz/TetraSpineStatic/SerializedSpineControl.h"
// This library
#include "core/tgModel.h"
#include "core/tgSimView.h"
#include "core/tgSimViewGraphics.h"
#include "core/tgSimulation.h"
#include "core/tgWorld.h"
#include "helpers/FileHelpers.h"
// The C++ Standard Library
#include <iostream>
#include <fstream>
// Google Test
#include "gtest/gtest.h"


using namespace std;

namespace {

	// The fixture for testing class FileHelpers.
	class HardwareTest : public ::testing::Test {
		protected:
			// You can remove any or all of the following functions if its body
			// is empty.
			
			HardwareTest() {
					
			}
			
			virtual ~HardwareTest() {
				// You can do clean-up work that doesn't throw exceptions here.
			}
			
			// If the constructor and destructor are not enough for setting up
			// and cleaning up each test, you can define the following methods:
			virtual void SetUp() {
				// Code here will be called immediately after the constructor (right
				// before each test).
			}
			
			virtual void TearDown() {
				// Code here will be called immediately after each test (right
				// before the destructor).
			}
			
			// Objects declared here can be used by all tests in the test case for FileHelpers.
	};

	TEST_F(HardwareTest, ICRA2015Static) {
				// First create the world
				const tgWorld::Config config(981); // gravity, cm/sec^2
				tgWorld world(config); 

				// Second create the view
				const double stepSize = 1.0/2000.0; // Seconds
				const double renderRate = 1.0/60.0; // Seconds
				tgSimView view(world, stepSize, renderRate);

				// Third create the simulation
				tgSimulation simulation(view);

				// Fourth create the models with their controllers and add the models to the
				// simulation
				const int segments = 3;
				TetraSpineStaticModel_hf* myModel =
				  new TetraSpineStaticModel_hf(segments);
				
				/* Required for setting up serialization file input/output.
				 * But overwritten in SerializedSpineControl...
				 *  */
				const std::string suffix("controlVars.json");
			   
				SerializedSpineControl* const myControl =
				  new SerializedSpineControl(suffix);

				myModel->attach(myControl);
				/*
				tgCPGLogger* const myLogger = 
				  new tgCPGLogger("logs/CPGValues.txt");
				
				myControl->attach(myLogger);
				*/
				simulation.addModel(myModel);
				
				int i = 0;
				while (i < 1)
				{
					simulation.run(120000);
					//simulation.reset();
					i++;
				}
				// Will print out another set of dist moved on teardown
				
				std::vector<double> simMaxTens = myModel->getStringMaxTensions();
				ASSERT_GE(simMaxTens.size(), 12);
				
	}

} // namespace

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
