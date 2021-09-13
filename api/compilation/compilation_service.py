import subprocess
from .compilation_dto import CompilingDto
from api.utils.cache.cache_service import save_compilation


def compile_mdoodz(compiling_dto: CompilingDto):
  save_compilation(compiling_dto)
  stream = subprocess.Popen(
    ['make', 'all', f'MODEL={compiling_dto.simulation_name}', f'OPT={compiling_dto.opt}]',
     f'MKL={compiling_dto.mkl}'],
    cwd='mdoodz_source')
  print(stream.stdout)


def clean_compiled():
  stream = subprocess.Popen(['make', 'clean'], cwd='mdoodz_source')
  print(stream.stdout)
