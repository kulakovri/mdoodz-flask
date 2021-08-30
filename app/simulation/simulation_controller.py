from flask import Blueprint, request
from .simulation_service import run_sumilation

blueprint = Blueprint('simulation', __name__)

@blueprint.post('/api/run-simulation')
def compile():
  run_sumilation()
  return ''
