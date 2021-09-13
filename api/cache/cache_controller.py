from flask import Blueprint
from . import cache_service

blueprint = Blueprint('cache', __name__)


@blueprint.get('/api/cache')
def get_cache():
  cache = cache_service.get_cache()
  return cache
