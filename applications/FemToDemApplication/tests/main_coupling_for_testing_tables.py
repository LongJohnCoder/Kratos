from __future__ import print_function, absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7

import KratosMultiphysics
from KratosMultiphysics import Logger
import KratosMultiphysics.FemToDemApplication as KratosFemDem
# import KratosMultiphysics.FemToDemApplication.MainCouplingFemDem as MainCouplingFemDem
import main_coupling_for_testing
import KratosMultiphysics.KratosUnittest as KratosUnittest

def Wait():
    input("Press Something")

def KratosPrintInfo(message):
    KratosMultiphysics.Logger.Print(message, label="")
    KratosMultiphysics.Logger.Flush()

#============================================================================================================================
class MainCouplingFemDemForTestingSolution(main_coupling_for_testing.MainCouplingFemDemForTestingSolution):
#============================================================================================================================

#============================================================================================================================
    def CheckControlValuesForTesting(self):  # KratosPrintInfo(str(dy))

        # Here we check the damage obtained at each FE
        element = self.FEM_Solution.main_model_part.GetElement(1)
        damage = element.CalculateOnIntegrationPoints(KratosFemDem.DAMAGE_ELEMENT, self.FEM_Solution.main_model_part.ProcessInfo)[0]

        if self.FEM_Solution.step == 26:
            if damage != 0.11526580049026725:
                raise ValueError('The computed damage at step = 26 is not correct')
        elif self.FEM_Solution.step == 36:
            if damage != 0.4722648310044538:
                raise ValueError('The computed damage at step = 36 is not correct')
        elif self.FEM_Solution.step == 46:
            if damage != 0.5600214207342531:
                raise ValueError('The computed damage at step = 46 is not correct')
        elif self.FEM_Solution.step == 61:
            if damage != 0.5600214207342531:
                raise ValueError('The computed damage at step = 61 is not correct')

        # Here we check the vertical displacement of a node
        node = self.FEM_Solution.main_model_part.GetNode(1)
        dy = node.GetSolutionStepValue(KratosMultiphysics.DISPLACEMENT_Y)
        if self.FEM_Solution.step == 26:
            if dy != 1.971665114439254e-05:
                raise ValueError('The computed displacement at step = 26 is not correct')
        elif self.FEM_Solution.step == 36:
            if dy != 2.7299978508712653e-05:
                raise ValueError('The computed displacement at step = 36 is not correct')
        elif self.FEM_Solution.step == 46:
            if dy != 2.578331303724928e-05:
                raise ValueError('The computed displacement at step = 46 is not correct')
        elif self.FEM_Solution.step == 61:
            if dy != 1.4408321991404051e-05:
                raise ValueError('The computed displacement at step = 61 is not correct')

        # Here we check the stresses and strains at one FE
        element = self.FEM_Solution.main_model_part.GetElement(1)
        Sx = element.CalculateOnIntegrationPoints(KratosFemDem.STRESS_VECTOR_INTEGRATED,           self.FEM_Solution.main_model_part.ProcessInfo)[0][0]
        Ex = element.CalculateOnIntegrationPoints(KratosMultiphysics.GREEN_LAGRANGE_STRAIN_VECTOR, self.FEM_Solution.main_model_part.ProcessInfo)[0][0]

        if self.FEM_Solution.step == 26:
            if Sx != 1441812.5386046136:
                raise ValueError('The computed stress at step = 26 is not correct')
            if Ex != 4.6561604584527234e-05:
                raise ValueError('The computed strain at step = 26 is not correct')
        elif self.FEM_Solution.step == 36:
            if Sx != 1190806.4343404802:
                raise ValueError('The computed stress at step = 36 is not correct')
            if Ex != 6.446991404011464e-05:
                raise ValueError('The computed strain at step = 36 is not correct')
        elif self.FEM_Solution.step == 46:
            if Sx != 937633.4336071612:
                raise ValueError('The computed stress at step = 46 is not correct')
            if Ex != 6.0888252148997134e-05:
                raise ValueError('The computed strain at step = 46 is not correct')
        elif self.FEM_Solution.step == 61:
            if Sx != 523971.6246628269:
                raise ValueError('The computed stress at step = 61 is not correct')
            if Ex != 3.4025787965616143e-05:
                raise ValueError('The computed strain at step = 61 is not correct')
        


class TestAnalytics(KratosUnittest.TestCase):
    
    def setUp(self):
        pass

    @classmethod
    def tables(self):
        model = KratosMultiphysics.Model()
        MainCouplingFemDemForTestingSolution(model, "small_tests/table_bc_imposition/").Run()



if __name__ == "__main__":
    KratosMultiphysics.Logger.GetDefaultOutput().SetSeverity(Logger.Severity.WARNING)
    KratosUnittest.main()