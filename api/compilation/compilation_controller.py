from flask import Blueprint, request
from .compilation_dto import CompilingDto
from .compilation_service import compile_mdoodz, clean_compiled

blueprint = Blueprint('compiler', __name__)
cleaner_blueprint = Blueprint('cleaner', __name__)


@blueprint.post('/api/compile')
def compile():
  compiling_dto = CompilingDto.Schema().load(request.json)
  compile_mdoodz(compiling_dto)
  return ''


@cleaner_blueprint.post('/api/clean')
def clean():
  clean_compiled()
  return ''
