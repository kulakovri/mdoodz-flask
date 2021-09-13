import subprocess
from .simulation_dto import SimulationDto


def run_sumilation(simulation_dto: SimulationDto):
  stream = subprocess.Popen([f'./Doodzi_{simulation_dto.simulation_name}', f'{simulation_dto.simulation_name}.txt'],
                            cwd='mdoodz_source')
  print(stream.stdout)
