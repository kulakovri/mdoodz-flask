from flask import Blueprint, request
from .visualisation_service import run_visualisation

blueprint = Blueprint('visualisation', __name__)

@blueprint.post('/api/visualise')
def visualise():
  run_visualisation()
  return ''
