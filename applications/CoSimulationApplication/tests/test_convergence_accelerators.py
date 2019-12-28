from __future__ import print_function, absolute_import, division  # makes these scripts backward compatible with python 2.6 and 2.7

import KratosMultiphysics as KM
import KratosMultiphysics.KratosUnittest as KratosUnittest
import math

from KratosMultiphysics.CoSimulationApplication.coupling_interface_data import CouplingInterfaceData
# from KratosMultiphysics.CoSimulationApplication.base_classes.co_simulation_convergence_accelerator import CoSimulationConvergenceAccelerator
from KratosMultiphysics.CoSimulationApplication.convergence_accelerators.convergence_accelerator_wrapper import ConvergenceAcceleratorWrapper
from testing_utilities import DummySolverWrapper

from unittest.mock import Mock
import numpy as np
from random import uniform

if KM.IsDistributedRun():
    import KratosMultiphysics.mpi as KratosMPI

class TestConvergenceAcceleratorWrapper(KratosUnittest.TestCase):

    def setUp(self):
        self.model = KM.Model()
        self.model_part = self.model.CreateModelPart("default")
        self.model_part.AddNodalSolutionStepVariable(KM.PRESSURE)
        self.model_part.AddNodalSolutionStepVariable(KM.PARTITION_INDEX)
        self.dimension = 3
        self.model_part.ProcessInfo[KM.DOMAIN_SIZE] = self.dimension

        num_proc = KM.DataCommunicator.GetDefault().Size()
        my_pid = KM.DataCommunicator.GetDefault().Rank()
        self.num_nodes = my_pid % 5 + 3 # num_nodes in range (3 ... 7)
        if my_pid == 4:
            self.num_nodes = 0 # in order to emulate one partition not having local nodes

        for i in range(self.num_nodes):
            node = self.model_part.CreateNewNode(i, 0.1*i, 0.0, 0.0) # this creates the same coords in different ranks, which does not matter for this test

            node.SetSolutionStepValue(KM.PARTITION_INDEX, my_pid)
            node.SetSolutionStepValue(KM.PRESSURE, uniform(-10, 50))

        if KM.IsDistributedRun():
            KratosMPI.ParallelFillCommunicator(self.model_part).Execute()

        data_settings = KM.Parameters("""{
            "model_part_name" : "default",
            "variable_name"   : "PRESSURE"
        }""")
        self.interface_data = CouplingInterfaceData(data_settings, self.model)
        self.interface_data.Initialize()

        self.dummy_solver_wrapper = DummySolverWrapper({"data_4_testing" : self.interface_data})

    # def test_accelerator_without_support_for_distributed_data(self):
    #     pass


    def test_accelerator_with_support_for_distributed_data(self):
        conv_acc_settings = KM.Parameters("""{
            "type"      : "constant_relaxation",
            "data_name" : "data_4_testing"
        }""")
        conv_acc_wrapper = ConvergenceAcceleratorWrapper(conv_acc_settings, self.dummy_solver_wrapper)

        conv_acc_mock = Mock()
        attrs = {
            'SupportsDistributedData.return_value': True,
            'UpdateSolution.return_value' : [uniform(-10, 50) for _ in range(self.num_nodes)]
        }
        conv_acc_mock.configure_mock(**attrs)

        conv_acc_wrapper.conv_acc = conv_acc_mock

        conv_acc_wrapper.InitializeSolutionStep()

        self.assertEqual(conv_acc_mock.SupportsDistributedData.call_count, 1)
        self.assertFalse(conv_acc_wrapper.gather_scatter_required)

        conv_acc_wrapper.InitializeNonLinearIteration()

        exp_inp = self.interface_data.GetData()

        # setting new solution for computing the residual
        rand_data = [uniform(-10, 50) for _ in range(self.num_nodes)]
        self.interface_data.SetData(rand_data)
        exp_res = rand_data - exp_inp

        conv_acc_wrapper.ComputeAndApplyUpdate()

        # numpy arrays cannot be compared using the mack-functions, hence using the numpy functions
        np.testing.assert_array_equal(exp_res, conv_acc_mock.UpdateSolution.call_args[0][0])
        np.testing.assert_array_equal(exp_inp, conv_acc_mock.UpdateSolution.call_args[0][1])


if __name__ == '__main__':
    KratosUnittest.main()
