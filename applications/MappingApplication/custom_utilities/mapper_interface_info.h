//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Philipp Bucher, Jordi Cotela
//
// See Master-Thesis P.Bucher
// "Development and Implementation of a Parallel
//  Framework for Non-Matching Grid Mapping"

#if !defined(KRATOS_MAPPER_INTERFACE_INFO_H_INCLUDED)
#define  KRATOS_MAPPER_INTERFACE_INFO_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "geometries/point.h"
#include "custom_searching/interface_object.h"


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
template<class TDataHolder>
class MapperInterfaceInfo
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of MapperInterfaceInfo
    KRATOS_CLASS_POINTER_DEFINITION(MapperInterfaceInfo);

    using IndexType = std::size_t;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    MapperInterfaceInfo() {}

    MapperInterfaceInfo(const Point rPoint, const IndexType SourceLocalSystemIndex, const int SourceRank=0)
        : mSourceLocalSystemIndex(SourceLocalSystemIndex),
          mSourceRank(SourceRank),
          mCoordinates(rPoint)
    {
    }

    /// Destructor.
    virtual ~MapperInterfaceInfo() {
        std::cout << "Destructor of MapperInterfaceInfo called" << std::endl;
    }


    ///@}
    ///@name Operators
    ///@{


    ///@}
    ///@name Operations
    ///@{

    void ProcessSearchResult(InterfaceObject::Pointer pInterfaceObject)
    {
        mInterfaceData.ProcessSearchResult(pInterfaceObject);
    }

    TDataHolder& GetInterfaceData()
    {
        return mInterfaceData;
    }

    // bool SendBack()
    // {
    //     return mInterfaceData.SendBack();
    // }

    MapperInterfaceInfo::Pointer Create(const Point rPoint, const int SourceLocalSystemIndex, const int SouceRank)
    {
        return Kratos::make_shared<MapperInterfaceInfo<TDataHolder>>(rPoint,
                                                                     SourceLocalSystemIndex,
                                                                     SouceRank);
    }

    void Clear()
    {
        mInterfaceData.Clear();
    }

    bool GetLocalSearchWasSuccessful() const
    {
        return mInterfaceData.GetLocalSearchWasSuccessful();
    }

    IndexType GetLocalSystemIndex() const { return mSourceLocalSystemIndex; }

    int GetSourceRank() const { return mSourceRank; }


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
    virtual std::string Info() const
    {
        return "MapperInterfaceInfo";
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const {}

    /// Print object's data.
    virtual void PrintData(std::ostream& rOStream) const {}


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

    // These variables need serialization
    IndexType mSourceLocalSystemIndex;

    // These variables are NOT being serialized bcs they are not needed after searching!
    int mSourceRank = 0;
    array_1d<double, 3> mCoordinates;
    bool mLocalSearchWasSuccessful = false;

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

    TDataHolder mInterfaceData;


    ///@}
    ///@name Private Operators
    ///@{


    ///@}
    ///@name Private Operations
    ///@{


    ///@}
    ///@name Private  Access
    ///@{

    friend class Serializer;

    virtual void save(Serializer& rSerializer) const
    {
        rSerializer.save("LocalSysIdx", mSourceLocalSystemIndex);
        rSerializer.save("InterfaceData", mInterfaceData);
    }

    virtual void load(Serializer& rSerializer)
    {
        rSerializer.load("LocalSysIdx", mSourceLocalSystemIndex);
        rSerializer.load("InterfaceData", mInterfaceData);
    }

    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    // MapperInterfaceInfo& operator=(MapperInterfaceInfo const& rOther) {}

    /// Copy constructor.
    // MapperInterfaceInfo(MapperInterfaceInfo const& rOther) {}

    ///@}

}; // Class MapperInterfaceInfo

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{

// inline std::istream & operator >> (std::istream& rIStream, MapperInterfaceInfo& rThis);

// /// output stream function
// inline std::ostream & operator << (std::ostream& rOStream, const MapperInterfaceInfo& rThis) {
// //   rThis.PrintInfo(rOStream);
//   rOStream << " : " << std::endl;
// //   rThis.PrintData(rOStream);
//   return rOStream;
// }

// /// output stream function
// inline std::ostream & operator << (std::ostream& rOStream, const std::vector<MapperInterfaceInfo::Pointer>& rThis) {
// //   rThis.PrintInfo(rOStream);
//   rOStream << " : " << std::endl;
// //   rThis.PrintData(rOStream);
//   return rOStream;
// }


///@}

///@} addtogroup block

}  // namespace Kratos.

#endif // KRATOS_MAPPER_INTERFACE_INFO_H_INCLUDED  defined
