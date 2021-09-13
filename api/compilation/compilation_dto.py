from marshmallow_dataclass import dataclass


@dataclass
class CompilingDto:
  simulation_name: str
  mkl: str
  opt: str
