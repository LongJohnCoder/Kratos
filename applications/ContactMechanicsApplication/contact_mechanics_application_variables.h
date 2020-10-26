//-------------------------------------------------------------
//          ___         _           _
//  KRATOS / __|___ _ _| |_ __ _ __| |_
//        | (__/ _ \ ' \  _/ _` / _|  _|
//         \___\___/_||_\__\__,_\__|\__|MECHANICS
//
//  License:(BSD)    ContactMechanicsApplication/license.txt
//
//  Main authors:    Josep Maria Carbonell
//                   ...
//
//-------------------------------------------------------------
//
//   Project Name:        KratosContactMechanicsApplication $
//   Created by:          $Author:              JMCarbonell $
//   Last modified by:    $Co-Author:                       $
//   Date:                $Date:                  July 2016 $
//   Revision:            $Revision:                    0.0 $
//
//


#if !defined(KRATOS_CONTACT_MECHANICS_APPLICATION_VARIABLES_H_INCLUDED )
#define  KRATOS_CONTACT_MECHANICS_APPLICATION_VARIABLES_H_INCLUDED

// System includes

// External includes

// Project includes

// Core applications
#include "includes/mat_variables.h"
#include "delaunay_meshing_application_variables.h"
#include "custom_friction/friction_law.hpp"

namespace Kratos
{
   ///@name Type Definitions
   ///@{
   typedef FrictionLaw::Pointer  FrictionLawPointerType;
   ///@}

   ///@name Kratos Globals
   ///@{

   //Define Variables

   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, std::string, FRICTION_LAW_NAME )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, FrictionLawPointerType, FRICTION_LAW )

   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, bool, FRICTION_ACTIVE )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, bool, HYDRAULIC )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, PENALTY_PARAMETER )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, LAGRANGE_MULTIPLIER_NORMAL )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, LAGRANGE_MULTIPLIER_NORMAL_REACTION )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, LAGRANGE_MULTIPLIER_TANGENTIAL )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, LAGRANGE_MULTIPLIER_TANGENTIAL_REACTION )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, TAU_STAB )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, MU_STATIC )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, MU_DYNAMIC )

   KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( CONTACT_MECHANICS_APPLICATION, WATER_CONTACT_FORCE )
   KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( CONTACT_MECHANICS_APPLICATION, CONTACT_STRESS )
   KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( CONTACT_MECHANICS_APPLICATION, EFFECTIVE_CONTACT_STRESS )
   KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( CONTACT_MECHANICS_APPLICATION, EFFECTIVE_CONTACT_FORCE )

   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, CONTACT_ADHESION )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, CONTACT_FRICTION_ANGLE )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, TANGENTIAL_PENALTY_RATIO )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, CONTACT_PLASTIC_SLIP )

   //thermal properties
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, HEAT_CONDUCTIVITY )

   //solution
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, int, SEGREGATED_STEP )
   KRATOS_DEFINE_APPLICATION_VARIABLE( CONTACT_MECHANICS_APPLICATION, double, CONTACT_STEP_TIME )
   ///@}
}

#endif	// KRATOS_CONTACT_MECHANICS_APPLICATION_VARIABLES_H_INCLUDED