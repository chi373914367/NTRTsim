/*
 * Copyright © 2015, United States Government, as represented by the
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
 * either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
*/

/**
 * @file UpperLimbModel.cpp
 * @brief Contains the implementation of class UpperLimbModel. See README
 * $Id$
 */

// This module
#include "UpperLimbModel.h"
// This library
#include "core/tgBasicActuator.h"
#include "core/tgRod.h"
#include "core/abstractMarker.h"
#include "tgcreator/tgBuildSpec.h"
#include "tgcreator/tgBasicActuatorInfo.h"
#include "tgcreator/tgKinematicContactCableInfo.h"
#include "tgcreator/tgRodInfo.h"
#include "tgcreator/tgStructure.h"
#include "tgcreator/tgStructureInfo.h"
// The Bullet Physics library
#include "LinearMath/btVector3.h"
// The C++ Standard Library
#include <stdexcept>

namespace {
    // see tgBaseString.h for a descripton of some of these rod parameters
    // (specifically, those related to the motor moving the strings.)
    // NOTE that any parameter that depends on units of length will scale
    // with the current gravity scaling. E.g., with gravity as 98.1,
    // the length units below are in decimeters.

    const struct ConfigRod {
        double density;
        double radius;
        double rod_length;
        double rod_space;    
        double friction;
        double rollFriction;
        double restitution;
    } cRod = {
        0.05,    // density (kg / length^3)
        0.8,     // radius (length)
        15.0,     // rod_length (length)
        7.5,      // rod_space (length)
        1.0,      // friction (unitless)
        0.01,     // rollFriction (unitless)
        0.2      // restitution (?)
    };

    const struct ConfigCable {
        double elasticity;
        double damping;
        double stiffness;
        double pretension_olecranon;
        double pretension_anconeus;
        double pretension_brachioradialis;
        double pretension_support;
        bool   history;  
        double maxTens;
        double targetVelocity; 
        double mRad;
        double motorFriction;
        double motorInertia;
        bool backDrivable;
    } cCable = {
        1000.0,         // elasticity
        200.0,          // damping (kg/s)
        3000.0,         // stiffness (kg / sec^2)
        3000.0/1,       // pretension_olecranon (force), stiffness/initial length
        3000.0/15.55,   // pretension_anconeus (force), stiffness/initial length
        3000.0/262,     // pretension_brachioradialis (force), stiffness/initial length 
        30000.0/1,      // pretension_support (force), stiffness/initial length 
        false,          // history (boolean)
        100000,         // maxTens
        10000,           // targetVelocity  
        1.0,            // mRad
        10.0,           // motorFriction
        1.0,            // motorInertia
        false           // backDrivable (boolean)
    };
} // namespace

UpperLimbModel::UpperLimbModel() : tgModel() {}

UpperLimbModel::~UpperLimbModel() {}

void UpperLimbModel::addNodes(tgStructure& s) {
    const double scale = 0.5;
    const double bone_scale = 0.3;
    const size_t nNodes = 14; //TODO: Change to variable, incremented by calling sub-structure node adding functions (i.e. one for olecranon, one for humerus, etc.)
    
    // Average Adult Male Measurements with scale
    // Lengths are in mm
    const double b = 334 * scale * bone_scale; //ulna length
    const double c = 265 * scale * bone_scale; //humerus length
    const double g = 17 * scale; //ulna proximal width
    const double e = g/4;

    //Format: (x, z, y)
    // olecranon
    nodePositions.push_back(btVector3(0, 0, 0));
    nodePositions.push_back(btVector3(-g, g, 0));
    nodePositions.push_back(btVector3(g, g, 0));
    nodePositions.push_back(btVector3(g, -g, 0));
    
    // ulna-radius
    nodePositions.push_back(btVector3(3*e, 0, g));
    nodePositions.push_back(btVector3(3*e, 0, -g));
    nodePositions.push_back(btVector3(7*e, 0, 0));
    nodePositions.push_back(btVector3(b+7*e, 0, 0));

    // humerus
    nodePositions.push_back(btVector3(0, 3*e, g));
    nodePositions.push_back(btVector3(0, 3*e, -g));
    nodePositions.push_back(btVector3(0, 7*e, 0));
    nodePositions.push_back(btVector3(0, c+7*e, 0));

    //top of humerus
    nodePositions.push_back(btVector3(0, c+7*e+g, g));
    nodePositions.push_back(btVector3(0, c+7*e+g, -g));/*
    nodePositions.push_back(btVector3(,,));
    nodePositions.push_back(btVector3(,,));
    nodePositions.push_back(btVector3(,,));
    nodePositions.push_back(btVector3(,,));
    nodePositions.push_back(btVector3(,,));*/

    for(size_t i=0;i<nNodes;i++) {
		s.addNode(nodePositions[i][0],nodePositions[i][1],nodePositions[i][2]);
    }
}
                  
void UpperLimbModel::addRods(tgStructure& s) {   
    //olecranon
    s.addPair(0, 1, "bone");
    s.addPair(0, 2, "bone");
    s.addPair(0, 3, "bone");

    // ulna and radius
    s.addPair(4, 6,  "bone");
    s.addPair(5, 6,  "bone");
    s.addPair(6, 7,  "bone");

    // humerus
    s.addPair(8, 10,  "humerus massless");
    s.addPair(9, 10,  "humerus massless"); 
    s.addPair(10, 11,  "humerus massless");
    s.addPair(11, 12,  "humerus massless");
    s.addPair(11, 13,  "humerus massless");
}

void UpperLimbModel::addMuscles(tgStructure& s) {
    const std::vector<tgStructure*> children = s.getChildren();

    // ulna-radius to olecranon
    s.addPair(6, 2, "olecranon muscle");
    s.addPair(6, 3, "olecranon muscle");

    s.addPair(4, 0, "olecranon muscle");
    s.addPair(4, 2, "olecranon muscle");
    s.addPair(4, 3, "olecranon muscle");
    s.addPair(4, 8, "right anocneus  muscle");
    
    s.addPair(5, 0, "olecranon muscle");
    s.addPair(5, 2, "olecranon muscle");
    s.addPair(5, 3, "olecranon muscle");
    s.addPair(5, 9, "left anconeus muscle");
    
    // humerus to olecranon
    s.addPair(8, 0, "olecranon muscle");
    s.addPair(8, 1, "olecranon muscle");
    s.addPair(8, 2, "olecranon muscle");
    
    s.addPair(9, 0, "olecranon muscle");
    s.addPair(9, 1, "olecranon muscle");
    s.addPair(9, 2, "olecranon muscle");
    
    s.addPair(10, 1, "olecranon muscle");
    s.addPair(10, 2, "brachioradialis muscle");
}
 
/*
void UpperLimbModel::addMarkers(tgStructure &s) {
    std::vector<tgRod *> rods=find<tgRod>("rod");

	for(int i=0;i<10;i++)
	{
		const btRigidBody* bt = rods[rodNumbersPerNode[i]]->getPRigidBody();
		btTransform inverseTransform = bt->getWorldTransform().inverse();
		btVector3 pos = inverseTransform * (nodePositions[i]);
		abstractMarker tmp=abstractMarker(bt,pos,btVector3(0.08*i,1.0 - 0.08*i,.0),i);
		this->addMarker(tmp);
	}
}
*/
 
void UpperLimbModel::setup(tgWorld& world) {
    const tgRod::Config boneConfig(cRod.radius, cRod.density, cRod.friction, cRod.rollFriction, cRod.restitution);
    const tgRod::Config boneConfigMassless(cRod.radius, 0.00/*c.density*/, cRod.friction, cRod.rollFriction, cRod.restitution);
    /// @todo acceleration constraint was removed on 12/10/14 Replace with tgKinematicActuator as appropreate

    tgBasicActuator::Config olecranonMuscleConfig(cCable.stiffness, cCable.damping, cCable.pretension_olecranon, 
                                                  cCable.history, cCable.maxTens, cCable.targetVelocity);
    tgBasicActuator::Config anconeusMuscleConfig(cCable.stiffness, cCable.damping, cCable.pretension_anconeus, 
                                                 cCable.history, cCable.maxTens, cCable.targetVelocity);
    tgBasicActuator::Config brachioradialisMuscleConfig(cCable.stiffness, cCable.damping, cCable.pretension_brachioradialis, 
                                                        cCable.history, cCable.maxTens, cCable.targetVelocity);
    tgBasicActuator::Config supportstringMuscleConfig(cCable.stiffness, cCable.damping, cCable.pretension_support, 
                                                      cCable.history, cCable.maxTens, cCable.targetVelocity);
            
    // Start creating the structure
    tgStructure s;
    addNodes(s);
    addRods(s);
    addMuscles(s);
    
    // Move the arm out of the ground
    btVector3 offset(0.0, 50.0, 0.0);
    s.move(offset);
    
    // Create the build spec that uses tags to turn the structure into a real model
    tgBuildSpec spec;
    spec.addBuilder("bone", new tgRodInfo(boneConfig));
    spec.addBuilder("massless", new tgRodInfo(boneConfigMassless));
    spec.addBuilder("olecranon muscle", new tgBasicActuatorInfo(olecranonMuscleConfig));
    spec.addBuilder("anconeus muscle", new tgBasicActuatorInfo(anconeusMuscleConfig));
    spec.addBuilder("brachioradialis muscle", new tgBasicActuatorInfo(brachioradialisMuscleConfig));
    spec.addBuilder("support muscle", new tgBasicActuatorInfo(supportstringMuscleConfig));
    
    // Create your structureInfo
    tgStructureInfo structureInfo(s, spec);

    // Use the structureInfo to build ourselves
    structureInfo.buildInto(*this, world);

    // We could now use tgCast::filter or similar to pull out the
    // models (e.g. muscles) that we want to control. 
    allMuscles = tgCast::filter<tgModel, tgBasicActuator> (getDescendants());

    // call the onSetup methods of all observed things e.g. controllers
    notifySetup();

    // Actually setup the children
    tgModel::setup(world);

    //map the rods and add the markers to them
    //addMarkers(s);

}

void UpperLimbModel::step(double dt)
{
    // Precondition
    if (dt <= 0.0) {
        throw std::invalid_argument("dt is not positive");
    } else {
        // Notify observers (controllers) of the step so that they can take action
        notifyStep(dt);
        tgModel::step(dt);  // Step any children
    }
}

void UpperLimbModel::onVisit(tgModelVisitor& r)
{
    tgModel::onVisit(r);
}

const std::vector<tgBasicActuator*>& UpperLimbModel::getAllMuscles() const
{
    return allMuscles;
}
    
void UpperLimbModel::teardown()
{
    notifyTeardown();
    tgModel::teardown();
}
