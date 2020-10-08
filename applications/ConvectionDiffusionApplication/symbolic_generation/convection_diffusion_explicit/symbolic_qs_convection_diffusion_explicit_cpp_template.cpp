//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main author:     Riccardo Tosi
//

#include "symbolic_qs_convection_diffusion_explicit.h"

namespace Kratos
{

/***********************************************************************************/
/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::SymbolicQSConvectionDiffusionExplicit(
    IndexType NewId,
    GeometryType::Pointer pGeometry)
    : Element(NewId, pGeometry) {}

/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::SymbolicQSConvectionDiffusionExplicit(
    IndexType NewId,
    GeometryType::Pointer pGeometry,
    Properties::Pointer pProperties)
    : Element(NewId, pGeometry, pProperties) {}

/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::~SymbolicQSConvectionDiffusionExplicit() {}

/***********************************************************************************/
/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
Element::Pointer SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::Create(
    IndexType NewId,
    NodesArrayType const& ThisNodes,
    Properties::Pointer pProperties) const
{
    return Kratos::make_intrusive<SymbolicQSConvectionDiffusionExplicit>(NewId, this->GetGeometry().Create(ThisNodes), pProperties);
}

/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
Element::Pointer SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::Create(
    IndexType NewId,
    GeometryType::Pointer pGeom,
    Properties::Pointer pProperties) const
{
    return Kratos::make_intrusive<SymbolicQSConvectionDiffusionExplicit>(NewId, pGeom, pProperties);
}

/***********************************************************************************/
/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
void SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::EquationIdVector(
    EquationIdVectorType& rResult,
    const ProcessInfo& rCurrentProcessInfo) const
{
    KRATOS_TRY;

    ConvectionDiffusionSettings::Pointer p_settings = rCurrentProcessInfo.GetValue(CONVECTION_DIFFUSION_SETTINGS);
    const Variable<double>& r_unknown_var = p_settings->GetUnknownVariable();
    unsigned int local_size = GetGeometry().PointsNumber();
    if(rResult.size() != local_size)
        rResult.resize(local_size);
    for (unsigned int i=0; i<local_size; i++)
    {
        rResult[i] = GetGeometry()[i].GetDof(r_unknown_var).EquationId();
    }

    KRATOS_CATCH("");
}

/***********************************************************************************/
/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
void SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::GetDofList(
    DofsVectorType& rElementalDofList,
    const ProcessInfo& rCurrentProcessInfo) const
{
    KRATOS_TRY;

    ConvectionDiffusionSettings::Pointer p_settings = rCurrentProcessInfo.GetValue(CONVECTION_DIFFUSION_SETTINGS);
    const Variable<double>& r_unknown_var = p_settings->GetUnknownVariable();
    unsigned int local_size = GetGeometry().PointsNumber();
    if (rElementalDofList.size() != local_size)
        rElementalDofList.resize(local_size);
    for (unsigned int i = 0; i < local_size; i++)
    {
        rElementalDofList[i] = GetGeometry()[i].pGetDof(r_unknown_var);
    }

    KRATOS_CATCH("");
}

/***********************************************************************************/
/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
void SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::AddExplicitContribution(
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    const ProcessInfo& r_process_info = rCurrentProcessInfo;
    auto& r_geometry = GetGeometry();
    const unsigned int local_size = r_geometry.size();
    BoundedVector<double, TNumNodes> rhs;
    this->CalculateRightHandSideInternal(rhs,rCurrentProcessInfo);
    // Add the residual contribution
    // Note that the reaction is indeed the formulation residual
    const auto& reaction_variable = r_process_info[CONVECTION_DIFFUSION_SETTINGS]->GetReactionVariable();
    for (unsigned int i_node = 0; i_node < local_size; i_node++) {
        #pragma omp atomic
        r_geometry[i_node].FastGetSolutionStepValue(reaction_variable) += rhs[i_node];
    }

    KRATOS_CATCH("");
}

/***********************************************************************************/
/***********************************************************************************/

template <>
void SymbolicQSConvectionDiffusionExplicit<2,3>::CalculateMassMatrix(
    MatrixType& rMassMatrix,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    const unsigned int local_size = 3;
    // Resize, intialize and fill the mass matrix for linear triangular elements
    if (rMassMatrix.size1() != local_size)
        rMassMatrix.resize(local_size, local_size, false);
    noalias(rMassMatrix) = ZeroMatrix(local_size, local_size);
    const double one_six = 1.0 / 6.0;
    const double one_twelve = 1.0 / 12.0;
    rMassMatrix(0,0) = one_six; rMassMatrix(0,1) = one_twelve; rMassMatrix(0,2) = one_twelve;
    rMassMatrix(1,0) = one_twelve; rMassMatrix(1,1) = one_six; rMassMatrix(1,2) = one_twelve;
    rMassMatrix(2,0) = one_twelve; rMassMatrix(2,1) = one_twelve; rMassMatrix(2,2) = one_six;
    // Assumption all the Gauss points have the same weight, so we multiply by the volume
    rMassMatrix *= GetGeometry().Area();

    KRATOS_CATCH("");
}

/***********************************************************************************/

template <>
void SymbolicQSConvectionDiffusionExplicit<3,4>::CalculateMassMatrix(
    MatrixType &rMassMatrix,
    const ProcessInfo &rCurrentProcessInfo)
{
    KRATOS_TRY;

    const unsigned int local_size = 4;
    // Resize, intialize and fill the mass matrix for linear tetrahedral elements
    if (rMassMatrix.size1() != local_size)
        rMassMatrix.resize(local_size, local_size, false);
    noalias(rMassMatrix) = ZeroMatrix(local_size, local_size);
    const double one_ten = 0.1;
    const double one_twenty = 0.05;
    rMassMatrix(0,0) = one_ten; rMassMatrix(0,1) = one_twenty; rMassMatrix(0,2) = one_twenty; rMassMatrix(0,3) = one_twenty;
    rMassMatrix(1,0) = one_twenty; rMassMatrix(1,1) = one_ten; rMassMatrix(1,2) = one_twenty; rMassMatrix(1,3) = one_twenty;
    rMassMatrix(2,0) = one_twenty; rMassMatrix(2,1) = one_twenty; rMassMatrix(2,2) = one_ten; rMassMatrix(2,3) = one_twenty;
    rMassMatrix(3,0) = one_twenty; rMassMatrix(3,1) = one_twenty; rMassMatrix(3,2) = one_twenty; rMassMatrix(3,3) = one_ten;
    // Assumption all the Gauss points have the same weight, so we multiply by the volume
    rMassMatrix *= GetGeometry().Volume();

    KRATOS_CATCH("");
}

/***********************************************************************************/
/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
void SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::Calculate(
    const Variable<double>& rVariable,
    double& Output,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    if (rVariable == SCALAR_PROJECTION) {
        auto& r_geometry = GetGeometry();
        const unsigned int local_size = r_geometry.size();
        BoundedVector<double, TNumNodes> rhs_oss;
        this->CalculateOrthogonalSubgridScaleRHSInternal(rhs_oss,rCurrentProcessInfo);
        for (unsigned int i_node = 0; i_node < local_size; i_node++) {
            #pragma omp atomic
            r_geometry[i_node].GetValue(rVariable) += rhs_oss[i_node];
        }
    }
    else {
        BaseType::Calculate(rVariable,Output,rCurrentProcessInfo);
    }

    KRATOS_CATCH("");
}

/***********************************************************************************/
/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
void SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::InitializeEulerianElement(
    ElementData& rData,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    const ProcessInfo& r_process_info = rCurrentProcessInfo;
    ConvectionDiffusionSettings::Pointer p_settings = r_process_info[CONVECTION_DIFFUSION_SETTINGS];

    // Getting data for the given geometry and integration method
    const auto& r_geometry = GetGeometry();
    const unsigned int local_size = r_geometry.size();
    array_1d<double,TNumNodes> N_aux;
    GeometryUtils::CalculateGeometryData(r_geometry,rData.DN_DX,N_aux,rData.volume);
    rData.N_gausspoint = r_geometry.ShapeFunctionsValues(this->GetIntegrationMethod());

    // Initialize some scalar data
    rData.lumping_factor = 1.00 / double(TNumNodes);
    rData.diffusivity = 0.0;
    rData.dynamic_tau = rCurrentProcessInfo[DYNAMIC_TAU];

    for(unsigned int node_element = 0; node_element<local_size; node_element++) {
        // Observations
        // * SGS time derivative term approximated as (phi-phi_old)/(delta_time_coefficient*delta_time)
        //   observe that for RK step = 1 ASGS time derivative term = 0 because phi = phi_old (null acceleration wrt step n)
        // * convective velocity and forcing term:
        //   RK step 1: evaluated at previous time step
        //   RK steps 2 and 3: linear interpolation between current and oldprevious time step
        //   RK step 4: evaluated at current time step
        // * convective_velocity = velocity - velocity_mesh
        //   velocity_mesh = 0 in eulerian framework
        if (r_process_info.GetValue(RUNGE_KUTTA_STEP)==1) {
            rData.delta_time_coefficient = std::numeric_limits<double>::max();
            rData.forcing[node_element] = r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVolumeSourceVariable(),1);
            rData.convective_velocity(node_element,0) = r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable(),1)[0] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable(),1)[0];
            rData.convective_velocity(node_element,1) = r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable(),1)[1] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable(),1)[1];
            rData.convective_velocity(node_element,2) = r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable(),1)[2] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable(),1)[2];
        }
        else if (r_process_info.GetValue(RUNGE_KUTTA_STEP)==2 || r_process_info.GetValue(RUNGE_KUTTA_STEP)==3) {
            rData.delta_time_coefficient = 0.5;
            rData.forcing[node_element] = 0.5*(r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVolumeSourceVariable()) + r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVolumeSourceVariable(),1));
            rData.convective_velocity(node_element,0) = 0.5*(r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable(),1)[0] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable(),1)[0] + r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable())[0] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable())[0]);
            rData.convective_velocity(node_element,1) = 0.5*(r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable(),1)[1] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable(),1)[1] + r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable())[1] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable())[1]);
            rData.convective_velocity(node_element,2) = 0.5*(r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable(),1)[2] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable(),1)[2] + r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable())[2] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable())[2]);
        }
        else {
            rData.delta_time_coefficient = 1.0;
            rData.forcing[node_element] = r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVolumeSourceVariable());
            rData.convective_velocity(node_element,0) = r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable())[0] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable())[0];
            rData.convective_velocity(node_element,1) = r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable())[1] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable())[1];
            rData.convective_velocity(node_element,2) = r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetVelocityVariable())[2] - r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetMeshVelocityVariable())[2];
        }
        rData.oss_projection[node_element] = r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetProjectionVariable());
        rData.diffusivity += r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetDiffusionVariable());
        rData.delta_time = r_process_info[DELTA_TIME];
        rData.unknown[node_element] = r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetUnknownVariable());
        rData.unknown_old[node_element] = r_geometry[node_element].FastGetSolutionStepValue(p_settings->GetUnknownVariable(),1);
    }
    // divide by number of nodes scalar data
    rData.diffusivity *= rData.lumping_factor;

    KRATOS_CATCH("");
}

/***********************************************************************************/
/***********************************************************************************/

template <>
void SymbolicQSConvectionDiffusionExplicit<2,3>::CalculateRightHandSideInternal(
    BoundedVector<double, 3>& rRightHandSideVector,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    // Element data
    ElementData rData;
    this->InitializeEulerianElement(rData,rCurrentProcessInfo);

    // Compute tau
    this->CalculateTau(rData);

    // Retrieve element data
    const auto& alpha = rData.diffusivity;
    const auto& f = rData.forcing;
    const auto& phi = rData.unknown;
    const auto& phi_old = rData.unknown_old;
    const auto& delta_time = rData.delta_time;
    const auto& delta_time_coefficient = rData.delta_time_coefficient;
    const auto& v = rData.convective_velocity;
    const auto& tau = rData.tau;
    const auto& prj = rData.oss_projection;
    // Hardcoded shape functions gradients for linear triangular element
    // This is explicitly done to minimize the matrix acceses
    // The notation DN_i_j means shape function for node i in dimension j
    const double& DN_DX_0_0 = rData.DN_DX(0, 0);
    const double& DN_DX_0_1 = rData.DN_DX(0, 1);
    const double& DN_DX_1_0 = rData.DN_DX(1, 0);
    const double& DN_DX_1_1 = rData.DN_DX(1, 1);
    const double& DN_DX_2_0 = rData.DN_DX(2, 0);
    const double& DN_DX_2_1 = rData.DN_DX(2, 1);
    // RHS
    auto& rhs = rData.rhs;

    //substitute_rhs_2D

    const double local_size = 3;
    noalias(rRightHandSideVector) = rhs * rData.volume/local_size;

    KRATOS_CATCH("");
}

/***********************************************************************************/

template <>
void SymbolicQSConvectionDiffusionExplicit<3,4>::CalculateRightHandSideInternal(
    BoundedVector<double, 4>& rRightHandSideVector,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    // Element data
    ElementData rData;
    this->InitializeEulerianElement(rData,rCurrentProcessInfo);

    // Compute tau
    this->CalculateTau(rData);

    // Retrieve element data
    const auto& alpha = rData.diffusivity;
    const auto& f = rData.forcing;
    const auto& phi = rData.unknown;
    const auto& phi_old = rData.unknown_old;
    const auto& delta_time = rData.delta_time;
    const auto& delta_time_coefficient = rData.delta_time_coefficient;
    const auto& v = rData.convective_velocity;
    const auto& tau = rData.tau;
    const auto& prj = rData.oss_projection;
    // Hardcoded shape functions gradients for linear triangular element
    // This is explicitly done to minimize the matrix acceses
    // The notation DN_i_j means shape function for node i in dimension j
    const double& DN_DX_0_0 = rData.DN_DX(0,0);
    const double& DN_DX_0_1 = rData.DN_DX(0,1);
    const double& DN_DX_0_2 = rData.DN_DX(0,2);
    const double& DN_DX_1_0 = rData.DN_DX(1,0);
    const double& DN_DX_1_1 = rData.DN_DX(1,1);
    const double& DN_DX_1_2 = rData.DN_DX(1,2);
    const double& DN_DX_2_0 = rData.DN_DX(2,0);
    const double& DN_DX_2_1 = rData.DN_DX(2,1);
    const double& DN_DX_2_2 = rData.DN_DX(2,2);
    const double& DN_DX_3_0 = rData.DN_DX(3,0);
    const double& DN_DX_3_1 = rData.DN_DX(3,1);
    const double& DN_DX_3_2 = rData.DN_DX(3,2);
    // RHS
    auto& rhs = rData.rhs;

    //substitute_rhs_3D

    const double local_size = 4;
    noalias(rRightHandSideVector) = rhs * rData.volume/local_size;

    KRATOS_CATCH("");
}

/***********************************************************************************/
/***********************************************************************************/

template <>
void SymbolicQSConvectionDiffusionExplicit<2,3>::CalculateOrthogonalSubgridScaleRHSInternal(
    BoundedVector<double, 3>& rRightHandSideVector,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    // Element data
    ElementData rData;
    this->InitializeEulerianElement(rData,rCurrentProcessInfo);

    // Compute tau
    this->CalculateTau(rData);

    // Retrieve element data
    const auto& alpha = rData.diffusivity;
    const auto& f = rData.forcing;
    const auto& phi = rData.unknown;
    const auto& phi_old = rData.unknown_old;
    const auto& delta_time = rData.delta_time;
    const auto& delta_time_coefficient = rData.delta_time_coefficient;
    const auto& v = rData.convective_velocity;
    // Hardcoded shape functions gradients for linear triangular element
    // This is explicitly done to minimize the matrix acceses
    // The notation DN_i_j means shape function for node i in dimension j
    const double& DN_DX_0_0 = rData.DN_DX(0, 0);
    const double& DN_DX_0_1 = rData.DN_DX(0, 1);
    const double& DN_DX_1_0 = rData.DN_DX(1, 0);
    const double& DN_DX_1_1 = rData.DN_DX(1, 1);
    const double& DN_DX_2_0 = rData.DN_DX(2, 0);
    const double& DN_DX_2_1 = rData.DN_DX(2, 1);
    // RHS
    auto& rhs = rData.rhs;

    //substitute_oss_2D

    const double local_size = 3;
    noalias(rRightHandSideVector) = rhs * rData.volume/local_size;

    KRATOS_CATCH("");
}

/***********************************************************************************/

template <>
void SymbolicQSConvectionDiffusionExplicit<3,4>::CalculateOrthogonalSubgridScaleRHSInternal(
    BoundedVector<double, 4>& rRightHandSideVector,
    const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    // Element data
    ElementData rData;
    this->InitializeEulerianElement(rData,rCurrentProcessInfo);

    // Compute tau
    this->CalculateTau(rData);

    // Retrieve element data
    const auto& alpha = rData.diffusivity;
    const auto& f = rData.forcing;
    const auto& phi = rData.unknown;
    const auto& phi_old = rData.unknown_old;
    const auto& delta_time = rData.delta_time;
    const auto& delta_time_coefficient = rData.delta_time_coefficient;
    const auto& v = rData.convective_velocity;
    // Hardcoded shape functions gradients for linear triangular element
    // This is explicitly done to minimize the matrix acceses
    // The notation DN_i_j means shape function for node i in dimension j
    const double& DN_DX_0_0 = rData.DN_DX(0,0);
    const double& DN_DX_0_1 = rData.DN_DX(0,1);
    const double& DN_DX_0_2 = rData.DN_DX(0,2);
    const double& DN_DX_1_0 = rData.DN_DX(1,0);
    const double& DN_DX_1_1 = rData.DN_DX(1,1);
    const double& DN_DX_1_2 = rData.DN_DX(1,2);
    const double& DN_DX_2_0 = rData.DN_DX(2,0);
    const double& DN_DX_2_1 = rData.DN_DX(2,1);
    const double& DN_DX_2_2 = rData.DN_DX(2,2);
    const double& DN_DX_3_0 = rData.DN_DX(3,0);
    const double& DN_DX_3_1 = rData.DN_DX(3,1);
    const double& DN_DX_3_2 = rData.DN_DX(3,2);
    // RHS
    auto& rhs = rData.rhs;

    //substitute_oss_3D

    const double local_size = 4;
    noalias(rRightHandSideVector) = rhs * rData.volume/local_size;

    KRATOS_CATCH("");
}

/***********************************************************************************/
/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
double SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::ComputeH(
    BoundedMatrix<double,TNumNodes,TDim >& DN_DX)
{
    KRATOS_TRY;

    double h=0.0;
    for(unsigned int i=0; i<TNumNodes; i++)
    {
        double h_inv = 0.0;
        for(unsigned int k=0; k<TDim; k++)
        {
            h_inv += DN_DX(i,k)*DN_DX(i,k);
        }
        h += 1.0/h_inv;
    }
    h = sqrt(h)/static_cast<double>(TNumNodes);
    return h;

    KRATOS_CATCH("");
}

/***********************************************************************************/
/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
void SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::CalculateTau(
    ElementData& rData)
{
    KRATOS_TRY;

    // Calculate h
    double h = this->ComputeH(rData.DN_DX);
    // Calculate tau for each gauss point
    for(unsigned int g = 0; g<TNumNodes; g++) {
	const auto& N = row(rData.N_gausspoint,g);
        // Calculate velocity and velocity divergence in the gauss point
        const array_1d<double,3> vel_gauss = prod(N, rData.convective_velocity);
        double div_vel = 0;
        for(unsigned int node_element = 0; node_element<TNumNodes; node_element++) {
            for(unsigned int dim = 0; dim < TDim; dim++) {
                div_vel += rData.DN_DX(node_element,dim)*rData.convective_velocity(node_element,dim);
            }
        }
        const double norm_velocity = norm_2(vel_gauss);
        // Estimate tau
        // Dynamic part
        double inv_tau = rData.dynamic_tau/rData.delta_time;
        // Convection
        inv_tau += 2.0 * norm_velocity / h;
        inv_tau += 1.0*div_vel; // unitary coefficient in front of \nabla \cdot convective_velocity term in the strong equation
        // Diffusion
        inv_tau += 4.0 * rData.diffusivity / (h*h);
        // Limiting
        inv_tau = std::max(inv_tau, 1e-2);
        rData.tau[g] = (1.0) / inv_tau;
    }

    KRATOS_CATCH("");
}

/***********************************************************************************/
/***********************************************************************************/

template< unsigned int TDim, unsigned int TNumNodes >
Element::IntegrationMethod SymbolicQSConvectionDiffusionExplicit<TDim,TNumNodes>::GetIntegrationMethod() const
{
    return GeometryData::GI_GAUSS_2;
}

/***********************************************************************************/
/***********************************************************************************/

template class SymbolicQSConvectionDiffusionExplicit<2,3>;
template class SymbolicQSConvectionDiffusionExplicit<3,4>;

/***********************************************************************************/
/***********************************************************************************/

}
