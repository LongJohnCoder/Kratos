//  KRATOS  _____________
//         /  _/ ____/   |
//         / // / __/ /| |
//       _/ // /_/ / ___ |
//      /___/\____/_/  |_| Application

// System includes
#include "includes/variables.h"

// External includes

// Project includes
#include "truss_element.h"
#include "iga_application_variables.h"

namespace Kratos {
///@name Degrees of freedom
///@{

void TrussElement::EquationIdVector(
    EquationIdVectorType& rResult,
    const ProcessInfo& rCurrentProcessInfo
    ) const
{
    const auto& r_geometry = GetGeometry();
    const SizeType number_of_control_points = r_geometry.size();

    if (rResult.size() != 3 * number_of_control_points)
        rResult.resize(3 * number_of_control_points, false);

    const IndexType pos = r_geometry[0].GetDofPosition(DISPLACEMENT_X);

    for (IndexType i = 0; i < number_of_control_points; ++i) {
        const IndexType index = i * 3;
        rResult[index]     = r_geometry[i].GetDof(DISPLACEMENT_X, pos).EquationId();
        rResult[index + 1] = r_geometry[i].GetDof(DISPLACEMENT_Y, pos + 1).EquationId();
        rResult[index + 2] = r_geometry[i].GetDof(DISPLACEMENT_Z, pos + 2).EquationId();
    }
};

void TrussElement::GetDofList(
    DofsVectorType& rElementalDofList,
    const ProcessInfo& rCurrentProcessInfo
    ) const
{
    const auto& r_geometry = GetGeometry();
    const SizeType number_of_control_points = r_geometry.size();

    rElementalDofList.resize(3 * number_of_control_points);

    for (IndexType i = 0; i < number_of_control_points; ++i) {
        const IndexType index = i * 3;
        rElementalDofList[index]     = r_geometry[i].pGetDof(DISPLACEMENT_X);
        rElementalDofList[index + 1] = r_geometry[i].pGetDof(DISPLACEMENT_Y);
        rElementalDofList[index + 2] = r_geometry[i].pGetDof(DISPLACEMENT_Z);
    }
};

///@}
///@name Analysis stages
///@{

void TrussElement::Initialize(const ProcessInfo& rCurrentProcessInfo)
{
    const double num_integration_points = GetGeometry().IntegrationPointsNumber();
    if (mReferenceBaseVector.size() != num_integration_points) {
        mReferenceBaseVector.resize(num_integration_points);
    }
    for (IndexType point_number = 0; point_number < GetGeometry().IntegrationPointsNumber(); ++point_number) {
        mReferenceBaseVector[point_number] = CalculateActualBaseVector(point_number);
    }

    InitializeMaterial();
}

array_1d<double, 3> TrussElement::CalculateActualBaseVector(
    IndexType IntegrationPointIndex) const
{
    const auto& r_geometry = GetGeometry();
    const Matrix& r_DN_De = r_geometry.ShapeFunctionDerivatives(1, IntegrationPointIndex);

    array_1d<double, 3> actual_base_vector = ZeroVector(3);

    for (IndexType i = 0; i < r_geometry.size(); i++)
    {
        actual_base_vector[0] += r_DN_De(i, 0) * r_geometry[i].X();
        actual_base_vector[1] += r_DN_De(i, 0) * r_geometry[i].Y();
        actual_base_vector[2] += r_DN_De(i, 0) * r_geometry[i].Z();
    }

    return actual_base_vector;
}

void TrussElement::InitializeMaterial()
{
    const GeometryType& r_geometry = GetGeometry();
    const Properties& r_properties = GetProperties();
    const auto& r_N = r_geometry.ShapeFunctionsValues();

    const SizeType r_number_of_integration_points = r_geometry.IntegrationPointsNumber();

    //Constitutive Law initialisation
    if (mConstitutiveLawVector.size() != r_number_of_integration_points)
        mConstitutiveLawVector.resize(r_number_of_integration_points);

    for (IndexType point_number = 0; point_number < r_number_of_integration_points; ++point_number) {
        mConstitutiveLawVector[point_number] = GetProperties()[CONSTITUTIVE_LAW]->Clone();
        mConstitutiveLawVector[point_number]->InitializeMaterial(r_properties, r_geometry, row(r_N, point_number));
    }
}

void TrussElement::CalculateAll(
    MatrixType& rLeftHandSideMatrix,
    VectorType& rRightHandSideVector,
    const ProcessInfo& rCurrentProcessInfo,
    const bool ComputeLeftHandSide,
    const bool ComputeRightHandSide)
{
    const auto& r_geometry = GetGeometry();
    const IndexType nb_dofs = r_geometry.size() * 3;

    // get properties
    const auto& properties = GetProperties();
    const double A = properties[CROSS_AREA];

    // get integration data
    auto& r_integration_points = r_geometry.IntegrationPoints();

    std::vector<double> E_vector(r_integration_points.size());
    CalculateTangentModulus(E_vector, rCurrentProcessInfo);
    for (IndexType point_number = 0; point_number < r_integration_points.size(); ++point_number) {
        const double EA = E_vector[point_number] * A;

        const double& integration_weight = r_integration_points[point_number].Weight();
        const Matrix& r_DN_De = r_geometry.ShapeFunctionDerivatives(1, point_number);

        // compute base vectors
        const array_1d<double, 3> actual_base_vector = CalculateActualBaseVector(point_number);

        const double reference_a = norm_2(mReferenceBaseVector[point_number]);
        const double actual_a = norm_2(actual_base_vector);

        const double actual_aa = actual_a * actual_a;
        const double reference_aa = reference_a * reference_a;

        // green-lagrange strain
        const double e11_membrane = 0.5 * (actual_aa - reference_aa);

        // calculate prestress
        const double prestress = CalculatePrestressPK2(reference_a, actual_a);

        // normal force
        const double s11_membrane = prestress * A + e11_membrane * EA /
            reference_aa;

        for (IndexType r = 0; r < nb_dofs; r++) {
            const IndexType dof_type_r = r % 3;
            const IndexType shape_index_r = r / 3;

            const double epsilon_var_r = actual_base_vector[dof_type_r] *
                r_DN_De(shape_index_r, 0) / reference_aa;

            if (ComputeLeftHandSide) {
                for (IndexType s = 0; s < nb_dofs; s++) {
                    const IndexType dof_type_s = s % 3;
                    const IndexType shape_index_s = s / 3;

                    const double epsilon_var_s =
                        actual_base_vector[dof_type_s] *
                        r_DN_De(shape_index_s, 0) / reference_aa;

                    rLeftHandSideMatrix(r, s) = EA * epsilon_var_r * epsilon_var_s;

                    if (dof_type_r == dof_type_s) {
                        const double epsilon_var_rs =
                            r_DN_De(shape_index_r, 0) *
                            r_DN_De(shape_index_s, 0) / reference_aa;

                        rLeftHandSideMatrix(r, s) += s11_membrane * epsilon_var_rs;
                    }
                }
            }

            if (ComputeRightHandSide) {
                rRightHandSideVector[r] = -s11_membrane * epsilon_var_r;
            }
        }

        if (ComputeLeftHandSide) {
            rLeftHandSideMatrix *= reference_a * integration_weight;
        }

        if (ComputeRightHandSide) {
            rRightHandSideVector *= reference_a * integration_weight;
        }
    }
}

void TrussElement::FinalizeSolutionStep(const ProcessInfo& rCurrentProcessInfo)
{
    const auto& r_geometry = GetGeometry();
    SizeType num_integration_points = r_geometry.IntegrationPointsNumber();
    std::vector<double> green_lagrange_strains(num_integration_points);
    CalculateGreenLagrangeStrain(green_lagrange_strains);
    for (IndexType point_number = 0; point_number < num_integration_points; ++point_number) {
        ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);
        Vector strain = ZeroVector(1);
        Vector stress = ZeroVector(1);
        strain[0] = green_lagrange_strains[point_number];
        Values.SetStrainVector(strain);
        Values.SetStressVector(stress);
        mConstitutiveLawVector[point_number]->FinalizeMaterialResponse(
            Values, ConstitutiveLaw::StressMeasure_PK2);
    }
}

///@}
///@name Internal functions
///@{

/// Computes tangent E
void TrussElement::CalculateTangentModulus(
    std::vector<double>& rTangentModulusVector,
    const ProcessInfo& rCurrentProcessInfo)
{
    const auto& r_geometry = GetGeometry();
    SizeType num_integration_points = r_geometry.IntegrationPointsNumber();
    if (rTangentModulusVector.size() != num_integration_points) {
        rTangentModulusVector.resize(num_integration_points);
    }
    std::vector<double> green_lagrange_strains(num_integration_points);
    CalculateGreenLagrangeStrain(green_lagrange_strains);
    for (IndexType point_number = 0; point_number < num_integration_points; ++point_number) {
        Vector strain_vector = ZeroVector(mConstitutiveLawVector[point_number]->GetStrainSize());
        strain_vector[0] = green_lagrange_strains[point_number];

        ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);
        Values.SetStrainVector(strain_vector);

        mConstitutiveLawVector[point_number]->CalculateValue(Values, TANGENT_MODULUS, rTangentModulusVector[point_number]);
    }
}

void TrussElement::CalculateGreenLagrangeStrain(
    std::vector<double>& rGreenLagrangeVector) const
{
    const auto& r_geometry = GetGeometry();
    auto& r_integration_points = r_geometry.IntegrationPoints();
    SizeType num_integration_points = r_integration_points.size();
    if (rGreenLagrangeVector.size() != num_integration_points) {
        rGreenLagrangeVector.resize(num_integration_points);
    }
    for (IndexType point_number = 0; point_number < num_integration_points; ++point_number) {
        const double l = r_integration_points[point_number].Weight() * norm_2(CalculateActualBaseVector(point_number));
        const double L = r_integration_points[point_number].Weight() * norm_2(mReferenceBaseVector[point_number]);
        rGreenLagrangeVector[point_number] = ((l * l - L * L) / (2.00 * L * L));
    }
}

/// Returns prestress
double TrussElement::CalculatePrestressPK2(double reference_a, double actual_a) const
{
    // pk2 = cauchy * (L / l);
    // No integration weight needs to be considered as this would cut off.
    if (GetProperties().Has(TRUSS_PRESTRESS_PK2)) {
        return GetProperties()[TRUSS_PRESTRESS_PK2];
    }
    else if (GetProperties().Has(TRUSS_PRESTRESS_CAUCHY)) {
        return GetProperties()[TRUSS_PRESTRESS_CAUCHY] * (reference_a / actual_a);
    }
    else {
        return 0.0;
    }
}

/// Returns PK2 stress
void TrussElement::CalculateStressPK2(
    std::vector<double>& rStressVector,
    const ProcessInfo& rCurrentProcessInfo) const
{
    const auto& r_geometry = GetGeometry();
    std::vector<double> green_lagrang_strains(GetGeometry().size());
    CalculateGreenLagrangeStrain(green_lagrang_strains);

    Vector temp_strain = ZeroVector(1);
    Vector temp_stress = ZeroVector(1);

    const double num_integration_points = r_geometry.IntegrationPointsNumber();
    if (rStressVector.size() != num_integration_points) {
        rStressVector.resize(num_integration_points);
    }

    for (IndexType point_number = 0; point_number < num_integration_points; ++point_number) {
        ConstitutiveLaw::Parameters Values(r_geometry, GetProperties(), rCurrentProcessInfo);
        temp_strain[0] = green_lagrang_strains[point_number];
        Values.SetStrainVector(temp_strain);
        Values.SetStressVector(temp_stress);
        mConstitutiveLawVector[point_number]->CalculateMaterialResponse(Values, ConstitutiveLaw::StressMeasure_PK2);

        const double prestress = CalculatePrestressPK2(
            norm_2(mReferenceBaseVector[point_number]), norm_2(CalculateActualBaseVector(point_number)));
        temp_stress[0] += prestress;
        rStressVector[point_number] = temp_stress[0];
    }
}

/// Returns cauchy stress
void TrussElement::CalculateStressCauchy(
    std::vector<double>& rStressVector,
    const ProcessInfo& rCurrentProcessInfo) const
{
    const auto& r_geometry = GetGeometry();
    std::vector<double> green_lagrang_strains(GetGeometry().size());
    CalculateGreenLagrangeStrain(green_lagrang_strains);

    Vector temp_strain = ZeroVector(1);
    Vector temp_stress = ZeroVector(1);

    auto& r_integration_points = r_geometry.IntegrationPoints();
    const double num_integration_points = r_integration_points.size();
    if (rStressVector.size() != num_integration_points) {
        rStressVector.resize(num_integration_points);
    }
    for (IndexType point_number = 0; point_number < num_integration_points; ++point_number) {
        ConstitutiveLaw::Parameters Values(r_geometry, GetProperties(), rCurrentProcessInfo);
        temp_strain[0] = green_lagrang_strains[point_number];
        Values.SetStrainVector(temp_strain);
        Values.SetStressVector(temp_stress);
        mConstitutiveLawVector[point_number]->CalculateMaterialResponse(Values, ConstitutiveLaw::StressMeasure_PK2);

        const double reference_a = norm_2(mReferenceBaseVector[point_number]);
        const double actual_a = norm_2(CalculateActualBaseVector(point_number));
        const double prestress = CalculatePrestressPK2(reference_a, actual_a);

        temp_stress[0] += prestress;

        // No integration weight needs to be considered as this would cut off.
        const double l = actual_a;
        const double L = reference_a;

        temp_stress[0] *= l / L;

        rStressVector[point_number] = temp_stress[0];
    }
}


void TrussElement::GetValuesVector(Vector& rValues, int Step) const
{
    const auto& r_geometry = GetGeometry();
    const IndexType nb_nodes = r_geometry.size();
    if (rValues.size() != nb_nodes * 3) {
        rValues.resize(nb_nodes * 3, false);
    }

    for (int i = 0; i < nb_nodes; ++i) {
        int index = i * 3;
        const auto& disp =
            r_geometry[i].FastGetSolutionStepValue(DISPLACEMENT, Step);

        rValues[index] = disp[0];
        rValues[index + 1] = disp[1];
        rValues[index + 2] = disp[2];
    }
}

void TrussElement::GetFirstDerivativesVector(Vector& rValues, int Step) const
{
    const auto& r_geometry = GetGeometry();
    const IndexType nb_nodes = r_geometry.size();
    if (rValues.size() != nb_nodes * 3) {
        rValues.resize(nb_nodes * 3, false);
    }

    for (int i = 0; i < nb_nodes; ++i) {
        int index = i * 3;
        const auto& vel =
            r_geometry[i].FastGetSolutionStepValue(VELOCITY, Step);

        rValues[index] = vel[0];
        rValues[index + 1] = vel[1];
        rValues[index + 2] = vel[2];
    }
}

void TrussElement::GetSecondDerivativesVector(Vector& rValues, int Step) const
{
    const auto& r_geometry = GetGeometry();
    const IndexType nb_nodes = r_geometry.size();
    if (rValues.size() != nb_nodes * 3) {
        rValues.resize(nb_nodes * 3, false);
    }

    for (int i = 0; i < nb_nodes; ++i) {
        int index = i * 3;
        const auto& acc =
            r_geometry[i].FastGetSolutionStepValue(ACCELERATION, Step);

        rValues[index] = acc[0];
        rValues[index + 1] = acc[1];
        rValues[index + 2] = acc[2];
    }
}

///@}
///@name Output functions
///@{

void TrussElement::CalculateOnIntegrationPoints(
    const Variable<double>& rVariable,
    std::vector<double>& rOutput,
    const ProcessInfo& rCurrentProcessInfo)
{
    const auto& integration_points = GetGeometry().IntegrationPoints();

    if (rOutput.size() != integration_points.size()) {
        rOutput.resize(integration_points.size());
    }

    if (rVariable == TRUSS_GREEN_LAGRANGE_STRAIN) {
        CalculateGreenLagrangeStrain(rOutput);
    }
    else if (rVariable == TANGENT_MODULUS) {
        CalculateTangentModulus(rOutput, rCurrentProcessInfo);
    }
    else if (rVariable == TRUSS_STRESS_PK2) {
        CalculateStressPK2(rOutput, rCurrentProcessInfo);
    }
    else if (rVariable == TRUSS_STRESS_CAUCHY) {
        CalculateStressCauchy(rOutput, rCurrentProcessInfo);
    }
    else if (rVariable == TRUSS_FORCE) {
        CalculateStressCauchy(rOutput, rCurrentProcessInfo);
        const double A = GetProperties()[CROSS_AREA];
        for (IndexType i = 0; i < rOutput.size(); ++i) {
            rOutput[i] *= A;
        }
    }
}

///@}
///@name Info
///@{

/// Check provided parameters
int TrussElement::Check(const ProcessInfo& rCurrentProcessInfo) const
{
    KRATOS_TRY
    const double numerical_limit = std::numeric_limits<double>::epsilon();

    KRATOS_ERROR_IF((GetGeometry().WorkingSpaceDimension() != 3) || (GetGeometry().size() != 2))
        << "The truss element works only in 3D and with 2 noded elements" << std::endl;

    // verify that the variables are correctly initialized
    KRATOS_ERROR_IF(DISPLACEMENT.Key() == 0) << "DISPLACEMENT has Key zero! Check if the application is "
        "registered properly." << std::endl;
    KRATOS_ERROR_IF(CROSS_AREA.Key() == 0) << "CROSS_AREA has Key zero! Check if the application is "
        "registered properly." << std::endl;

    // verify that the dofs exist
    for (IndexType i = 0; i < GetGeometry().size(); ++i) {
        if (GetGeometry()[i].SolutionStepsDataHas(DISPLACEMENT) == false) {
            KRATOS_ERROR << "missing variable DISPLACEMENT on node "
                << GetGeometry()[i].Id() << std::endl;
        }
        if (GetGeometry()[i].HasDofFor(DISPLACEMENT_X) == false ||
            GetGeometry()[i].HasDofFor(DISPLACEMENT_Y) == false ||
            GetGeometry()[i].HasDofFor(DISPLACEMENT_Z) == false) {
            KRATOS_ERROR
                << "missing one of the dofs for the variable DISPLACEMENT on node "
                << GetGeometry()[i].Id() << std::endl;
        }
    }

    KRATOS_ERROR_IF(!GetProperties().Has(CROSS_AREA) ||
        GetProperties()[CROSS_AREA] <= numerical_limit)
        << "Please provide a reasonable value for \"CROSS_AREA\" for element #"
        << Id() << std::endl;

    KRATOS_ERROR_IF(!GetProperties().Has(YOUNG_MODULUS) ||
        GetProperties()[YOUNG_MODULUS] <= numerical_limit)
        << "Please provide a reasonable value for \"YOUNG_MODULUS\" for element #"
        << Id() << std::endl;

    KRATOS_ERROR_IF(!GetProperties().Has(DENSITY) ||
        GetProperties()[DENSITY] <= numerical_limit)
        << "Please provide a reasonable value for \"DENSITY\" for element #"
        << Id() << ". Provided density: " << GetProperties()[DENSITY] << std::endl;

    KRATOS_ERROR_IF(!GetProperties().Has(POISSON_RATIO))
        << "\"POISSON_RATIO\" not provided for element #" << Id() << std::endl;

    return 0;

    KRATOS_CATCH("")
}

///@}
} // namespace Kratos