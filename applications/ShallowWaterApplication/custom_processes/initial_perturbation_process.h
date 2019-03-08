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

#ifndef KRATOS_INITIAL_PERTURBATION_PROCESS_H_INCLUDED
#define KRATOS_INITIAL_PERTURBATION_PROCESS_H_INCLUDED


// System includes
#include <iostream>


// External includes


// Project includes
#include "includes/define.h"
#include "includes/model_part.h"
#include "processes/process.h"
#include "geometries/point.h"
#include "includes/kratos_parameters.h"
#include "shallow_water_application_variables.h"


namespace Kratos
{
///@addtogroup ApplicationNameApplication
///@{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/// Short class definition.
/** Detail class definition.
*/
class KRATOS_API(SHALLOW_WATER_APPLICATION) InitialPerturbationProcess : public Process
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of InitialPerturbationProcess
    KRATOS_CLASS_POINTER_DEFINITION(InitialPerturbationProcess);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    InitialPerturbationProcess(
        ModelPart& rThisModelPart,
        Parameters& rThisParameters);

    /// Destructor.
    ~InitialPerturbationProcess() override {}

    ///@}
    ///@name Operators
    ///@{

    /*
     * @brief This operator is provided to call the process as a function and simply calls the Execute method.
     */
    void operator()()
    {
        Execute();
    }

    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief Execute method is used to execute the Process algorithms.
     */
    void Execute() override;

    /**
     * @brief Perform a check with the parameters.
     */
    int Check() override;

    /**
     * @brief this function is designed for being execute once before the solution loop but
     * after all of the solvers where built
     */
    void ExecuteBeforeSolutionLoop() override;

    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    virtual std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "InitialPerturbationProcess" ;
        return buffer.str();
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const override {rOStream << "InitialPerturbationProcess";}

    /// Print object's data.
    virtual void PrintData(std::ostream& rOStream) const override {}


    ///@}
    ///@name Friends
    ///@{


    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{


    ///@}
    ///@name Protected member Variables
    ///@{


    ///@}
    ///@name Protected Operators
    ///@{


    ///@}
    ///@name Protected Operations
    ///@{


    ///@}
    ///@name Protected  Access
    ///@{


    ///@}
    ///@name Protected Inquiry
    ///@{


    ///@}
    ///@name Protected LifeCycle
    ///@{


    ///@}

private:
    ///@name Static Member Variables
    ///@{


    ///@}
    ///@name Member Variables
    ///@{

    ModelPart& mrModelPart;
    Variable<double> mVariable = FREE_SURFACE_ELEVATION;
    double mDefaultValue;
    // Point mSourcePoint;
    double mInfluenceDistance;
    double mPerturbation;

    ///@}
    ///@name Private Operators
    ///@{


    ///@}
    ///@name Private Operations
    ///@{


    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    // InitialPerturbationProcess& operator=(InitialPerturbationProcess const& rOther);

    /// Copy constructor.
    // InitialPerturbationProcess(InitialPerturbationProcess const& rOther);


    ///@}

}; // Class InitialPerturbationProcess

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
inline std::istream& operator >> (std::istream& rIStream,
                InitialPerturbationProcess& rThis);

/// output stream function
inline std::ostream& operator << (std::ostream& rOStream,
                const InitialPerturbationProcess& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}

///@} addtogroup block

}  // namespace Kratos.

#endif // KRATOS_INITIAL_PERTURBATION_PROCESS_H_INCLUDED  defined
