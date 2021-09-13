from flask import Blueprint, request
from .visualisation_service import run_visualisation

blueprint = Blueprint('visualisation', __name__)


@blueprint.get('/api/visualise')
def visualise():
  print('visualising')
  run_visualisation()
  return ''
