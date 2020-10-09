// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:         BSD License
//                     license: structural_mechanics_application/license.txt
//
//  Main authors:    Riccardo Rossi
//

// System includes

#if defined(KRATOS_PYTHON)
// External includes


// Project includes
#include "includes/define_python.h"
#include "structural_mechanics_application_variables.h"
#include "structural_mechanics_application.h"
#include "custom_python/add_custom_strategies_to_python.h"
#include "custom_python/add_custom_processes_to_python.h"
#include "custom_python/add_custom_utilities_to_python.h"
#include "custom_python/add_custom_constitutive_laws_to_python.h"
#include "custom_python/add_custom_response_functions_to_python.h"

#ifndef STRUCTURAL_DISABLE_ADVANCED_CONSTITUTIVE_LAWS
#include "custom_python/add_custom_advanced_constitutive_laws_to_python.h"
#endif // STRUCTURAL_DISABLE_ADVANCED_CONSTITUTIVE_LAWS

namespace Kratos {
namespace Python {

PYBIND11_MODULE(KratosStructuralMechanicsApplication,m)
{
    namespace py = pybind11;

    py::class_<KratosStructuralMechanicsApplication,
            KratosStructuralMechanicsApplication::Pointer,
            KratosApplication >(m,"KratosStructuralMechanicsApplication")
            .def(py::init<>())
            ;

    AddCustomStrategiesToPython(m);
    AddCustomProcessesToPython(m);
    AddCustomUtilitiesToPython(m);
    AddCustomConstitutiveLawsToPython(m);
    AddCustomResponseFunctionUtilitiesToPython(m);

#ifndef STRUCTURAL_DISABLE_ADVANCED_CONSTITUTIVE_LAWS
    AddCustomAdvancedConstitutiveLawsToPython(m);
#endif // STRUCTURAL_DISABLE_ADVANCED_CONSTITUTIVE_LAWS

    py::class_<Variable<ShellCrossSection::Pointer>,VariableData >(m,"ShellCrossSectionVariable");

    // General pourpose
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INTEGRATION_ORDER); // The integration order considered on the element
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LOCAL_MATERIAL_AXIS_1 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LOCAL_MATERIAL_AXIS_2 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LOCAL_MATERIAL_AXIS_3 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LOCAL_PRESTRESS_AXIS_1 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LOCAL_PRESTRESS_AXIS_2 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CENTER_OF_GRAVITY )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MASS_MOMENT_OF_INERTIA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ELASTICITY_TENSOR )

    //registering variables in python
    // Generalized eigenvalue problem
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BUILD_LEVEL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EIGENVALUE_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EIGENVECTOR_MATRIX )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MODAL_MASS_MATRIX )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MODAL_STIFFNESS_MATRIX )

    // Cross section
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_CROSS_SECTION )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_CROSS_SECTION_OUTPUT_PLY_ID )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_CROSS_SECTION_OUTPUT_PLY_LOCATION )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ORTHOTROPIC_LAYERS)

    // Geometrical
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,LOCAL_ELEMENT_ORIENTATION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,MATERIAL_ORIENTATION_ANGLE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,USE_CONSISTENT_MASS_MATRIX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,CONDENSED_DOF_LIST)

    // Truss generalized variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,TRUSS_PRESTRESS_PK2)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, HARDENING_MODULUS_1D)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TANGENT_MODULUS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_ALPHA)

    // Beam generalized variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,AREA_EFFECTIVE_Y)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,AREA_EFFECTIVE_Z)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,INERTIA_ROT_Y)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,INERTIA_ROT_Z)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,LOCAL_AXES_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,TORSIONAL_INERTIA)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,I22)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,I33)

    // shell generalized variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, STENBERG_SHEAR_STABILIZATION_SUITABLE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_OFFSET )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_STRAIN )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_STRAIN_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_CURVATURE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_CURVATURE_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_FORCE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_FORCE_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_MOMENT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_MOMENT_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_STRESS_TOP_SURFACE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_STRESS_TOP_SURFACE_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_STRESS_MIDDLE_SURFACE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_STRESS_MIDDLE_SURFACE_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_STRESS_BOTTOM_SURFACE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_STRESS_BOTTOM_SURFACE_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, VON_MISES_STRESS_TOP_SURFACE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, VON_MISES_STRESS_MIDDLE_SURFACE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, VON_MISES_STRESS_BOTTOM_SURFACE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHEAR_ANGLE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ORTHOTROPIC_STRESS_BOTTOM_SURFACE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ORTHOTROPIC_STRESS_TOP_SURFACE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ORTHOTROPIC_STRESS_BOTTOM_SURFACE_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ORTHOTROPIC_STRESS_TOP_SURFACE_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ORTHOTROPIC_4PLY_THROUGH_THICKNESS )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TSAI_WU_RESERVE_FACTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ORTHOTROPIC_LAMINA_STRENGTHS )

    // Shell energies
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ELEMENT_MEMBRANE_ENERGY )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ELEMENT_BENDING_ENERGY )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ELEMENT_SHEAR_ENERGY )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ELEMENT_MEMBRANE_ENERGY_FRACTION )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ELEMENT_BENDING_ENERGY_FRACTION )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHELL_ELEMENT_SHEAR_ENERGY_FRACTION )

    // Membrane variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MEMBRANE_PRESTRESS )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRESTRESS_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRESTRESS_AXIS_1_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRESTRESS_AXIS_2_GLOBAL )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRESTRESS_AXIS_1 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRESTRESS_AXIS_2 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PROJECTION_TYPE_COMBO )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRINCIPAL_CAUCHY_STRESS_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRINCIPAL_PK2_STRESS_VECTOR )

    // Mixed formulations variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, REACTION_STRAIN )

    //Formfinding
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LAMBDA_MAX )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, IS_FORMFINDING )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BASE_REF_1 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BASE_REF_2 )

    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, NODAL_INITIAL_DISPLACEMENT )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, NODAL_DISPLACEMENT_STIFFNESS )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, NODAL_ROTATIONAL_STIFFNESS )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, NODAL_DAMPING_RATIO )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, NODAL_ROTATIONAL_DAMPING_RATIO )

    // For explicit central difference scheme
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MASS_FACTOR)
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, MIDDLE_VELOCITY)
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, MIDDLE_ANGULAR_VELOCITY)
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, FRACTIONAL_ACCELERATION)
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, FRACTIONAL_ANGULAR_ACCELERATION)
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, NODAL_INERTIA)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, NODAL_DISPLACEMENT_DAMPING)
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, NODAL_ROTATION_DAMPING)

    // CONDITIONS
    /* Point moment */
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, POINT_MOMENT )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CROSS_AREA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MEAN_RADIUS )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SECTION_SIDES )

    /* Adding the SPRISM EAS variables */
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,ALPHA_EAS);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,CONSIDER_IMPLICIT_EAS_SPRISM_ELEMENT);

    /* Adding the SPRISM additional variables */
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,ANG_ROT);

    /* Adding the SPRISM variable to deactivate the quadratic interpolation */
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,CONSIDER_QUADRATIC_SPRISM_ELEMENT);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,PURE_EXPLICIT_RHS_COMPUTATION);

    /* Reset equations ids "flag" */
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,RESET_EQUATION_IDS);

    /* Hencky strain */
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,HENCKY_STRAIN_VECTOR);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,HENCKY_STRAIN_TENSOR);

    // Adding the LAMBDA load factor considered in the arclength
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,LAMBDA);

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, VON_MISES_STRESS )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, REFERENCE_DEFORMATION_GRADIENT);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, REFERENCE_DEFORMATION_GRADIENT_DETERMINANT);

    // Rayleigh variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RAYLEIGH_ALPHA )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RAYLEIGH_BETA )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, XI_DAMPING)

    // System damping
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SYSTEM_DAMPING_RATIO )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SECOND_SYSTEM_DAMPING_RATIO )

    //nodal load variables
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,  POINT_LOAD )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,  LINE_LOAD )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,  SURFACE_LOAD )

    //condition load variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,POINT_LOADS_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,LINE_LOADS_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,SURFACE_LOADS_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,POSITIVE_FACE_PRESSURES_VECTOR )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,NEGATIVE_FACE_PRESSURES_VECTOR )

    // Displacement-Control variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LOAD_FACTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRESCRIBED_DISPLACEMENT)

    // Response function variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,RESPONSE_VALUE)

    // Constitutive laws variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INELASTIC_FLAG)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INFINITY_YIELD_STRESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, YIELD_STRESS_TENSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_STRAIN_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_DEFORMATION_GRADIENT)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, YIELD_STRESS_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DILATANCY_ANGLE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SOFTENING_TYPE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SOFTENING_TYPE_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, HARDENING_CURVE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MAX_NUMBER_NL_CL_ITERATIONS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, VISCOUS_PARAMETER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DELAY_TIME)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, UNIAXIAL_STRESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FRICTION_ANGLE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_MATRIX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_FIBER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, THRESHOLD)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, COHESION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INTEGRATED_STRESS_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_STRAIN_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_TENSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, THRESHOLD_TENSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, THRESHOLD_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, UNIAXIAL_STRESS_TENSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, UNIAXIAL_STRESS_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FRACTURE_ENERGY_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CURVE_FITTING_PARAMETERS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_STRAIN_INDICATORS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EQUIVALENT_PLASTIC_STRAIN)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, KINEMATIC_PLASTICITY_PARAMETERS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, KINEMATIC_HARDENING_TYPE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CONSIDER_PERTURBATION_THRESHOLD)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TANGENT_OPERATOR_ESTIMATION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TENSION_STRESS_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, COMPRESSION_STRESS_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TENSION_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, COMPRESSION_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EFFECTIVE_TENSION_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EFFECTIVE_COMPRESSION_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CAUCHY_STRESS_TENSOR_FIBER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CAUCHY_STRESS_TENSOR_MATRIX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, GREEN_LAGRANGE_STRAIN_TENSOR_MATRIX)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, GREEN_LAGRANGE_STRAIN_TENSOR_FIBER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EXPONENTIAL_SATURATION_YIELD_STRESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ACCUMULATED_PLASTIC_STRAIN)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, HIGH_CYCLE_FATIGUE_COEFFICIENTS)
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, EULER_ANGLES)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LAYER_EULER_ANGLES)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FATIGUE_REDUCTION_FACTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, NUMBER_OF_CYCLES)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, WOHLER_STRESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BACK_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BACK_STRESS_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FRACTURE_ENERGY_DAMAGE_PROCESS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, HARDENING_MODULI_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, OGDEN_BETA_1)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, OGDEN_BETA_2)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MULTI_LINEAR_ELASTICITY_MODULI)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MULTI_LINEAR_ELASTICITY_STRAINS)


    // D+D- Damage Constitutive laws variables, additional Masonry 2D & 3D
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BACK_STRESS_VECTOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BACK_STRESS_TENSOR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_ONSET_STRESS_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BIAXIAL_COMPRESSION_MULTIPLIER)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FRACTURE_ENERGY_TENSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RESIDUAL_STRESS_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BEZIER_CONTROLLER_C1)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BEZIER_CONTROLLER_C2)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BEZIER_CONTROLLER_C3)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, YIELD_STRAIN_COMPRESSION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TRIAXIAL_COMPRESSION_COEFFICIENT)


    // Adjoint Variables
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ADJOINT_DISPLACEMENT)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ADJOINT_PARTICULAR_DISPLACEMENT)
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ADJOINT_ROTATION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PERTURBATION_SIZE)
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ADJOINT_CURVATURE);
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ADJOINT_STRAIN);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ADAPT_PERTURBATION_SIZE)

    // Variables for output of sensitivities
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CROSS_AREA_SENSITIVITY);
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, POINT_LOAD_SENSITIVITY );
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, I22_SENSITIVITY );
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, I33_SENSITIVITY );
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, THICKNESS_SENSITIVITY );
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, YOUNG_MODULUS_SENSITIVITY);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, AREA_EFFECTIVE_Y_SENSITIVITY);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, AREA_EFFECTIVE_Z_SENSITIVITY);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, IS_ADJOINT );
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, NODAL_DISPLACEMENT_STIFFNESS_SENSITIVITY );
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, NODAL_ROTATIONAL_STIFFNESS_SENSITIVITY );

    // Variables to for computing parts of sensitivity analysis
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TRACED_STRESS_TYPE);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, STRESS_DISP_DERIV_ON_NODE);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, STRESS_DISP_DERIV_ON_GP);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, STRESS_DESIGN_DERIVATIVE_ON_NODE);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, STRESS_DESIGN_DERIVATIVE_ON_GP);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, STRESS_ON_GP);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, STRESS_ON_NODE);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DESIGN_VARIABLE_NAME);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SERIAL_PARALLEL_EQUILIBRIUM_TOLERANCE);

    // for DEM-FEM 2D
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, IMPOSED_Z_STRAIN_OPTION)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, IMPOSED_Z_STRAIN_VALUE)

    // The ratios between the yield strength in the isotropic space and the anisotropic space
    // at each direction in local coordinates ratio_x = ft / ft,x
    KRATOS_REGISTER_IN_PYTHON_SYMMETRIC_3D_TENSOR_VARIABLE_WITH_COMPONENTS(m, ISOTROPIC_ANISOTROPIC_YIELD_RATIO );
    KRATOS_REGISTER_IN_PYTHON_SYMMETRIC_3D_TENSOR_VARIABLE_WITH_COMPONENTS(m, ORTHOTROPIC_ELASTIC_CONSTANTS );
}

}  // namespace Python.
}  // namespace Kratos.

#endif // KRATOS_PYTHON defined
