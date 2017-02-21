//
//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//


// System includes

// External includes
#include <boost/python.hpp>


// Project includes
#include "includes/define.h"
#include "processes/process.h"
#include "python/add_utilities_to_python.h"
#include "utilities/variable_utils.h"
#include "utilities/normal_calculation_utils.h"
#include "utilities/body_normal_calculation_utils.h"
#include "utilities/body_distance_calculation_utils.h"
#include "utilities/signed_distance_calculation_utils.h"
#include "utilities/parallel_levelset_distance_calculator.h"
#include "utilities/openmp_utils.h"
#include "utilities/pointlocation.h"
#include "utilities/deflation_utils.h"
#include "utilities/iso_printer.h"
#include "utilities/activation_utilities.h"
#include "utilities/convect_particles_utilities.h"


// #include "utilities/signed_distance_calculator_bin_based.h"
#include "utilities/divide_elem_utils.h"
#include "utilities/timer.h"

//#include "spatial_containers/bounding_box.h"
#include "utilities/bounding_box_utilities.h"
#include "utilities/binbased_fast_point_locator.h"
#include "utilities/binbased_nodes_in_element_locator.h"
#include "utilities/geometry_tester.h"
#include "utilities/connectivity_preserve_modeler.h"
#include "utilities/cutting_utility.h"

namespace Kratos
{

namespace Python
{


class PythonGenericFunctionUtility
{
    public:
        KRATOS_CLASS_POINTER_DEFINITION(PythonGenericFunctionUtility);
        
        PythonGenericFunctionUtility(  boost::python::object obj )
        {
            mpy_obj = obj;
            muse_local_system = false;
        }
        
        PythonGenericFunctionUtility(  boost::python::object obj, const Matrix& R, const Vector& xc): mpy_obj(obj), muse_local_system(true), mR(R), mxc(xc)
        {
            if(mR.size1() != 3 or mR.size2() != 3)
                KRATOS_ERROR << "rotation matrix is expected to have size 3. The matrix given is " << mR;
            if(mxc.size() != 3)
                KRATOS_ERROR << "center position is expected to have size 3. The position given is " << mxc;
        }
        
        bool UseLocalSystem()
        { return muse_local_system; }

        
        double RotateAndCallFunction(const double x, const double y, const double z, const double t)
        {
            array_1d<double,3> xglobal;
            xglobal[0] = x;
            xglobal[1] = y;
            xglobal[2] = z;
            array_1d<double,3> xlocal = prod(mR, (xglobal - mxc) );
            return CallFunction(xlocal[0],xlocal[1],xlocal[2],t);
        }

        double CallFunction(const double x, const double y, const double z, const double t)
        {
            return boost::python::call_method<double>(mpy_obj.ptr(), "f", x,y,z,t);
        }

        
    private:
        boost::python::object mpy_obj;
        bool muse_local_system = false;
        Matrix mR;
        Vector mxc;
};

class ApplyFunctionToNodesUtility
{
    public:
        KRATOS_CLASS_POINTER_DEFINITION(ApplyFunctionToNodesUtility);
        
        ApplyFunctionToNodesUtility(  ModelPart::NodesContainerType& rNodes, PythonGenericFunctionUtility::Pointer pfunction): mrNodes(rNodes), mpfunction(pfunction)
        {};

        template < class TVarType >
        void ApplyFunction(const TVarType& rVariable, const double t)
        {
            if(mpfunction->UseLocalSystem() == false)
            {
                //WARNING: do NOT put this loop in parallel, the python GIL does not allow you to do it!!
                for (int k = 0; k< static_cast<int> (mrNodes.size()); k++)
                {
                    ModelPart::NodesContainerType::iterator i = mrNodes.begin() + k;
                    const double value = mpfunction->CallFunction(i->X(), i->Y(), i->Z(), t);
                    i->FastGetSolutionStepValue(rVariable) = value;
                }
            }
            else
            {
                //WARNING: do NOT put this loop in parallel, the python GIL does not allow you to do it!!
                for (int k = 0; k< static_cast<int> (mrNodes.size()); k++)
                {
                    ModelPart::NodesContainerType::iterator i = mrNodes.begin() + k;
                    const double value = mpfunction->RotateAndCallFunction(i->X(), i->Y(), i->Z(), t);
                    i->FastGetSolutionStepValue(rVariable) = value;
                }
            }
        }


        std::vector <double> ReturnFunction(const double t)
        {
            std::vector<double> values;
            //WARNING: do NOT put this loop in parallel, the python GIL does not allow you to do it!!
            if(mpfunction->UseLocalSystem() == false)
            {
                for (int k = 0; k< static_cast<int> (mrNodes.size()); k++)
                {
                    ModelPart::NodesContainerType::iterator i = mrNodes.begin() + k;
                    const double value = mpfunction->CallFunction(i->X(), i->Y(), i->Z(), t);
                    values.push_back(value);
                }
            }
            else
            {
                for (int k = 0; k< static_cast<int> (mrNodes.size()); k++)
                {
                    ModelPart::NodesContainerType::iterator i = mrNodes.begin() + k;
                    const double value = mpfunction->RotateAndCallFunction(i->X(), i->Y(), i->Z(), t);
                    values.push_back(value);
                }
            }                

            return values;
        }

    private:
        ModelPart::NodesContainerType& mrNodes;
        PythonGenericFunctionUtility::Pointer mpfunction;
};


void GenerateModelPart(ConnectivityPreserveModeler& GM, ModelPart& origin_model_part, ModelPart& destination_model_part, const char* ElementName, const char* ConditionName)
{
    if( !KratosComponents< Element >::Has( ElementName ) )
        KRATOS_THROW_ERROR(std::invalid_argument, "Element name not found in KratosComponents< Element > -- name is ", ElementName);
    if( !KratosComponents< Condition >::Has( ConditionName ) )
        KRATOS_THROW_ERROR(std::invalid_argument, "Condition name not found in KratosComponents< Condition > -- name is ", ConditionName);

    GM.GenerateModelPart(origin_model_part, destination_model_part,
                         KratosComponents<Element>::Get(ElementName),
                         KratosComponents<Condition>::Get(ConditionName));

}

void AddUtilitiesToPython()
{
    using namespace boost::python;

    // NOTE: this function is special in that it accepts a "pyObject" - this is the reason for which it is defined in this same file
    class_<PythonGenericFunctionUtility,  PythonGenericFunctionUtility::Pointer >("PythonGenericFunctionUtility", init<boost::python::object >() )
    .def(init<boost::python::object, const Matrix&, const Vector&>())
    .def("UseLocalSystem", &PythonGenericFunctionUtility::UseLocalSystem)
    .def("RotateAndCallFunction", &PythonGenericFunctionUtility::RotateAndCallFunction)
    .def("CallFunction", &PythonGenericFunctionUtility::CallFunction)
    ;
    
    class_<ApplyFunctionToNodesUtility >("ApplyFunctionToNodesUtility", init<ModelPart::NodesContainerType&, PythonGenericFunctionUtility::Pointer >() )
    .def("ApplyFunction", &ApplyFunctionToNodesUtility::ApplyFunction< Variable<double> >)
    .def("ApplyFunction", &ApplyFunctionToNodesUtility::ApplyFunction<VariableComponent<VectorComponentAdaptor<array_1d<double, 3> > > >)
    .def("ReturnFunction", &ApplyFunctionToNodesUtility::ReturnFunction)
    ;


    class_<DeflationUtils>("DeflationUtils", init<>())
    .def("VisualizeAggregates",&DeflationUtils::VisualizeAggregates)
    ;


    class_<VariableUtils > ("VariableUtils", init<>())
    .def("SetVectorVar", &VariableUtils::SetVectorVar)
    .def("SetScalarVar", &VariableUtils::SetScalarVar< Variable<double> >)
    .def("SetScalarVar", &VariableUtils::SetScalarVar< VariableComponent< VectorComponentAdaptor<array_1d<double, 3> > > >)
    .def("SaveVectorVar", &VariableUtils::SaveVectorVar)
    .def("SaveScalarVar", &VariableUtils::SaveScalarVar)
    .def("SelectNodeList", &VariableUtils::SelectNodeList)
    .def("CopyVectorVar", &VariableUtils::CopyVectorVar)
    .def("CopyScalarVar", &VariableUtils::CopyScalarVar)
    .def("SetToZero_VectorVar", &VariableUtils::SetToZero_VectorVar)
    .def("SetToZero_ScalarVar", &VariableUtils::SetToZero_ScalarVar)
    // .def("SetToZero_VelocityVectorVar", &VariableUtils::SetToZero_VelocityVectorVar)
    // .def("CheckVariableExists", &VariableUtils::SetToZero_VelocityVectorVar)
    .def("ApplyFixity", &VariableUtils::ApplyFixity< Variable<double> >)
    .def("ApplyFixity", &VariableUtils::ApplyFixity< VariableComponent< VectorComponentAdaptor<array_1d<double, 3> > > > )
    .def("ApplyVector", &VariableUtils::ApplyVector< Variable<double> >)
    .def("ApplyVector", &VariableUtils::ApplyVector< VariableComponent< VectorComponentAdaptor<array_1d<double, 3> > > > )

    .def("SumHistoricalNodeScalarVariable", &VariableUtils::SumHistoricalNodeScalarVariable< Variable<double> > )
    .def("SumHistoricalNodeScalarVariable", &VariableUtils::SumHistoricalNodeScalarVariable< VariableComponent<VectorComponentAdaptor<array_1d<double, 3> > > > )
    .def("SumHistoricalNodeVectorVariable", &VariableUtils::SumHistoricalNodeVectorVariable)

    .def("SumNonHistoricalNodeScalarVariable", &VariableUtils::SumNonHistoricalNodeScalarVariable< Variable<double> > )
    .def("SumNonHistoricalNodeScalarVariable", &VariableUtils::SumNonHistoricalNodeScalarVariable< VariableComponent<VectorComponentAdaptor<array_1d<double, 3> > > > )
    .def("SumNonHistoricalNodeVectorVariable", &VariableUtils::SumNonHistoricalNodeVectorVariable)

    .def("SumConditionScalarVariable", &VariableUtils::SumConditionScalarVariable< Variable<double> > )
    .def("SumConditionScalarVariable", &VariableUtils::SumConditionScalarVariable< VariableComponent<VectorComponentAdaptor<array_1d<double, 3> > > > )
    .def("SumConditionVectorVariable", &VariableUtils::SumConditionVectorVariable)

    .def("SumElementScalarVariable", &VariableUtils::SumElementScalarVariable< Variable<double> > )
    .def("SumElementScalarVariable", &VariableUtils::SumElementScalarVariable< VariableComponent<VectorComponentAdaptor<array_1d<double, 3> > > > )
    .def("SumElementVectorVariable", &VariableUtils::SumElementVectorVariable)
    ;

    // This is required to recognize the different overloads of NormalCalculationUtils::CalculateOnSimplex
    typedef  void (NormalCalculationUtils::*CalcOnSimplexCondType)(NormalCalculationUtils::ConditionsArrayType&,int);
    typedef  void (NormalCalculationUtils::*CalcOnSimplexMPType)(ModelPart&,int);
    typedef  void (NormalCalculationUtils::*CalcOnSimplexWithDoubleVarType)(ModelPart&,int,Variable<double>&);
    typedef  void (NormalCalculationUtils::*CalcOnSimplexWithIntVarType)(ModelPart&,int,Variable<int>&);
//            typedef  void (NormalCalculationUtils::*CalcOnSimplexWithArrayVarType)(ModelPart&,int,Variable< array_1d<double,3> >&,const array_1d<double,3>&);
    typedef  void (NormalCalculationUtils::*CalcOnSimplexWithDoubleVarAlphaType)(ModelPart&,int,Variable<double>&,const double,const double);

    CalcOnSimplexCondType CalcOnSimplex_Cond = &NormalCalculationUtils::CalculateOnSimplex;
    CalcOnSimplexMPType CalcOnSimplex_ModelPart = &NormalCalculationUtils::CalculateOnSimplex;
    CalcOnSimplexWithDoubleVarType CalcOnSimplexWithDoubleVar = &NormalCalculationUtils::CalculateOnSimplex;
    CalcOnSimplexWithIntVarType CalcOnSimplexWithIntVar = &NormalCalculationUtils::CalculateOnSimplex;
    CalcOnSimplexWithDoubleVarAlphaType CalcOnSimplexWithDoubleVarAlpha = &NormalCalculationUtils::CalculateOnSimplex;

    class_<NormalCalculationUtils > ("NormalCalculationUtils", init<>())
    .def("CalculateOnSimplex", CalcOnSimplex_Cond)
    .def("CalculateOnSimplex", CalcOnSimplex_ModelPart)
    .def("CalculateOnSimplex", CalcOnSimplexWithDoubleVar)
    .def("CalculateOnSimplex", CalcOnSimplexWithIntVar)
    .def("CalculateOnSimplex", CalcOnSimplexWithDoubleVarAlpha)
    .def("SwapNormals", &NormalCalculationUtils::SwapNormals)
//                    .def("CalculateOnSimplex", CalcOnSimplexWithArrayVar)
    ;

    class_<BodyNormalCalculationUtils > ("BodyNormalCalculationUtils", init<>())
    .def("CalculateBodyNormals", &BodyNormalCalculationUtils::CalculateBodyNormals)
    ;

    class_<BodyDistanceCalculationUtils > ("BodyDistanceCalculationUtils", init<>())
    .def("CalculateDistances2D", &BodyDistanceCalculationUtils::CalculateDistances < 2 >)
    .def("CalculateDistances3D", &BodyDistanceCalculationUtils::CalculateDistances < 3 >)
    ;

    class_<SignedDistanceCalculationUtils < 2 > >("SignedDistanceCalculationUtils2D", init<>())
    .def("CalculateDistances", &SignedDistanceCalculationUtils < 2 > ::CalculateDistances)
    .def("FindMaximumEdgeSize", &SignedDistanceCalculationUtils < 2 > ::FindMaximumEdgeSize)
    ;

    class_<SignedDistanceCalculationUtils < 3 > >("SignedDistanceCalculationUtils3D", init<>())
    .def("CalculateDistances", &SignedDistanceCalculationUtils < 3 > ::CalculateDistances)
    .def("FindMaximumEdgeSize", &SignedDistanceCalculationUtils < 3 > ::FindMaximumEdgeSize)
    ;

    class_<ParallelDistanceCalculator < 2 >, boost::noncopyable > ("ParallelDistanceCalculator2D", init<>())
    .def("CalculateDistances", &ParallelDistanceCalculator < 2 > ::CalculateDistances)
    .def("CalculateInterfacePreservingDistances", &ParallelDistanceCalculator < 2 > ::CalculateInterfacePreservingDistances)
    .def("CalculateDistancesLagrangianSurface", &ParallelDistanceCalculator < 2 > ::CalculateDistancesLagrangianSurface)
    .def("FindMaximumEdgeSize", &ParallelDistanceCalculator < 2 > ::FindMaximumEdgeSize)
    ;

    class_<ParallelDistanceCalculator < 3 >, boost::noncopyable > ("ParallelDistanceCalculator3D", init<>())
    .def("CalculateDistances", &ParallelDistanceCalculator < 3 > ::CalculateDistances)
    .def("CalculateInterfacePreservingDistances", &ParallelDistanceCalculator < 3 > ::CalculateInterfacePreservingDistances)
    .def("CalculateDistancesLagrangianSurface", &ParallelDistanceCalculator < 3 > ::CalculateDistancesLagrangianSurface)
    .def("FindMaximumEdgeSize", &ParallelDistanceCalculator < 3 > ::FindMaximumEdgeSize)
    ;

    class_<PointLocation > ("PointLocation", init<ModelPart& >())
    .def("Find", &PointLocation::Find)
    .def("Find2D", &PointLocation::Find2D)
    .def("Find3D", &PointLocation::Find3D)
    .def("found", &PointLocation::found)
    .def("ReturnDefaultPointData_scalar", &PointLocation::ReturnDefaultPointData_scalar)
    .def("ReturnDefaultPointData_vector", &PointLocation::ReturnDefaultPointData_vector)
    .def("ReturnCustomPointData_scalar", &PointLocation::ReturnCustomPointData_scalar)
    .def("ReturnCustomPointData_vector", &PointLocation::ReturnCustomPointData_vector)
    ;

    class_<ParticleConvectUtily<2> > ("ParticleConvectUtily2D", init< BinBasedFastPointLocator < 2 >::Pointer >())
    .def("MoveParticles_Substepping", &ParticleConvectUtily<2>::MoveParticles_Substepping)
    .def("MoveParticles_RK4", &ParticleConvectUtily<2>::MoveParticles_RK4)
    ;

    class_<ParticleConvectUtily<3> > ("ParticleConvectUtily3D", init< BinBasedFastPointLocator < 3 >::Pointer >())
    .def("MoveParticles_Substepping", &ParticleConvectUtily<3>::MoveParticles_Substepping)
    .def("MoveParticles_RK4", &ParticleConvectUtily<3>::MoveParticles_RK4)
    ;



    class_<IsosurfacePrinterApplication, boost::noncopyable >
    ("IsosurfacePrinterApplication",
     init<ModelPart& >() )
    .def("AddScalarVarIsosurface", &IsosurfacePrinterApplication::AddScalarVarIsosurface)
    .def("AddScalarVarIsosurfaceAndLower", &IsosurfacePrinterApplication::AddScalarVarIsosurfaceAndLower)
    .def("AddScalarVarIsosurfaceAndHigher", &IsosurfacePrinterApplication::AddScalarVarIsosurfaceAndHigher)
    .def("ClearData", &IsosurfacePrinterApplication::ClearData)
    .def("AddSkinConditions", &IsosurfacePrinterApplication::AddSkinConditions)
    .def("CreateNodesArray", &IsosurfacePrinterApplication::CreateNodesArray)
    ;


    // 	  class_<SignedDistanceCalculationBinBased<2> >("SignedDistanceCalculationBinBased2D", init<>())
    // 			  .def("CalculateDistances",&SignedDistanceCalculationBinBased<2>::CalculateDistances )
    //                           .def("FindMaximumEdgeSize",&SignedDistanceCalculationBinBased<2>::FindMaximumEdgeSize )
    // 			  ;
    //
    // 	  class_<SignedDistanceCalculationBinBased<3> >("SignedDistanceCalculationBinBased3D", init<>())
    // 			  .def("CalculateDistances",&SignedDistanceCalculationBinBased<3>::CalculateDistances )
    //                           .def("FindMaximumEdgeSize",&SignedDistanceCalculationBinBased<3>::FindMaximumEdgeSize )
    // 			  ;

    class_<DivideElemUtils > ("DivideElemUtils", init<>())
    .def("DivideElement_2D", &DivideElemUtils::DivideElement_2D)
    ;

    class_<Timer > ("Timer", init<>())
    .add_property("PrintOnScreen", &Timer::GetPrintOnScreen, &Timer::SetPrintOnScreen)
    .def("Start", &Timer::Start)
    .def("Stop", &Timer::Stop)
    .staticmethod("Start")
    .staticmethod("Stop")
    // 	    .def("PrintTimingInformation",Timer::PrintTimingInformation)
    .def(self_ns::str(self))
    ;




    class_<BoundingBoxUtilities > ("BoundingBoxUtilities", init<ModelPart&, const unsigned int& >())
    .def("Test", &BoundingBoxUtilities::Test)
    ;


    //           class_<SplitElements, boost::noncopyable >
    //                     ("SplitElements", init<ModelPart&, int >() )
    //                     .def("Split", &SplitElements::Split)
    //                     ;


    // 	  def("PrintTimingInformation",Timer::PrintTimingInformation);

    class_<OpenMPUtils > ("OpenMPUtils", init<>())
    .def("SetNumThreads", &OpenMPUtils::SetNumThreads)
    .staticmethod("SetNumThreads")
    .def("GetNumThreads", &OpenMPUtils::GetNumThreads)
    .staticmethod("GetNumThreads")
    .def("PrintOMPInfo", &OpenMPUtils::PrintOMPInfo)
    .staticmethod("PrintOMPInfo")
    ;

    class_< BinBasedFastPointLocator < 2 > > ("BinBasedFastPointLocator2D", init<ModelPart& >())
    .def("UpdateSearchDatabase", &BinBasedFastPointLocator < 2 > ::UpdateSearchDatabase)
    .def("UpdateSearchDatabaseAssignedSize", &BinBasedFastPointLocator < 2 > ::UpdateSearchDatabaseAssignedSize)
    .def("FindPointOnMesh", &BinBasedFastPointLocator < 2 > ::FindPointOnMeshSimplified)
    .def("UpdateSearchDatabaseAssignedSize", &BinBasedFastPointLocator < 2 > ::UpdateSearchDatabaseAssignedSize)
    ;

    class_< BinBasedFastPointLocator < 3 > > ("BinBasedFastPointLocator3D", init<ModelPart&  >())
    .def("UpdateSearchDatabase", &BinBasedFastPointLocator < 3 > ::UpdateSearchDatabase)
    .def("FindPointOnMesh", &BinBasedFastPointLocator < 3 > ::FindPointOnMeshSimplified)
    .def("UpdateSearchDatabaseAssignedSize", &BinBasedFastPointLocator < 3 > ::UpdateSearchDatabaseAssignedSize)
    ;

    class_< BinBasedNodesInElementLocator < 2 > > ("BinBasedNodesInElementLocator2D", init<ModelPart& >())
    .def("UpdateSearchDatabase", &BinBasedNodesInElementLocator < 2 > ::UpdateSearchDatabase)
    .def("FindNodesInElement", &BinBasedNodesInElementLocator < 2 > ::FindNodesInElement)
    .def("UpdateSearchDatabaseAssignedSize", &BinBasedNodesInElementLocator < 2 > ::UpdateSearchDatabaseAssignedSize)
    ;

    class_< BinBasedNodesInElementLocator < 3 > > ("BinBasedNodesInElementLocator3D", init<ModelPart&  >())
    .def("UpdateSearchDatabase", &BinBasedNodesInElementLocator < 3 > ::UpdateSearchDatabase)
    .def("FindNodesInElement", &BinBasedNodesInElementLocator < 3 > ::FindNodesInElement)
    .def("UpdateSearchDatabaseAssignedSize", &BinBasedNodesInElementLocator < 3 > ::UpdateSearchDatabaseAssignedSize)
    ;

    class_< ActivationUtilities > ("ActivationUtilities", init< >())
    .def("ActivateElementsAndConditions", &ActivationUtilities::ActivateElementsAndConditions)
    ;

    class_< GeometryTesterUtility, boost::noncopyable> ("GeometryTesterUtility", init< >())
    .def("RunTest", &GeometryTesterUtility::RunTest)
    .def("TestTriangle2D3N", &GeometryTesterUtility::TestTriangle2D3N)
    .def("TestTriangle2D6N", &GeometryTesterUtility::TestTriangle2D6N)
    .def("TestTetrahedra3D4N", &GeometryTesterUtility::TestTetrahedra3D4N)
    .def("TestTetrahedra3D10N", &GeometryTesterUtility::TestTetrahedra3D10N)
    .def("TestHexahedra3D8N", &GeometryTesterUtility::TestHexahedra3D8N)
    .def("TestHexahedra3D27N", &GeometryTesterUtility::TestHexahedra3D27N)
    .def("TestHexahedra3D20N", &GeometryTesterUtility::TestHexahedra3D20N)
    ;

    class_<ConnectivityPreserveModeler, boost::noncopyable > ("ConnectivityPreserveModeler", init< >())
    .def("GenerateModelPart", GenerateModelPart)
    ;

    class_<CuttingUtility >("CuttingUtility", init< >())
    .def("GenerateCut", &CuttingUtility::GenerateCut)
    .def("UpdateCutData", &CuttingUtility ::UpdateCutData)
    .def("AddSkinConditions", &CuttingUtility ::AddSkinConditions)
    .def("FindSmallestEdge", &CuttingUtility ::FindSmallestEdge)
    ;
}

} // namespace Python.

} // Namespace Kratos
