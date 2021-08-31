from marshmallow_dataclass import dataclass
from typing import Optional


@dataclass
class CompilingDto:
  simulation_name: str
  mkl: Optional[str]
  opt: Optional[str]
