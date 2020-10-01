# import Kratos
import KratosMultiphysics
import KratosMultiphysics.FemToDemApplication
import KratosMultiphysics.KratosUnittest as KratosUnittest

# Small femdem cases
import main_coupling_for_testing
import main_coupling_total_lagrangian_for_testing
import main_coupling_for_testing_tables


def AssembleTestSuites():


    suites = KratosUnittest.KratosSuites

    smallSuite = suites['small']

    smallSuite.addTest(main_coupling_for_testing.TestAnalytics("small_strain")) #defined inside main_coupling_for_testing
    smallSuite.addTest(main_coupling_total_lagrangian_for_testing.TestAnalytics("total_lagrangian")) #defined inside main_coupling_total_lagrangian_for_testing
    smallSuite.addTest(main_coupling_for_testing_tables.TestAnalytics("tables")) #defined inside main_coupling_for_testing_tables

    # Create a test suit with the selected tests plus all small tests
    nightSuite = suites['nightly']

    # nightSuite.addTests(smallSuite)

    validationSuite = suites['validation']

    # Create a test suit that contains all the tests:
    allSuite = suites['all']
    allSuite.addTests(
        smallSuite
        #KratosUnittest.TestLoader().loadTestsFromTestCases([])
    )

    return suites


if __name__ == '__main__':
    KratosUnittest.runTests(AssembleTestSuites())
