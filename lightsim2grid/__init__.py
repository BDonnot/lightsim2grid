__version__ = "0.5.5"

__all__ = ["newtonpf", "SolverType", "solver"]

# import directly from c++ module
from lightsim2grid.solver import SolverType

try:
    from lightsim2grid.lightSimBackend import LightSimBackend
    __all__.append("LightSimBackend")
except ImportError:
    # grid2op is not installed, the Backend will not be available
    pass

try:
    from lightsim2grid.physical_law_checker import PhysicalLawChecker
    __all__.append("PhysicalLawChecker")
except ImportError as exc_:
    # grid2op is not installed, the PhysicalLawChecker will not be available
    pass

try:
    from lightsim2grid.timeSerie import TimeSerie
    __all__.append("TimeSerie")
    __all__.append("timeSerie")
except ImportError as exc_:
    # grid2op is not installed, the TimeSeries module will not be available
    pass

try:
    from lightsim2grid.securityAnalysis import SecurityAnalysis
    __all__.append("SecurityAnalysis")
    __all__.append("securityAnalysis")
except ImportError as exc_:
    # grid2op is not installed, the SecurtiyAnalysis module will not be available
    pass
