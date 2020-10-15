//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Miguel Maso Sotomayor
//

// System includes


// External includes


// Project includes
#include "algebraic_flux_correction_utility.h"
#include "shallow_water_application_variables.h"
#include "processes/calculate_nodal_area_process.h"
#include "processes/find_nodal_neighbours_process.h"
#include "utilities/variable_utils.h"

namespace Kratos
{
    AlgebraicFluxCorrectionUtility::AlgebraicFluxCorrectionUtility(
        ModelPart& rModelPart,
        Parameters ThisParameters) :
        mrModelPart(rModelPart)
    {
        ThisParameters.ValidateAndAssignDefaults(this->GetDefaultParameters());
        mRebuildLevel = ThisParameters["rebuild_level"].GetInt();
        mLimitingVariableName = ThisParameters["limiting_variable"].GetString();

        Check();
        InitializeNonhistoricalVariables();
        ResizeNodalAndElementalVectors();
        const auto& r_data_communicator = mrModelPart.GetCommunicator().GetDataCommunicator();
        FindGlobalNodalElementalNeighboursProcess(r_data_communicator, mrModelPart).Execute();
        GetElementalDofList();
        AssembleElementalMassMatrices();
    }

    void AlgebraicFluxCorrectionUtility::ExecuteInitializeLowOrderStep()
    {
        if (mRebuildLevel > 0) {
            ResizeNodalAndElementalVectors();
            const auto& r_data_communicator = mrModelPart.GetCommunicator().GetDataCommunicator();
            FindGlobalNodalElementalNeighboursProcess(r_data_communicator, mrModelPart).Execute();
            GetElementalDofList();
            AssembleElementalMassMatrices();
        }
        SetProcessInfoLowOrderFlags();
    }

    void AlgebraicFluxCorrectionUtility::ExecuteFinalizeLowOrderStep()
    {
        const Variable<double>& limiting_variable = KratosComponents<Variable<double>>::Get(mLimitingVariableName);
        ComputeLimitingVariables();
        GetLowOrderValues(limiting_variable);
    }

    void AlgebraicFluxCorrectionUtility::ExecuteInitializeHighOrderStep()
    {
        SetProcessInfoHighOrderFlags();
    }

    void AlgebraicFluxCorrectionUtility::ExecuteFinalizeHighOrderStep()
    {
        const Variable<double>& limiting_variable = KratosComponents<Variable<double>>::Get(mLimitingVariableName);
        GetHighOrderValues(limiting_variable);
        ComputeLimitingVariables();
        ComputeElementalAlgebraicFluxCorrections();
        ComputeLimiters(limiting_variable);
        AssembleLimitedCorrections();
    }

    void AlgebraicFluxCorrectionUtility::SetProcessInfoHighOrderFlags()
    {
        ProcessInfo& r_process_info = mrModelPart.GetProcessInfo();
        r_process_info.GetValue(LUMPED_MASS_FACTOR) = 1.0;
        r_process_info.GetValue(IS_MONOTONIC_CALCULATION) = false;
    }

    void AlgebraicFluxCorrectionUtility::SetProcessInfoLowOrderFlags()
    {
        ProcessInfo& r_process_info = mrModelPart.GetProcessInfo();
        r_process_info.GetValue(LUMPED_MASS_FACTOR) = 0.0;
        r_process_info.GetValue(IS_MONOTONIC_CALCULATION) = true;
    }

    void AlgebraicFluxCorrectionUtility::GetHighOrderValues(const Variable<double>& rLimitingVariable)
    {
        // Elemental values
        #pragma omp parallel for
        for (size_t i = 0; i < mrModelPart.NumberOfElements(); ++i)
        {
            Vector& r_element_values_vector = *(mHighOrderValues.begin() + i);
            (mrModelPart.ElementsBegin() + i)->GetValuesVector(r_element_values_vector, 0);
        }

        // Nodal values
        #pragma omp parallel for
        for (size_t i = 0; i < mrModelPart.NumberOfNodes(); ++i)
        {
            double& r_nodal_value = *(mNodalHighOrderValues.begin() + i);
            r_nodal_value = (mrModelPart.NodesBegin() + i)->FastGetSolutionStepValue(rLimitingVariable);
        }
    }

    void AlgebraicFluxCorrectionUtility::GetLowOrderValues(const Variable<double>& rLimitingVariable)
    {
        // Elemental values
        #pragma omp parallel for
        for (size_t i = 0; i < mrModelPart.NumberOfElements(); ++i)
        {
            Vector& r_element_values_vector = *(mLowOrderValues.begin() + i);
            (mrModelPart.ElementsBegin() + i)->GetValuesVector(r_element_values_vector, 0);
        }

        // Nodal values
        #pragma omp parallel for
        for (size_t i = 0; i < mrModelPart.NumberOfNodes(); ++i)
        {
            double& r_nodal_value = *(mNodalLowOrderValues.begin() + i);
            r_nodal_value = (mrModelPart.NodesBegin() + i)->FastGetSolutionStepValue(rLimitingVariable);

            Vector& lo_values = *(mDofsLowOrderValues.begin() + i);
            const auto& dofs = (mrModelPart.NodesBegin() + i)->GetDofs();
            lo_values.resize(dofs.size(), false);
            for (size_t d = 0; d < dofs.size(); ++d)
            {
                lo_values[d] = (*dofs[d])(0);
            }
        }
    }

    void AlgebraicFluxCorrectionUtility::GetElementalPreviousValues()
    {
        #pragma omp parallel for
        for (size_t i = 0; i < mrModelPart.NumberOfElements(); ++i)
        {
            Vector& r_element_values_vector = *(mPreviousValues.begin() + i);
            (mrModelPart.ElementsBegin() + i)->GetValuesVector(r_element_values_vector, 1);
        }
    }

    void AlgebraicFluxCorrectionUtility::ComputeElementalAlgebraicFluxCorrections()
    {
        #pragma omp parallel for
        for (size_t i = 0; i < mrModelPart.NumberOfElements(); ++i)
        {
            auto& corrections = *(mAlgebraicFluxCorrections.begin() + i);
            auto ho_values = *(mHighOrderValues.begin() + i);
            auto lo_values = *(mLowOrderValues.begin() + i);
            auto mass_matrix = *(mElementalMassMatrices.begin() + i);
            corrections.resize(ho_values.size(), false);
            for (size_t j = 0; j < ho_values.size(); ++j)
            {
                corrections[j] = (ho_values[j] - lo_values[j]) * mass_matrix[j];
            }
        }
    }

    void AlgebraicFluxCorrectionUtility::AssembleElementalMassMatrices()
    {
        const auto& r_process_info = mrModelPart.GetProcessInfo();
        const size_t domain_size = r_process_info.GetValue(DOMAIN_SIZE);
        CalculateNodalAreaProcess<CalculateNodalAreaSettings::SaveAsNonHistoricalVariable>(mrModelPart, domain_size).Execute();

        #pragma omp parallel for
        for (size_t i = 0; i < mrModelPart.NumberOfElements(); ++i)
        {
            auto it_elem = mrModelPart.ElementsBegin() + i;
            Matrix mass_matrix;
            it_elem->CalculateMassMatrix(mass_matrix, r_process_info);
            const size_t local_size = mass_matrix.size1();
            const size_t number_of_nodes = it_elem->GetGeometry().PointsNumber();
            const size_t block_size = local_size / number_of_nodes;
            const auto& r_geom = it_elem->GetGeometry();
            Vector lumped_mass_matrix = ZeroVector(local_size);
            for (size_t i_node = 0; i_node < number_of_nodes; ++i_node)
            {
                for (size_t i_dof = 0; i_dof < block_size; ++ i_dof)
                {
                    const size_t row = i_node*block_size + i_dof;
                    for (size_t col = 0; col < local_size; ++col)
                    {
                        lumped_mass_matrix[row] += mass_matrix(row, col);
                    }
                    lumped_mass_matrix[row] /= r_geom[i_node].GetValue(NODAL_AREA);
                }
            }
            *(mElementalMassMatrices.begin() + i) = lumped_mass_matrix;
        }
    }

    void AlgebraicFluxCorrectionUtility::ComputeLimiters(const Variable<double>& rVariable)
    {
        // first step: Get the nodal positive and negative contributions
        const Vector nodal_contributions = mNodalHighOrderValues - mNodalLowOrderValues;
        const size_t number_of_nodes = mrModelPart.NumberOfNodes();
        Vector nodal_positive_contributions(number_of_nodes);
        Vector nodal_negative_contributions(number_of_nodes);
        #pragma omp parallel for
        for (size_t i = 0; i < number_of_nodes; ++i)
        {
            auto it_node = mrModelPart.NodesBegin() + i;
            const double aec = *(nodal_contributions.begin() + i);
            it_node->GetValue(ALGEBRAIC_CONTRIBUTION) = aec;
            *(nodal_positive_contributions.begin() + i) = std::max(0.0, aec);
            *(nodal_negative_contributions.begin() + i) = std::min(0.0, aec);

            const double u_n = it_node->FastGetSolutionStepValue(rVariable,1);
            const double u_l = *(mNodalLowOrderValues.begin() + i);
            const double u_max = std::max(u_l, u_n);
            const double u_min = std::min(u_l, u_n);
            it_node->GetValue(MAXIMUM_VALUE) = u_max;
            it_node->GetValue(MINIMUM_VALUE) = u_min;
        }

        // second step: get the maximum and minimum increments. This requires a previous loop over elements.
        const size_t number_of_elements = mrModelPart.NumberOfElements();
        #pragma omp parallel for
        for (size_t i = 0; i < number_of_elements; ++i)
        {
            auto it_elem = mrModelPart.ElementsBegin() + i;
            auto& geom = it_elem->GetGeometry();
            double u_max = geom[0].GetValue(MAXIMUM_VALUE);
            double u_min = geom[0].GetValue(MINIMUM_VALUE);
            for (size_t j = 1; j < geom.PointsNumber(); ++j)
            {
                u_max = std::max(u_max, geom[j].GetValue(MAXIMUM_VALUE));
                u_min = std::min(u_min, geom[j].GetValue(MINIMUM_VALUE));
            }
            it_elem->GetValue(MAXIMUM_VALUE) = u_max;
            it_elem->GetValue(MINIMUM_VALUE) = u_min;
        }

        #pragma omp parallel for
        for (size_t i = 0; i < number_of_nodes; ++i)
        {
            auto it_node = mrModelPart.NodesBegin() + i;
            auto neighbor_elems = it_node->GetValue(NEIGHBOUR_ELEMENTS);
            double u_max = neighbor_elems[0].GetValue(MAXIMUM_VALUE);
            double u_min = neighbor_elems[0].GetValue(MINIMUM_VALUE);
            for (size_t j = 1; j < neighbor_elems.size(); ++j)
            {
                u_max = std::max(u_max, neighbor_elems[j].GetValue(MAXIMUM_VALUE));
                u_min = std::min(u_min, neighbor_elems[j].GetValue(MINIMUM_VALUE));
            }
            const double u_l = *(mNodalLowOrderValues.begin() + i);
            const double nodal_max_increment = u_max - u_l;
            const double nodal_min_increment = u_min - u_l;

            // using the maximum/minimum increments to compute the positive/negative ratios
            const double positive_contribution = *(nodal_positive_contributions.begin() + i);
            if (positive_contribution > mEpsilon) {
                it_node->GetValue(POSITIVE_RATIO) = std::min(1.0, nodal_max_increment / positive_contribution);
            } else {
                it_node->GetValue(POSITIVE_RATIO) = 0.0;
            }
            const double negative_contribution = *(nodal_negative_contributions.begin() + i);
            if (negative_contribution < -mEpsilon) {
                it_node->GetValue(NEGATIVE_RATIO) = std::min(1.0, nodal_min_increment / negative_contribution);
            } else {
                it_node->GetValue(NEGATIVE_RATIO) = 0.0;
            }
        }

        // third step: get the elemental limiter
        #pragma omp parallel for
        for (size_t i = 0; i < mrModelPart.NumberOfElements(); ++i)
        {
            auto it_elem = mrModelPart.ElementsBegin() + i;
            auto& geom = it_elem->GetGeometry();
            double c = 1.0;
            for (auto& r_node : geom)
            {
                if (r_node.GetValue(ALGEBRAIC_CONTRIBUTION) > 0.0){
                    c = std::min(c, r_node.GetValue(POSITIVE_RATIO));
                } else {
                    c = std::min(c, r_node.GetValue(NEGATIVE_RATIO));
                }
            }
            *(mElementalLimiters.begin() + i) = c;

            it_elem->SetValue(AUX_INDEX, c);
        }
    }

    void AlgebraicFluxCorrectionUtility::AssembleLimitedCorrections()
    {
        #pragma omp parallel for
        for (int i = 0; i < static_cast<int>(mrModelPart.NumberOfNodes()); ++i)
        {
            const Vector& lo_values = *(mDofsLowOrderValues.begin() + i);
            auto& dofs = (mrModelPart.NodesBegin() + i)->GetDofs();
            for (size_t d = 0; d < dofs.size(); ++d)
            {
                (*dofs[d])(0) = lo_values[d];
            }
        }

        #pragma omp parallel for
        for (int i = 0; i < static_cast<int>(mrModelPart.NumberOfElements()); ++i)
        {
            const Vector& corrections = *(mAlgebraicFluxCorrections.begin() + i);
            double limiter = *(mElementalLimiters.begin() + i);
            DofsVectorType dofs = *(mElementalDofs.begin() + i);
            for (size_t d = 0; d < dofs.size(); ++d)
            {
                #pragma omp atomic
                (*dofs[d])(0) += limiter * corrections[d];
            }
        }
    }

    void AlgebraicFluxCorrectionUtility::ComputeLimitingVariables()
    {
    }

    void AlgebraicFluxCorrectionUtility::InitializeNonhistoricalVariables()
    {
        VariableUtils().SetNonHistoricalVariableToZero(MAXIMUM_VALUE, mrModelPart.Nodes());
        VariableUtils().SetNonHistoricalVariableToZero(MINIMUM_VALUE, mrModelPart.Nodes());
        VariableUtils().SetNonHistoricalVariableToZero(POSITIVE_RATIO, mrModelPart.Nodes());
        VariableUtils().SetNonHistoricalVariableToZero(NEGATIVE_RATIO, mrModelPart.Nodes());
        VariableUtils().SetNonHistoricalVariableToZero(ALGEBRAIC_CONTRIBUTION, mrModelPart.Nodes());
        VariableUtils().SetNonHistoricalVariableToZero(MAXIMUM_VALUE, mrModelPart.Elements());
        VariableUtils().SetNonHistoricalVariableToZero(MINIMUM_VALUE, mrModelPart.Elements());
    }

    void AlgebraicFluxCorrectionUtility::ResizeNodalAndElementalVectors()
    {
        size_t number_of_elements = mrModelPart.NumberOfElements();
        size_t number_of_nodes = mrModelPart.NumberOfNodes();
        mHighOrderValues.resize(number_of_elements);
        mLowOrderValues.resize(number_of_elements);
        mPreviousValues.resize(number_of_elements);
        mNodalHighOrderValues.resize(number_of_nodes);
        mNodalLowOrderValues.resize(number_of_nodes);
        mAlgebraicFluxCorrections.resize(number_of_elements);
        mElementalMassMatrices.resize(number_of_elements);
        mElementalLimiters.resize(number_of_elements);
        mElementalDofs.resize(number_of_elements);
        mDofsLowOrderValues.resize(number_of_nodes);
    }

    const Parameters AlgebraicFluxCorrectionUtility::GetDefaultParameters() const
    {
        const Parameters default_parameters = Parameters(R"(
        {
            "name"              : "algebraic_flux_correction_utility",
            "rebuild_level"     : 0,
            "limiting_variable" : "VARIABLE_NAME"
        })");
        return default_parameters;
    }

    int AlgebraicFluxCorrectionUtility::Check()
    {
        const auto domain_size = mrModelPart.GetProcessInfo().GetValue(DOMAIN_SIZE);
        KRATOS_ERROR_IF(((domain_size != 2) && (domain_size != 3))) << "Domain size must be 2 or 3" << std::endl;
        return 1;
    }

    void AlgebraicFluxCorrectionUtility::GetElementalDofList()
    {
        const ProcessInfo& r_process_info = mrModelPart.GetProcessInfo();
        #pragma omp parallel for
        for (size_t i = 0; i < mrModelPart.NumberOfElements(); ++i)
        {
            (mrModelPart.ElementsBegin() + i)->GetDofList(*(mElementalDofs.begin() + i), r_process_info);
        }
    }
}
