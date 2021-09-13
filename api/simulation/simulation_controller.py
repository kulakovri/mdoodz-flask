from flask import Blueprint, request
from .simulation_dto import SimulationDto
from .simulation_service import run_sumilation

blueprint = Blueprint('simulation', __name__)


@blueprint.post('/api/run-simulation')
def compile():
  simulation_dto = SimulationDto.Schema().load(request.json)
  run_sumilation(simulation_dto)
