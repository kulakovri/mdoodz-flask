from flask import Blueprint
from .models import run_visualisation

visualizer = Blueprint('visualizer', __name__)

@visualizer.get('/visualise')
def visualise():
  run_visualisation()
  return ''
