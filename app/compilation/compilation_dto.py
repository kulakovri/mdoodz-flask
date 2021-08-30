from marshmallow_dataclass import dataclass
from typing import Optional


@dataclass
class CompilingDto:
  modelName: str
  mkl: Optional[str]
  opt: Optional[str]
