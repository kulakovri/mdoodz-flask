from flask import Blueprint, request
from .models import compile_mdoodz, clean, run_sumilation

compiler = Blueprint('compiler', __name__)
cleaner = Blueprint('cleaner', __name__)
simulation_runner = Blueprint('simulation_runner', __name__)


@compiler.get('/mdoodz/compile')
def compile():
  print(request.args.get('pickedSimulationName'))
  #compile_mdoodz()
  return ''


@cleaner.post('/mdoodz/clean')
def run_simulation():
  clean()
  return ''


@simulation_runner.post('/mdoodz/run-simulation')
def run_simulation():
  run_sumilation()
  return ''
