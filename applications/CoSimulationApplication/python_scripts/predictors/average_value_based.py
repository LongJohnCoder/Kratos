from __future__ import print_function, absolute_import, division  # makes these scripts backward compatible with python 2.6 and 2.7

# Importing the base class
from KratosMultiphysics.CoSimulationApplication.base_classes.co_simulation_predictor import CoSimulationPredictor

# Other imports
import KratosMultiphysics.CoSimulationApplication.co_simulation_tools as cs_tools

# Predictor implemented according to:
# "A new staggered scheme for fluid-structure interaction"; W.G. Dettmer and D. Peric
# Numerical Methods in Engineering 2013; 93; 1-22

def Create(settings, solver_wrapper):
    cs_tools.SettingsTypeCheck(settings)
    return AverageValuePredictor(settings, solver_wrapper)

class AverageValuePredictor(CoSimulationPredictor):
    # @param beta factor for weighting last and current value of the predicted values. Can be set in interval: [0, 1.0]
    def __init__(self, settings, solver_wrapper):
        super(AverageValuePredictor, self).__init__(settings, solver_wrapper)
        self.beta = self.settings["beta"].GetDouble()
        if self.beta > 1.0 or self.beta < 0:
            raise Exception("Wrong value for beta. Admissible interval [0.0, 1.0]")

    def Predict(self):
        current_data  = self.interface_data.GetNumpyArray(0)
        previous_data = self.interface_data.GetNumpyArray(1)

        self.predicted_data = 2*current_data - previous_data

        self._UpdateData(self.predicted_data)

    def FinalizeSolutionStep(self):
        current_data  = self.interface_data.GetNumpyArray(0)

        self.predicted_data = self.beta * current_data + (1-self.beta) * self.predicted_data

        self._UpdateData(self.predicted_data)

    @classmethod
    def _GetDefaultSettings(cls):
        this_defaults = cs_tools.cs_data_structure.Parameters("""{
            "beta"     : 0.5
        }""")
        this_defaults.AddMissingParameters(super(AverageValuePredictor, cls)._GetDefaultSettings())
        return this_defaults
