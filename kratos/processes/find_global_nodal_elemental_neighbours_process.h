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


#if !defined(KRATOS_FIND_GLOBAL_NODAL_ELEMENTAL_NEIGHBOURS_PROCESS_H_INCLUDED )
#define  KRATOS_FIND_GLOBAL_NODAL_ELEMENTAL_NEIGHBOURS_PROCESS_H_INCLUDED



// System includes
#include <string>
#include <iostream>


// External includes


// Project includes
#include "includes/define.h"
#include "processes/process.h"
#include "includes/node.h"
#include "includes/element.h"
#include "includes/model_part.h"
#include "includes/global_pointer_variables.h"
#include "utilities/mpi_coloring_utilities.h"
#include "utilities/pointer_communicator.h"
#include "includes/mpi_serializer.h"
#include "utilities/global_pointer_utilities.h"

namespace Kratos
{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{
typedef  ModelPart::NodesContainerType NodesContainerType;
typedef  ModelPart::ElementsContainerType ElementsContainerType;


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
class FindGlobalNodalNeighboursProcess
    : public Process
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of FindGlobalNodalNeighboursProcess
    KRATOS_CLASS_POINTER_DEFINITION(FindGlobalNodalNeighboursProcess);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    /// the better the guess for the quantities above the less memory occupied and the fastest the algorithm
    FindGlobalNodalNeighboursProcess(const DataCommunicator& rComm, ModelPart& model_part, unsigned int avg_elements)
        : mrComm(rComm),mr_model_part(model_part)
    {
        mavg_elements = avg_elements;
    }

    /// Destructor.
    ~FindGlobalNodalNeighboursProcess() override
    {
    }


    ///@}
    ///@name Operators
    ///@{

    void operator()()
    {
        Execute();
    }


    ///@}
    ///@name Operations
    ///@{

    void Execute() override
    {
        NodesContainerType& rNodes = mr_model_part.Nodes();

        //first of all the neighbour nodes and elements array are initialized to the guessed size
        //and empties the old entries
        for(NodesContainerType::iterator in = rNodes.begin(); in!=rNodes.end(); in++)
        {
            auto& rN = in->GetValue(NEIGHBOUR_ELEMENTS);
            rN = GlobalPointersVector< Element >();
        }

        //compute the complete list of local neighbours
        for(auto& relem : mr_model_part.Elements())
        {
            for(auto& node : relem.GetGeometry())
            {
                node.GetValue(NEIGHBOUR_ELEMENTS).push_back( GlobalPointer<Element>(&*relem));
            }
        }

        if(mrComm.IsDistributed())
        {
            int current_rank = mrComm.Rank();

            typedef std::unordered_map<int, GlobalPointersVector<Element>> neighbours_map_type; //contains id vs vector_of_neighbours 
            typedef std::unordered_map< int, neighbours_map_type > non_local_map_type;

            //construct the lit of nodes that need to be sent
            non_local_map_type non_local_map;
            for(const auto& node : mrModelPart.Nodes())
            {
                int owner_rank = node.FastGetSolutionStepValue(PARTITION_INDEX);
                if(owner_rank != current_rank) //not owned node!
                {
                    non_local_map[owner_rank][node.Id] = node.GetValue(NEIGHBOUR_ELEMENTS);
                }
            }

            //here communicate non local data
            //compute communication plan
            std::vector<int> send_list;
            send_list.reserve( neighbours_ids.size() );
            for(auto& it : non_local_map)
                send_list.push_back( it.first );

            std::sort(send_list.begin(), send_list.end());
            auto colors = CommunicationColoringUtilities::ComputeCommunicationScheduling(send_list, mrComm);

            //finalize computation of neighbour ids on owner nodes
            non_local_map_type recv_map;
            for(int color : colors)
            {
                if(color >= 0)
                {
                    //recev the global neighbours as computed on color
                    recv_map[color] = SendRecv(non_local_map[color], color, color );

                    for(auto& item : recv_map[color])
                    {
                        auto& recv_node = mrModelPart.Nodes()[item.first]; 
                        auto& neighbours = recv_nodes.GetValue(NEIGHBOUR_ELEMENTS);
                        for(auto& gp : item.second)
                            neighbours.push_back(gp);
                    }
                }
            } //after this loop is finished neighbours are ok for the owner nodes

            //fill back the recv_map with the updated information
            for(int color : colors)
            {
                if(color >= 0)
                {
                    for(auto& item : recv_map[color])
                    {
                        auto& recv_node = mrModelPart.Nodes()[item.first]; 
                        item.second = recv_nodes.GetValue(NEIGHBOUR_ELEMENTS);
                    }

                    //obtain the final list of neighbours for nodes owned on color
                    auto final_gp_map = SendRecv(non_local_map[color], color, color );

                    //update the local database
                    for(auto& item : final_gp_map)
                    {
                        auto& recv_node = mrModelPart.Nodes()[item.first]; 
                        recv_nodes.GetValue(NEIGHBOUR_ELEMENTS) = item.second;
                    }
                }
            }
        }

        //reorder by Id
        GlobalPointerCommunicator<Element> pointer_comm(mrComm, gp_list);
        auto id_proxy = pointer_comm.Apply(
                [](GlobalPointer<Element>& gp){return gp->Id();}
        );

        #pragma omp parallel for
        for(int i=0; i<static_cast<int>(mr_model_part.Nodes().size()); ++i)
        {
            auto it = mr_model_part.NodesBegin() + i;
            auto& neighbours = it->GetValue(NEIGHBOUR_ELEMENTS);
            neighbours.shrink_to_fit();
            std::sort(neighbours.ptr_begin(), neighbours.ptr_end(),
                [](GlobalPointer<Node<3>>& gp1, GlobalPointer<Node<3>>& gp2)
                {
                    return id_proxy.Get(gp1) < id_proxy.Get(gp2);
                }
            );
        }
    }

    void ClearNeighbours()
    {
        NodesContainerType& rNodes = mr_model_part.Nodes();
        for(NodesContainerType::iterator in = rNodes.begin(); in!=rNodes.end(); in++)
        {
            auto& rN = in->GetValue(NEIGHBOUR_ELEMENTS);
            rN.erase(rN.begin(),rN.end() );
            rN.shrink_to_fit();
        }
    }

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
    std::string Info() const override
    {
        return "FindGlobalNodalNeighboursProcess";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << "FindGlobalNodalNeighboursProcess";
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
    }


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
    const DataCommunicator& mrComm;
    ModelPart& mr_model_part;
    unsigned int mavg_nodes;


    ///@}
    ///@name Private Operators
    ///@{


    //TODO: remove in favor of generic implementation in communicator
    template< class TDataType>
    TDataType SendRecv(TDataType& send_buffer, int send_rank, int recv_rank)
    {
        MpiSerializer send_serializer;
        send_serializer.save("data",send_buffer);
        std::string send_string = send_serializer.GetStringRepresentation();

        std::string recv_string = mrComm.SendRecv(send_string, send_rank, send_rank);

        MpiSerializer recv_serializer(recv_string);

        TDataType recv_data;
        recv_serializer.load("data",recv_data);
        return recv_data;
    }


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
    FindGlobalNodalNeighboursProcess& operator=(FindGlobalNodalNeighboursProcess const& rOther);

    /// Copy constructor.
    //FindGlobalNodalNeighboursProcess(FindGlobalNodalNeighboursProcess const& rOther);


    ///@}

}; // Class FindGlobalNodalNeighboursProcess

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
inline std::istream& operator >> (std::istream& rIStream,
                                  FindGlobalNodalNeighboursProcess& rThis);

/// output stream function
inline std::ostream& operator << (std::ostream& rOStream,
                                  const FindGlobalNodalNeighboursProcess& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}


}  // namespace Kratos.

#endif // KRATOS_FIND_GLOBAL_NODAL_ELEMENTAL_NEIGHBOURS_PROCESS_H_INCLUDED  defined 


