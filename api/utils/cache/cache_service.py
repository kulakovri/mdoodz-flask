from api.compilation.compilation_dto import CompilingDto
from configparser import ConfigParser

cache_file_name = 'cache.ini'
config = ConfigParser()


def save_compilation(compiling_dto: CompilingDto):
  config.read(cache_file_name)
  section_name = 'compiling'
  create_section(section_name)
  config.set(section_name, 'simulation_name', compiling_dto.simulation_name)
  config.set(section_name, 'mkl', compiling_dto.mkl)
  config.set(section_name, 'opt', compiling_dto.opt)
  _update(config)


def save_compilation_status(status: str):
  config.read(cache_file_name)
  section_name = 'compiling'
  create_section(section_name)
  config.set(section_name, 'status', status)
  _update(config)


def create_section(section_name: str):
  if not config.has_section(section_name):
    config.add_section(section_name)


def _update(config):
  with open(cache_file_name, 'w') as f:
    config.write(f)
