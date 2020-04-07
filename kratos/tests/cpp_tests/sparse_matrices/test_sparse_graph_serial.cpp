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

#include <utility>
#include <iostream>
#include <random>

#include "testing/testing.h"
#include "containers/sparse_graph.h"
#include "containers/sparse_contiguous_row_graph.h"
#include "includes/key_hash.h"
#include "utilities/openmp_utils.h"

namespace Kratos {
namespace Testing {

typedef std::vector<std::vector<std::size_t>> ElementConnectivityType;
typedef std::unordered_map<std::pair<std::size_t, std::size_t>,
                          double,
                          PairHasher<std::size_t, std::size_t>,
                          PairComparor<std::size_t, std::size_t>
                          > MatrixMapType;


ElementConnectivityType RandomElementConnectivities(
    const IndexType block_size,
    const IndexType nodes_in_elem,
    const IndexType nel,
    const IndexType ndof,
    const IndexType standard_dev
)
{

    std::cout << std::endl;
    std::cout << "beginning generation" << std::endl;
    double start = OpenMPUtils::GetCurrentTime();
    //generating random indices
    ElementConnectivityType connectivities(nel*block_size);

    #pragma omp parallel for
    for(int i=0; i<static_cast<int>(nel);++i)
    {
        connectivities[i].resize(nodes_in_elem*block_size);
        std::mt19937 gen(i);
        //std::uniform_int_distribution<> dis(0,ndof-1);
        std::normal_distribution<> dis{
            static_cast<double>(ndof/nel*i),
            static_cast<double>(standard_dev)
            };

        for(int j = 0; j<static_cast<int>(nodes_in_elem); ++j){
            //IndexType eq_id = dis(gen)*block_size;
            IndexType eq_id;
            bool acceptable = false;
            while(!acceptable){
                auto randomid = static_cast<IndexType>(dis(gen));
                if(static_cast<IndexType>(randomid) > 0 &&
                   static_cast<IndexType>(randomid) < ndof-1)
                {
                    acceptable=true;
                    eq_id = randomid * block_size;
                }
            }

            for(IndexType k = 0; k<block_size; ++k){
                connectivities[i][j*block_size+k] = eq_id+k;
            }
        }
    }
    double end_gen = OpenMPUtils::GetCurrentTime();
    std::cout << "finishing generation - time = " << end_gen-start << std::endl;

    return connectivities;
}

ElementConnectivityType ElementConnectivities()
{
    ElementConnectivityType connectivities = {
        {19,11,7,39},
        {33,27,22,9},
        {11,2,3,6},
        {8,26,3,22},
        {0,26,5,31},
        {1,18,35,12},
        {3,36,23,7},
        {16,8,18,15},
        {16,33,10,26},
        {25,2,18,31},
        {33,26,4,6},
        {19,21,22,7},
        {9,37,29,14},
        {18,19,14,39},
        {24,34,37,7},
        {16,9,29,14},
        {17,18,11,4},
        {16,33,28,37},
        {37,26,11,5},
        {8,26,35,14},
        {24,4,30,15},
        {16,17,12,6},
        {32,25,35,28},
        {24,25,14,1},
        {24,35,5,6},
        {28,12,38,15},
        {8,18,35,6},
        {28,31,22,39},
        {1,28,13,7},
        {17,10,36,7},
        {25,14,30,9},
        };

    return connectivities;
}

MatrixMapType GetReferenceMatrixAsMap()
{
    MatrixMapType AMap{{{19,19},3.0},
        {{19,11},1.0},{{19,7},2.0},{{19,39},2.0},{{11,19},1.0},{{11,11},4.0},{{11,7},1.0},{{11,39},1.0},{{7,19},2.0},{{7,11},1.0},{{7,7},6.0},
        {{7,39},1.0},{{39,19},2.0},{{39,11},1.0},{{39,7},1.0},{{39,39},3.0},{{33,33},4.0},{{33,27},1.0},{{33,22},1.0},{{33,9},1.0},{{27,33},1.0},
        {{27,27},1.0},{{27,22},1.0},{{27,9},1.0},{{22,33},1.0},{{22,27},1.0},{{22,22},4.0},{{22,9},1.0},{{9,33},1.0},{{9,27},1.0},{{9,22},1.0},
        {{9,9},4.0},{{11,2},1.0},{{11,3},1.0},{{11,6},1.0},{{2,11},1.0},{{2,2},2.0},{{2,3},1.0},{{2,6},1.0},{{3,11},1.0},{{3,2},1.0},
        {{3,3},3.0},{{3,6},1.0},{{6,11},1.0},{{6,2},1.0},{{6,3},1.0},{{6,6},5.0},{{8,8},4.0},{{8,26},2.0},{{8,3},1.0},{{8,22},1.0},
        {{26,8},2.0},{{26,26},6.0},{{26,3},1.0},{{26,22},1.0},{{3,8},1.0},{{3,26},1.0},{{3,22},1.0},{{22,8},1.0},{{22,26},1.0},{{22,3},1.0},
        {{0,0},1.0},{{0,26},1.0},{{0,5},1.0},{{0,31},1.0},{{26,0},1.0},{{26,5},2.0},{{26,31},1.0},{{5,0},1.0},{{5,26},2.0},{{5,5},3.0},
        {{5,31},1.0},{{31,0},1.0},{{31,26},1.0},{{31,5},1.0},{{31,31},3.0},{{1,1},3.0},{{1,18},1.0},{{1,35},1.0},{{1,12},1.0},{{18,1},1.0},
        {{18,18},6.0},{{18,35},2.0},{{18,12},1.0},{{35,1},1.0},{{35,18},2.0},{{35,35},5.0},{{35,12},1.0},{{12,1},1.0},{{12,18},1.0},{{12,35},1.0},
        {{12,12},3.0},{{3,36},1.0},{{3,23},1.0},{{3,7},1.0},{{36,3},1.0},{{36,36},2.0},{{36,23},1.0},{{36,7},2.0},{{23,3},1.0},{{23,36},1.0},
        {{23,23},1.0},{{23,7},1.0},{{7,3},1.0},{{7,36},2.0},{{7,23},1.0},{{16,16},5.0},{{16,8},1.0},{{16,18},1.0},{{16,15},1.0},{{8,16},1.0},
        {{8,18},2.0},{{8,15},1.0},{{18,16},1.0},{{18,8},2.0},{{18,15},1.0},{{15,16},1.0},{{15,8},1.0},{{15,18},1.0},{{15,15},3.0},{{16,33},2.0},
        {{16,10},1.0},{{16,26},1.0},{{33,16},2.0},{{33,10},1.0},{{33,26},2.0},{{10,16},1.0},{{10,33},1.0},{{10,10},2.0},{{10,26},1.0},{{26,16},1.0},
        {{26,33},2.0},{{26,10},1.0},{{25,25},4.0},{{25,2},1.0},{{25,18},1.0},{{25,31},1.0},{{2,25},1.0},{{2,18},1.0},{{2,31},1.0},{{18,25},1.0},
        {{18,2},1.0},{{18,31},1.0},{{31,25},1.0},{{31,2},1.0},{{31,18},1.0},{{33,4},1.0},{{33,6},1.0},{{26,4},1.0},{{26,6},1.0},{{4,33},1.0},
        {{4,26},1.0},{{4,4},3.0},{{4,6},1.0},{{6,33},1.0},{{6,26},1.0},{{6,4},1.0},{{19,21},1.0},{{19,22},1.0},{{21,19},1.0},{{21,21},1.0},
        {{21,22},1.0},{{21,7},1.0},{{22,19},1.0},{{22,21},1.0},{{22,7},1.0},{{7,21},1.0},{{7,22},1.0},{{9,37},1.0},{{9,29},2.0},{{9,14},3.0},
        {{37,9},1.0},{{37,37},4.0},{{37,29},1.0},{{37,14},1.0},{{29,9},2.0},{{29,37},1.0},{{29,29},2.0},{{29,14},2.0},{{14,9},3.0},{{14,37},1.0},
        {{14,29},2.0},{{14,14},6.0},{{18,19},1.0},{{18,14},1.0},{{18,39},1.0},{{19,18},1.0},{{19,14},1.0},{{14,18},1.0},{{14,19},1.0},{{14,39},1.0},
        {{39,18},1.0},{{39,14},1.0},{{24,24},4.0},{{24,34},1.0},{{24,37},1.0},{{24,7},1.0},{{34,24},1.0},{{34,34},1.0},{{34,37},1.0},{{34,7},1.0},
        {{37,24},1.0},{{37,34},1.0},{{37,7},1.0},{{7,24},1.0},{{7,34},1.0},{{7,37},1.0},{{16,9},1.0},{{16,29},1.0},{{16,14},1.0},{{9,16},1.0},
        {{29,16},1.0},{{14,16},1.0},{{17,17},3.0},{{17,18},1.0},{{17,11},1.0},{{17,4},1.0},{{18,17},1.0},{{18,11},1.0},{{18,4},1.0},{{11,17},1.0},
        {{11,18},1.0},{{11,4},1.0},{{4,17},1.0},{{4,18},1.0},{{4,11},1.0},{{16,28},1.0},{{16,37},1.0},{{33,28},1.0},{{33,37},1.0},{{28,16},1.0},
        {{28,33},1.0},{{28,28},5.0},{{28,37},1.0},{{37,16},1.0},{{37,33},1.0},{{37,28},1.0},{{37,26},1.0},{{37,11},1.0},{{37,5},1.0},{{26,37},1.0},
        {{26,11},1.0},{{11,37},1.0},{{11,26},1.0},{{11,5},1.0},{{5,37},1.0},{{5,11},1.0},{{8,35},2.0},{{8,14},1.0},{{26,35},1.0},{{26,14},1.0},
        {{35,8},2.0},{{35,26},1.0},{{35,14},1.0},{{14,8},1.0},{{14,26},1.0},{{14,35},1.0},{{24,4},1.0},{{24,30},1.0},{{24,15},1.0},{{4,24},1.0},
        {{4,30},1.0},{{4,15},1.0},{{30,24},1.0},{{30,4},1.0},{{30,30},2.0},{{30,15},1.0},{{15,24},1.0},{{15,4},1.0},{{15,30},1.0},{{16,17},1.0},
        {{16,12},1.0},{{16,6},1.0},{{17,16},1.0},{{17,12},1.0},{{17,6},1.0},{{12,16},1.0},{{12,17},1.0},{{12,6},1.0},{{6,16},1.0},{{6,17},1.0},
        {{6,12},1.0},{{32,32},1.0},{{32,25},1.0},{{32,35},1.0},{{32,28},1.0},{{25,32},1.0},{{25,35},1.0},{{25,28},1.0},{{35,32},1.0},{{35,25},1.0},
        {{35,28},1.0},{{28,32},1.0},{{28,25},1.0},{{28,35},1.0},{{24,25},1.0},{{24,14},1.0},{{24,1},1.0},{{25,24},1.0},{{25,14},2.0},{{25,1},1.0},
        {{14,24},1.0},{{14,25},2.0},{{14,1},1.0},{{1,24},1.0},{{1,25},1.0},{{1,14},1.0},{{24,35},1.0},{{24,5},1.0},{{24,6},1.0},{{35,24},1.0},
        {{35,5},1.0},{{35,6},2.0},{{5,24},1.0},{{5,35},1.0},{{5,6},1.0},{{6,24},1.0},{{6,35},2.0},{{6,5},1.0},{{28,12},1.0},{{28,38},1.0},
        {{28,15},1.0},{{12,28},1.0},{{12,38},1.0},{{12,15},1.0},{{38,28},1.0},{{38,12},1.0},{{38,38},1.0},{{38,15},1.0},{{15,28},1.0},{{15,12},1.0},
        {{15,38},1.0},{{8,6},1.0},{{18,6},1.0},{{6,8},1.0},{{6,18},1.0},{{28,31},1.0},{{28,22},1.0},{{28,39},1.0},{{31,28},1.0},{{31,22},1.0},
        {{31,39},1.0},{{22,28},1.0},{{22,31},1.0},{{22,39},1.0},{{39,28},1.0},{{39,31},1.0},{{39,22},1.0},{{1,28},1.0},{{1,13},1.0},{{1,7},1.0},
        {{28,1},1.0},{{28,13},1.0},{{28,7},1.0},{{13,1},1.0},{{13,28},1.0},{{13,13},1.0},{{13,7},1.0},{{7,1},1.0},{{7,28},1.0},{{7,13},1.0},
        {{17,10},1.0},{{17,36},1.0},{{17,7},1.0},{{10,17},1.0},{{10,36},1.0},{{10,7},1.0},{{36,17},1.0},{{36,10},1.0},{{7,17},1.0},{{7,10},1.0},
        {{25,30},1.0},{{25,9},1.0},{{14,30},1.0},{{30,25},1.0},{{30,14},1.0},{{30,9},1.0},{{9,25},1.0},{{9,30},1.0},};

    return AMap;
}

bool CheckGraph(
        const SparseContiguousRowGraph& rAgraph,
        const MatrixMapType& rReferenceGraph)
{
    //check that all entries in Agraph are also in reference_A_map
    const auto& rgraph = rAgraph.GetGraph();
    for(IndexType I = 0; I<rgraph.size();++I)
    {
        for(auto J : rgraph[I] )
        {
            if(rReferenceGraph.find({I,J}) == rReferenceGraph.end()) //implies it is not present
                KRATOS_ERROR << "Entry " << I << "," << J << "not present in A graph"  << std::endl;
        }
    }

    for(auto it=rAgraph.begin(); it!=rAgraph.end(); ++it)
    {
        const auto I = it.GetRowIndex();
        for(auto J : *it )
        {
            if(rReferenceGraph.find({I,J}) == rReferenceGraph.end()) //implies it is not present
                KRATOS_ERROR << "Entry " << I << "," << J << "not present in A graph"  << std::endl;
        }
    }

    //check that all the entries of reference_A_map are also in Agraph
    for(unsigned int I=0; I<rgraph.size(); ++I)
    {
        for(auto J : rgraph[I])
            if(rgraph[I].find(J) == rgraph[I].end())
                 KRATOS_ERROR << "Entry " << I << "," << J << " is in the reference graph but not in Agraph"  << std::endl;
    }

    return true;
}

bool CheckGraph(
        const SparseGraph& rAgraph,
        const MatrixMapType& rReferenceGraph)
{
    //check that all entries in Agraph are also in reference_A_map
    const auto& rgraph = rAgraph.GetGraph();
    for(const auto& item : rgraph)
    {
        const auto I = item.first;
        for(auto J : item.second )
        {
            if(rReferenceGraph.find({I,J}) == rReferenceGraph.end()) //implies it is not present
                KRATOS_ERROR << "Entry " << I << "," << J << "not present in A graph"  << std::endl;
        }
    }

    for(auto it=rAgraph.begin(); it!=rAgraph.end(); ++it)
    {
        const auto I = it.GetRowIndex();
        for(auto J : *it )
        {
            if(rReferenceGraph.find({I,J}) == rReferenceGraph.end()) //implies it is not present
                KRATOS_ERROR << "Entry " << I << "," << J << "not present in A graph"  << std::endl;
        }
    }


    //check that all the entries of reference_A_map are also in Agraph
    for(const auto& item : rReferenceGraph)
    {
        auto I = item.first.first;
        auto J = item.first.second;
        if(!rAgraph.Has(I,J))
            KRATOS_ERROR << "Entry " << I << "," << J << " is in the reference graph but not in Agraph"  << std::endl;
    }

    return true;
}

bool CheckCSRGraphArrays(
    const vector<SparseGraph::IndexType>& rRowIndices,
    const vector<SparseGraph::IndexType>& rColIndices,
    const MatrixMapType& rReferenceGraph)
{
    auto N = rRowIndices.size()-1;
    for (IndexType I = 0; I < N; ++I)
    {
        for (auto k = rRowIndices[I]; k < rRowIndices[I + 1]; ++k)
        {
            IndexType J = rColIndices[k];
            if (rReferenceGraph.find({I, J}) == rReferenceGraph.end()) //implies it is not present
                KRATOS_ERROR << "Entry " << I << "," << J << "not present in A graph" << std::endl;

            //check that that cols are ordered
            if(k-rRowIndices[I] > 0)
                if(rColIndices[k-1]>rColIndices[k])
                    KRATOS_ERROR << "columns are not ordered in csr" << std::endl;
        }
    }
    return true;
}


template<typename TGraphType>
std::unique_ptr<TGraphType> AssembleGraph(
    ElementConnectivityType& rConnectivities,
    IndexType GraphSize)
{
    std::unique_ptr<TGraphType> pAgraph(nullptr);

    #pragma omp parallel
    {
        std::unique_ptr<TGraphType>
            plocal_graph(new TGraphType(GraphSize));
        #pragma omp for
        for(int i=0; i<static_cast<int>(rConnectivities.size()); ++i){
            plocal_graph->AddEntries(rConnectivities[i]);
        }

        #pragma omp critical
        {
            if(pAgraph == nullptr )
                pAgraph.swap(plocal_graph);
            else
                pAgraph->AddEntries(*plocal_graph);
        }
    }

    pAgraph->Finalize();
    return pAgraph;
}

// Basic Type
KRATOS_TEST_CASE_IN_SUITE(GraphConstruction, KratosCoreFastSuite)
{
    const auto connectivities = ElementConnectivities();
    auto reference_A_map = GetReferenceMatrixAsMap();
KRATOS_WATCH(__LINE__)
    SparseGraph Agraph;
    for(const auto& c : connectivities)
        Agraph.AddEntries(c);
KRATOS_WATCH(__LINE__)
    Agraph.Finalize();
KRATOS_WATCH(__LINE__)
    CheckGraph(Agraph, reference_A_map);

    //check serialization
    StreamSerializer serializer;
    const std::string tag_string("TestString");
    serializer.save(tag_string, Agraph);
    Agraph.Clear();
    serializer.load(tag_string, Agraph);
    CheckGraph(Agraph, reference_A_map);

    //check exporting
    vector<IndexType> row_index, col_index;
    Agraph.ExportCSRArrays(row_index, col_index);

    CheckCSRGraphArrays(row_index, col_index, reference_A_map);

}

KRATOS_TEST_CASE_IN_SUITE(GraphContiguousRowConstruction, KratosCoreFastSuite)
{
    const auto connectivities = ElementConnectivities();
    auto reference_A_map = GetReferenceMatrixAsMap();

    SparseContiguousRowGraph Agraph(40);
    for(const auto& c : connectivities)
        Agraph.AddEntries(c);
    Agraph.Finalize();

    CheckGraph(Agraph, reference_A_map);

    //check serialization
    StreamSerializer serializer;
    const std::string tag_string("TestString");
    serializer.save(tag_string, Agraph);
    Agraph.Clear();
    serializer.load(tag_string, Agraph);
    CheckGraph(Agraph, reference_A_map);

    // //check exporting
    // vector<IndexType> row_index, col_index;
    // Agraph.ExportCSRArrays(row_index, col_index);

    // CheckCSRGraphArrays(row_index, col_index, reference_A_map);

}

// Basic Type
KRATOS_TEST_CASE_IN_SUITE(OpenMPGraphContiguousRowConstruction, KratosCoreFastSuite)
{
    const auto connectivities = ElementConnectivities();
    auto reference_A_map = GetReferenceMatrixAsMap();

    std::unique_ptr<SparseContiguousRowGraph> pAgraph(nullptr);

    #pragma omp parallel
    {
        std::unique_ptr<SparseContiguousRowGraph> plocal_graph(new SparseContiguousRowGraph(40));
        #pragma omp for
        for(int i=0; i<static_cast<int>(connectivities.size()); ++i){
            plocal_graph->AddEntries(connectivities[i]);
        }

        #pragma omp critical
        {
            if(pAgraph == nullptr )
                pAgraph.swap(plocal_graph);
            else
                pAgraph->AddEntries(*plocal_graph);
        }
    }

    pAgraph->Finalize();

    CheckGraph(*pAgraph, reference_A_map);
}

KRATOS_TEST_CASE_IN_SUITE(PerformanceBenchmarkSparseGraph, KratosCoreFastSuite)
{
    const IndexType block_size = 4;
    const IndexType nodes_in_elem = 4;
    const IndexType nel = 1e6;
    const IndexType ndof = nel/6;
    const IndexType standard_dev = 100;
    auto connectivities = RandomElementConnectivities(block_size,nodes_in_elem, nel,ndof,standard_dev);

    double start_graph = OpenMPUtils::GetCurrentTime();
    auto pAgraph = AssembleGraph<SparseGraph>(connectivities, ndof*block_size);
    double end_graph = OpenMPUtils::GetCurrentTime();

    pAgraph->Finalize();
    std::cout << "SparseGraph time = " << end_graph-start_graph << std::endl;
}

KRATOS_TEST_CASE_IN_SUITE(PerformanceBenchmarkSparseContiguousRowGraph, KratosCoreFastSuite)
{
    const IndexType block_size = 4;
    const IndexType nodes_in_elem = 4;
    const IndexType nel = 1e6;
    const IndexType ndof = nel/6;
    const IndexType standard_dev = 100;
    auto connectivities = RandomElementConnectivities(block_size,nodes_in_elem, nel,ndof,standard_dev);

    double start_graph = OpenMPUtils::GetCurrentTime();
    auto pAgraph = AssembleGraph<SparseContiguousRowGraph>(connectivities, ndof*block_size);
    double end_graph = OpenMPUtils::GetCurrentTime();

    std::cout << "SparseGraphDiscontinuousRow generation time = " << end_graph-start_graph << std::endl;
}
} // namespace Testing
} // namespace Kratos