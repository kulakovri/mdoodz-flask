from flask import Blueprint, send_file, request
from . import visualisation_service

blueprint = Blueprint('visualisation', __name__)
blueprint_images = Blueprint('get-image', __name__)


@blueprint.post('/api/visualise')
def visualise():
  visualisation_service.run_visualisation()
  return ''


@blueprint_images.get('/api/images')
def get_image():
  number = request.args.get('number')
  if number is None:
    return visualisation_service.get_image_count()
  else:
    endcoded_file = visualisation_service.get_image(number)
    return send_file(endcoded_file, mimetype='image/png')
