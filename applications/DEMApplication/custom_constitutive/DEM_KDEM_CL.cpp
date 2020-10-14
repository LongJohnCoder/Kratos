// System includes
#include <string>
#include <iostream>
#include <cmath>

// Project includes
#include "DEM_KDEM_CL.h"
#include "custom_elements/spheric_continuum_particle.h"

namespace Kratos {

    DEMContinuumConstitutiveLaw::Pointer DEM_KDEM::Clone() const {
        DEMContinuumConstitutiveLaw::Pointer p_clone(new DEM_KDEM(*this));
        return p_clone;
    }

    void DEM_KDEM::SetConstitutiveLawInProperties(Properties::Pointer pProp, bool verbose) {
        KRATOS_INFO("DEM") << "Assigning DEM_KDEM to Properties " << pProp->Id() << std::endl;
        pProp->SetValue(DEM_CONTINUUM_CONSTITUTIVE_LAW_POINTER, this->Clone());
        this->Check(pProp);
    }

    void DEM_KDEM::Check(Properties::Pointer pProp) const {
        DEMContinuumConstitutiveLaw::Check(pProp);

        if(!pProp->Has(ROTATIONAL_MOMENT_COEFFICIENT)) {
            KRATOS_WARNING("DEM")<<std::endl;
            KRATOS_WARNING("DEM")<<"WARNING: Variable ROTATIONAL_MOMENT_COEFFICIENT should be present in the properties when using DEM_KDEM. 0.0 value assigned by default."<<std::endl;
            KRATOS_WARNING("DEM")<<std::endl;
            pProp->GetValue(ROTATIONAL_MOMENT_COEFFICIENT) = 0.0;
        }
    }

    void DEM_KDEM::CalculateContactArea(double radius, double other_radius, double& calculation_area) {

        KRATOS_TRY
        double radius_sum = radius + other_radius;
        double equiv_radius = radius * other_radius / radius_sum;
        calculation_area = Globals::Pi * equiv_radius * equiv_radius;
        KRATOS_CATCH("")
    }

    double DEM_KDEM::CalculateContactArea(double radius, double other_radius, Vector& v) {
        double a = 0.0;
        CalculateContactArea(radius, other_radius, a);
        unsigned int old_size = v.size();
        Vector backup = v;
        v.resize(old_size + 1, false);
        v[old_size] = a;
        for (unsigned int i=0; i<old_size; i++) {
            v[i] = backup[i];
        }
        return a;
    }

    void DEM_KDEM::GetContactArea(const double radius, const double other_radius, const Vector& vector_of_initial_areas, const int neighbour_position, double& calculation_area) {
        if (vector_of_initial_areas.size()) calculation_area = vector_of_initial_areas[neighbour_position];
        else CalculateContactArea(radius, other_radius, calculation_area);
    }

    void DEM_KDEM::CalculateElasticConstants(double& kn_el, double& kt_el, double initial_dist, double equiv_young,
                                             double equiv_poisson, double calculation_area, SphericContinuumParticle* element1, SphericContinuumParticle* element2) {

        KRATOS_TRY

        const double equiv_shear = equiv_young / (2.0 * (1 + equiv_poisson));
        kn_el = equiv_young * calculation_area / initial_dist;
        kt_el = equiv_shear * calculation_area / initial_dist;

        KRATOS_CATCH("")
    }

    void DEM_KDEM::CalculateViscoDampingCoeff(double& equiv_visco_damp_coeff_normal,
                                              double& equiv_visco_damp_coeff_tangential,
                                              SphericContinuumParticle* element1,
                                              SphericContinuumParticle* element2,
                                              const double kn_el,
                                              const double kt_el) {

        KRATOS_TRY

        const double my_mass    = element1->GetMass();
        const double other_mass = element2->GetMass();
        const double equiv_mass = 1.0 / (1.0/my_mass + 1.0/other_mass);

        array_1d<double, 3> other_to_me_vect;
        noalias(other_to_me_vect) = element1->GetGeometry()[0].Coordinates() - element2->GetGeometry()[0].Coordinates();
        const double distance = DEM_MODULUS_3(other_to_me_vect);
        const double norm_distance = (element1->GetRadius() + element2->GetRadius()) / distance; // If spheres are not tangent the Damping coefficient has to be normalized

        const double my_gamma    = element1->GetProperties()[DAMPING_GAMMA];
        const double other_gamma = element2->GetProperties()[DAMPING_GAMMA];
        const double equiv_gamma = 0.5 * (my_gamma + other_gamma);

        equiv_visco_damp_coeff_normal     = 2.0 * equiv_gamma * sqrt(equiv_mass * kn_el) * norm_distance;
        equiv_visco_damp_coeff_tangential = 2.0 * equiv_gamma * sqrt(equiv_mass * kt_el) * norm_distance;

        KRATOS_CATCH("")
    }

    double DEM_KDEM::LocalMaxSearchDistance(const int i,
                                            SphericContinuumParticle* element1,
                                            SphericContinuumParticle* element2) {

        Properties& element1_props = element1->GetProperties();
        Properties& element2_props = element2->GetProperties();
        double mTensionLimit;

        // calculation of equivalent Young modulus
        double myYoung = element1->GetYoung();
        double other_young = element2->GetYoung();
        double equiv_young = 2.0 * myYoung * other_young / (myYoung + other_young);

        const double my_radius = element1->GetRadius();
        const double other_radius = element2->GetRadius();
        double calculation_area = 0;

        Vector& vector_of_contact_areas = element1->GetValue(NEIGHBOURS_CONTACT_AREAS);
        GetContactArea(my_radius, other_radius, vector_of_contact_areas, i, calculation_area);

        double radius_sum = my_radius + other_radius;
        double initial_delta = element1->GetInitialDelta(i);
        double initial_dist = radius_sum - initial_delta;

        // calculation of elastic constants
        double kn_el = equiv_young * calculation_area / initial_dist;

        if (&element1_props == &element2_props) {
            mTensionLimit = GetContactSigmaMax(element1);
        } else {
            mTensionLimit = 0.5 * (GetContactSigmaMax(element1) + GetContactSigmaMax(element2));
        }

        const double Ntstr_el = mTensionLimit * calculation_area;
        double u1 = Ntstr_el / kn_el;
        if (u1 > 2*radius_sum) {u1 = 2*radius_sum;}   // avoid error in special cases with too high tensile
        return u1;
    }

    double DEM_KDEM::GetContactSigmaMax(SphericContinuumParticle* element) {

        KRATOS_TRY

        const double angle_of_internal_friction_in_radians = atan(element->GetFastProperties()->GetContactInternalFricc());
        const double contact_tau_zero = element->GetFastProperties()->GetContactTauZero();

        return (2.0 * contact_tau_zero * cos(angle_of_internal_friction_in_radians)) / (1.0 + sin(angle_of_internal_friction_in_radians));

        KRATOS_CATCH("")
    }

    void DEM_KDEM::CalculateForces(const ProcessInfo& r_process_info,
                                   double OldLocalElasticContactForce[3],
                                   double LocalElasticContactForce[3],
                                   double LocalElasticExtraContactForce[3],
                                   double LocalCoordSystem[3][3],
                                   double LocalDeltDisp[3],
                                   const double kn_el,
                                   const double kt_el,
                                   double& contact_sigma,
                                   double& contact_tau,
                                   double& failure_criterion_state,
                                   double equiv_young,
                                   double equiv_shear,
                                   double indentation,
                                   double calculation_area,
                                   double& acumulated_damage,
                                   SphericContinuumParticle* element1,
                                   SphericContinuumParticle* element2,
                                   int i_neighbour_count,
                                   int time_steps,
                                   bool& sliding,
                                   double &equiv_visco_damp_coeff_normal,
                                   double &equiv_visco_damp_coeff_tangential,
                                   double LocalRelVel[3],
                                   double ViscoDampingLocalContactForce[3]) {

        KRATOS_TRY
        CalculateNormalForces(LocalElasticContactForce,
                kn_el,
                equiv_young,
                indentation,
                calculation_area,
                acumulated_damage,
                element1,
                element2,
                i_neighbour_count,
                time_steps,
            r_process_info);

        CalculateTangentialForces(OldLocalElasticContactForce,
                LocalElasticContactForce,
                LocalElasticExtraContactForce,
                LocalCoordSystem,
                LocalDeltDisp,
                kt_el,
                equiv_shear,
                contact_sigma,
                contact_tau,
                indentation,
                calculation_area,
                failure_criterion_state,
                element1,
                element2,
                i_neighbour_count,
                sliding,
                r_process_info);

        CalculateViscoDampingCoeff(equiv_visco_damp_coeff_normal,
                                   equiv_visco_damp_coeff_tangential,
                                   element1,
                                   element2,
                                   kn_el,
                                   kt_el);

        CalculateViscoDamping(LocalRelVel,
                              ViscoDampingLocalContactForce,
                              indentation,
                              equiv_visco_damp_coeff_normal,
                              equiv_visco_damp_coeff_tangential,
                              sliding,
                              element1->mIniNeighbourFailureId[i_neighbour_count]);

        KRATOS_CATCH("")
    }

    void DEM_KDEM::CalculateNormalForces(double LocalElasticContactForce[3],
            const double kn_el,
            double equiv_young,
            double indentation,
            double calculation_area,
            double& acumulated_damage,
            SphericContinuumParticle* element1,
            SphericContinuumParticle* element2,
            int i_neighbour_count,
            int time_steps,
            const ProcessInfo& r_process_info) {

        KRATOS_TRY

        if (indentation >= 0.0) { //COMPRESSION
            LocalElasticContactForce[2] = kn_el * indentation;
        }
        else { //tension
            int& failure_type = element1->mIniNeighbourFailureId[i_neighbour_count];
            if (failure_type == 0) {
                double mTensionLimit = 0.5 * (GetContactSigmaMax(element1) + GetContactSigmaMax(element2)); //N/m2
                const double limit_force = mTensionLimit * calculation_area;
                LocalElasticContactForce[2] = kn_el * indentation;
                if (fabs(LocalElasticContactForce[2]) > limit_force) {
                    failure_type = 4; //tension failure
                    LocalElasticContactForce[2] = 0.0;
                }
            }
            else {
                LocalElasticContactForce[2] = 0.0;
            }
        }

        KRATOS_CATCH("")
    }

    double DEM_KDEM::GetTauZero(SphericContinuumParticle* element1) {

        return element1->GetFastProperties()->GetContactTauZero();
    }

    double DEM_KDEM::GetInternalFricc(SphericContinuumParticle* element1) {

        return element1->GetFastProperties()->GetContactInternalFricc();
    }

    void DEM_KDEM::CalculateTangentialForces(double OldLocalElasticContactForce[3],
            double LocalElasticContactForce[3],
            double LocalElasticExtraContactForce[3],
            double LocalCoordSystem[3][3],
            double LocalDeltDisp[3],
            const double kt_el,
            const double equiv_shear,
            double& contact_sigma,
            double& contact_tau,
            double indentation,
            double calculation_area,
            double& failure_criterion_state,
            SphericContinuumParticle* element1,
            SphericContinuumParticle* element2,
            int i_neighbour_count,
            bool& sliding,
            const ProcessInfo& r_process_info) {

        KRATOS_TRY

        int& failure_type = element1->mIniNeighbourFailureId[i_neighbour_count];

        LocalElasticContactForce[0] = OldLocalElasticContactForce[0] - kt_el * LocalDeltDisp[0]; // 0: first tangential
        LocalElasticContactForce[1] = OldLocalElasticContactForce[1] - kt_el * LocalDeltDisp[1]; // 1: second tangential

        double ShearForceNow = sqrt(LocalElasticContactForce[0] * LocalElasticContactForce[0]
                                  + LocalElasticContactForce[1] * LocalElasticContactForce[1]);

        if (failure_type == 0) { // This means it has not broken
            //Properties& element1_props = element1->GetProperties();
            //Properties& element2_props = element2->GetProperties();
            if (r_process_info[SHEAR_STRAIN_PARALLEL_TO_BOND_OPTION]) { //TODO: use this only for intact bonds (not broken))
                AddContributionOfShearStrainParallelToBond(OldLocalElasticContactForce, LocalElasticExtraContactForce, element1->mNeighbourElasticExtraContactForces[i_neighbour_count], LocalCoordSystem, kt_el, calculation_area,  element1, element2);
            }

            const double mTauZero = 0.5 * (GetTauZero(element1) + GetTauZero(element2));
            const double mInternalFriction = 0.5 * (GetInternalFricc(element1) + GetInternalFricc(element2));

            contact_tau = ShearForceNow / calculation_area;
            contact_sigma = LocalElasticContactForce[2] / calculation_area;

            double tau_strength = mTauZero;

            if (contact_sigma >= 0) {
                tau_strength = mTauZero + mInternalFriction * contact_sigma;
            }

            if (contact_tau > tau_strength) {
                failure_type = 2; // shear
            }
        }
        else {
            double equiv_tg_of_static_fri_ang = 0.5 * (element1->GetTgOfStaticFrictionAngle() + element2->GetTgOfStaticFrictionAngle());
            double equiv_tg_of_dynamic_fri_ang = 0.5 * (element1->GetTgOfDynamicFrictionAngle() + element2->GetTgOfDynamicFrictionAngle());

            if(equiv_tg_of_static_fri_ang < 0.0 || equiv_tg_of_dynamic_fri_ang < 0.0) {
                KRATOS_ERROR << "The averaged friction is negative for one contact of element with Id: "<< element1->Id()<<std::endl;
            }

            double Frictional_ShearForceMax = equiv_tg_of_static_fri_ang * LocalElasticContactForce[2];
            if (ShearForceNow > Frictional_ShearForceMax) Frictional_ShearForceMax = equiv_tg_of_dynamic_fri_ang * LocalElasticContactForce[2];


            if (Frictional_ShearForceMax < 0.0) {
                Frictional_ShearForceMax = 0.0;
            }

            if ((ShearForceNow > Frictional_ShearForceMax) && (ShearForceNow != 0.0)) {
                LocalElasticContactForce[0] = (Frictional_ShearForceMax / ShearForceNow) * LocalElasticContactForce[0];
                LocalElasticContactForce[1] = (Frictional_ShearForceMax / ShearForceNow) * LocalElasticContactForce[1];
                sliding = true;
            }
        }

        KRATOS_CATCH("")
    }

    void DEM_KDEM::CalculateViscoDamping(double LocalRelVel[3],
                                         double ViscoDampingLocalContactForce[3],
                                         double indentation,
                                         double equiv_visco_damp_coeff_normal,
                                         double equiv_visco_damp_coeff_tangential,
                                         bool& sliding,
                                         int failure_id) {

        KRATOS_TRY

        if ((indentation > 0) || (failure_id == 0)) {
            ViscoDampingLocalContactForce[2] = -equiv_visco_damp_coeff_normal * LocalRelVel[2];
        }
        if (((indentation > 0) || (failure_id == 0)) && (sliding == false)) {
            ViscoDampingLocalContactForce[0] = -equiv_visco_damp_coeff_tangential * LocalRelVel[0];
            ViscoDampingLocalContactForce[1] = -equiv_visco_damp_coeff_tangential * LocalRelVel[1];
        }

        KRATOS_CATCH("")
    }

    void DEM_KDEM::ComputeParticleRotationalMoments(SphericContinuumParticle* element,
                                                    SphericContinuumParticle* neighbor,
                                                    double equiv_young,
                                                    double distance,
                                                    double calculation_area,
                                                    double LocalCoordSystem[3][3],
                                                    double ElasticLocalRotationalMoment[3],
                                                    double ViscoLocalRotationalMoment[3],
                                                    double equiv_poisson,
                                                    double indentation) {

        KRATOS_TRY
        double rotational_moment_coeff = element->GetProperties()[ROTATIONAL_MOMENT_COEFFICIENT];
        //double LocalRotationalMoment[3]     = {0.0};
        double LocalDeltaRotatedAngle[3]    = {0.0};
        double LocalDeltaAngularVelocity[3] = {0.0};

        array_1d<double, 3> GlobalDeltaRotatedAngle;
        noalias(GlobalDeltaRotatedAngle) = element->GetGeometry()[0].FastGetSolutionStepValue(PARTICLE_ROTATION_ANGLE) - neighbor->GetGeometry()[0].FastGetSolutionStepValue(PARTICLE_ROTATION_ANGLE);
        array_1d<double, 3> GlobalDeltaAngularVelocity;
        noalias(GlobalDeltaAngularVelocity) = element->GetGeometry()[0].FastGetSolutionStepValue(ANGULAR_VELOCITY) - neighbor->GetGeometry()[0].FastGetSolutionStepValue(ANGULAR_VELOCITY);

        GeometryFunctions::VectorGlobal2Local(LocalCoordSystem, GlobalDeltaRotatedAngle, LocalDeltaRotatedAngle);
        GeometryFunctions::VectorGlobal2Local(LocalCoordSystem, GlobalDeltaAngularVelocity, LocalDeltaAngularVelocity);
        //GeometryFunctions::VectorGlobal2Local(LocalCoordSystem, mContactMoment, LocalRotationalMoment);

        const double equivalent_radius = sqrt(calculation_area / Globals::Pi);

        const double element_mass  = element->GetMass();
        const double neighbor_mass = neighbor->GetMass();
        const double equiv_mass    = element_mass * neighbor_mass / (element_mass + neighbor_mass);

        AdjustEquivalentYoung(equiv_young, element, neighbor);

        const double equiv_shear   = equiv_young / (2.0 * (1 + equiv_poisson));

        const double Inertia_I     = 0.25 * Globals::Pi * equivalent_radius * equivalent_radius * equivalent_radius * equivalent_radius;
        const double Inertia_J     = 2.0 * Inertia_I; // This is the polar inertia

        const double my_gamma    = element->GetProperties()[DAMPING_GAMMA];
        const double other_gamma = neighbor->GetProperties()[DAMPING_GAMMA];
        const double equiv_gamma = 0.5 * (my_gamma + other_gamma);

        const double k_rot = equiv_young * Inertia_I / distance;
        const double k_tor = equiv_shear * Inertia_J / distance;

        const double norm_distance = (element->GetRadius() + neighbor->GetRadius()) / distance; // If spheres are not tangent the Damping coefficient, DeltaRotatedAngle and DeltaAngularVelocity have to be normalized

        //Viscous parameter taken from Olmedo et al., 'Discrete element model of the dynamic response of fresh wood stems to impact'
        array_1d<double, 3> visc_param;
        const double visc_param_rot = 2.0 * equiv_gamma * sqrt(equiv_mass * k_rot) * norm_distance;
        const double visc_param_tor = 2.0 * equiv_gamma * sqrt(equiv_mass * k_tor) * norm_distance;

        ElasticLocalRotationalMoment[0] = -k_rot * LocalDeltaRotatedAngle[0] * norm_distance;
        ElasticLocalRotationalMoment[1] = -k_rot * LocalDeltaRotatedAngle[1] * norm_distance;
        ElasticLocalRotationalMoment[2] = -k_tor * LocalDeltaRotatedAngle[2] * norm_distance;

        ViscoLocalRotationalMoment[0] = -visc_param_rot * LocalDeltaAngularVelocity[0] * norm_distance;
        ViscoLocalRotationalMoment[1] = -visc_param_rot * LocalDeltaAngularVelocity[1] * norm_distance;
        ViscoLocalRotationalMoment[2] = -visc_param_tor * LocalDeltaAngularVelocity[2] * norm_distance;

        DEM_MULTIPLY_BY_SCALAR_3(ElasticLocalRotationalMoment, rotational_moment_coeff);
        DEM_MULTIPLY_BY_SCALAR_3(ViscoLocalRotationalMoment, rotational_moment_coeff);

        // TODO: Judge if the rotation spring is broken or not
        /*
        double ForceN  = LocalElasticContactForce[2];
        double ForceS  = sqrt(LocalElasticContactForce[0] * LocalElasticContactForce[0] + LocalElasticContactForce[1] * LocalElasticContactForce[1]);
        double MomentS = sqrt(LocalRotaSpringMoment[0] * LocalRotaSpringMoment[0] + LocalRotaSpringMoment[1] * LocalRotaSpringMoment[1]);
        double MomentN = LocalRotaSpringMoment[2];
        // bending stress and axial stress add together, use edge of the bar will failure first
        double TensiMax = -ForceN / calculation_area + MomentS / Inertia_I * equiv_radius;
        double ShearMax =  ForceS / calculation_area + fabs(MomentN) / Inertia_J * equiv_radius;
        if (TensiMax > equiv_tension || ShearMax > equiv_cohesion) {
            mRotaSpringFailureType[i_neighbor_count] = 1;
            LocalRotaSpringMoment[0] = LocalRotaSpringMoment[1] = LocalRotaSpringMoment[2] = 0.0;
            //LocalRotaSpringMoment[1] = 0.0;
            //LocalRotaSpringMoment[2] = 0.0;
        }
        */
        //GeometryFunctions::VectorLocal2Global(LocalCoordSystem, LocalRotationalMoment, mContactMoment);
        KRATOS_CATCH("")
    }//ComputeParticleRotationalMoments

    void DEM_KDEM::AddPoissonContribution(const double equiv_poisson, double LocalCoordSystem[3][3], double& normal_force,
                                          double calculation_area, BoundedMatrix<double, 3, 3>* mSymmStressTensor, SphericContinuumParticle* element1,
                                          SphericContinuumParticle* element2, const ProcessInfo& r_process_info, const int i_neighbor_count, const double indentation) {

        if (!r_process_info[POISSON_EFFECT_OPTION]) return;
        if (element1->mIniNeighbourFailureId[i_neighbor_count] > 0  &&  indentation < 0.0) return;
        if (element1->IsSkin() || element2->IsSkin()) return;
        if (element1->Is(DEMFlags::STICKY) || element2->Is(DEMFlags::STICKY)) return;

        double force[3];
        BoundedMatrix<double, 3, 3> average_stress_tensor = ZeroMatrix(3,3);

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                average_stress_tensor(i,j) = 0.5 * ((*mSymmStressTensor)(i,j) + (*(element2->mSymmStressTensor))(i,j));
            }
        }

        for (int i = 0; i < 3; i++) {

            force[i] = (average_stress_tensor)(i,0) * LocalCoordSystem[0][0] +
                       (average_stress_tensor)(i,1) * LocalCoordSystem[0][1] +
                       (average_stress_tensor)(i,2) * LocalCoordSystem[0][2]; // StressTensor*unitaryNormal0
        }

        double sigma_x = force[0] * LocalCoordSystem[0][0] +
                         force[1] * LocalCoordSystem[0][1] +
                         force[2] * LocalCoordSystem[0][2]; // projection to normal to obtain value of the normal stress

        for (int i = 0; i < 3; i++) {

            force[i] = (average_stress_tensor)(i,0) * LocalCoordSystem[1][0] +
                       (average_stress_tensor)(i,1) * LocalCoordSystem[1][1] +
                       (average_stress_tensor)(i,2) * LocalCoordSystem[1][2]; // StressTensor*unitaryNormal1
        }

        double sigma_y = force[0] * LocalCoordSystem[1][0] +
                         force[1] * LocalCoordSystem[1][1] +
                         force[2] * LocalCoordSystem[1][2]; // projection to normal to obtain value of the normal stress

        double poisson_force = calculation_area * equiv_poisson * (sigma_x + sigma_y);

        normal_force -= poisson_force;

    } //AddPoissonContribution

    void DEM_KDEM::AddContributionOfShearStrainParallelToBond(double OldLocalElasticContactForce[3],
                                                              double LocalElasticExtraContactForce[3],
                                                              array_1d<double, 3>& OldElasticExtraContactForce,
                                                              double LocalCoordSystem[3][3],
                                                              const double kt_el,
                                                              const double calculation_area,
                                                              SphericContinuumParticle* element1,
                                                              SphericContinuumParticle* element2) {

        if (element1->mSymmStressTensor == NULL) return;
        if (element1->IsSkin() || element2->IsSkin()) return;
        if (element1->Is(DEMFlags::STICKY) || element2->Is(DEMFlags::STICKY)) return;

        double average_stress_tensor[3][3];

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                average_stress_tensor[i][j] = 0.5 * ((*(element1->mSymmStressTensor))(i,j) + (*(element2->mSymmStressTensor))(i,j));
            }
        }

        double current_sigma_local[3][3];
        GeometryFunctions::TensorGlobal2Local(LocalCoordSystem, average_stress_tensor, current_sigma_local);

        array_1d<double, 3> OldLocalElasticExtraContactForce;
        GeometryFunctions::VectorGlobal2Local(LocalCoordSystem, OldElasticExtraContactForce, OldLocalElasticExtraContactForce);

        double force_due_to_stress0 = calculation_area * current_sigma_local[0][2];
        double force_due_to_stress1 = calculation_area * current_sigma_local[1][2];

        LocalElasticExtraContactForce[0] = -OldLocalElasticContactForce[0] - force_due_to_stress0;
        LocalElasticExtraContactForce[1] = -OldLocalElasticContactForce[1] - force_due_to_stress1;

        if (fabs(LocalElasticExtraContactForce[0]) > fabs(force_due_to_stress0)) {
            LocalElasticExtraContactForce[0] = LocalElasticExtraContactForce[0] / fabs(LocalElasticExtraContactForce[0]) * fabs(force_due_to_stress0);
        }
        if (fabs(LocalElasticExtraContactForce[1]) > fabs(force_due_to_stress1)) {
            LocalElasticExtraContactForce[1] = LocalElasticExtraContactForce[1] / fabs(LocalElasticExtraContactForce[1]) * fabs(force_due_to_stress1);
        }
    }

    void DEM_KDEM::AdjustEquivalentYoung(double& equiv_young, const SphericContinuumParticle* element, const SphericContinuumParticle* neighbor) {}

} // namespace Kratos
