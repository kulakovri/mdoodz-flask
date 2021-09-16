from flask import Blueprint, request
from .compilation_dto import CompilingDto
from .compilation_service import compile_mdoodz, clean_compiled, get_cache

blueprint = Blueprint('compiler', __name__)
blueprint_cleaner = Blueprint('cleaner', __name__)
blueprint_cache = Blueprint('compiling_cache', __name__)


@blueprint.post('/api/compile')
def compile():
  compiling_dto = CompilingDto.Schema().load(request.json)
  compile_mdoodz(compiling_dto)
  return ''


@blueprint_cleaner.post('/api/clean')
def clean():
  clean_compiled()
  return ''


@blueprint_cache.get('/api/compiling-cache')
def get_compiling_cached():
  return get_cache()
