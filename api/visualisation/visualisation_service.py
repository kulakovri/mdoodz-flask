import h5py as h5py
import numpy as np
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure
import os

images_dir = 'images'
mdoodz_source_dir = 'mdoodz_source'


def get_image(number: str):
  file_path = os.path.join(images_dir, f'{number}.png')
  file = open(file_path, "rb")
  return file


def get_image_count():
  files = os.listdir(images_dir)
  return str(len(files))


def run_visualisation():
  output_files = _get_list_of_output_files()
  count = 0
  for file_name in output_files:
    file = h5py.File(f'{mdoodz_source_dir}/{file_name}', 'r')

    data = file['/Model/Params']
    Nx = data[3].astype(int)
    Nz = data[4].astype(int)
    Ncx = Nx - 1
    Ncz = Nz - 1

    fig = Figure()
    canvas = FigureCanvas(fig)

    pressure_ax = fig.add_subplot(211)
    P = file['Centers/P']
    P = np.reshape(P, (Ncz, Ncx))
    pcm = pressure_ax.pcolormesh(P, cmap='gist_heat')
    fig.colorbar(pcm)

    temperature_ax = fig.add_subplot(212)
    T = file['Centers/T']
    T = np.reshape(T, (Ncz, Ncx))
    tcm = temperature_ax.pcolormesh(T, cmap='gist_heat')
    fig.colorbar(tcm)

    canvas.print_figure(f'{images_dir}/{count}.png')
    count += 1


def _get_list_of_output_files():
  files = os.listdir(f'{mdoodz_source_dir}')
  output_files = list(filter(lambda file: _filter_output_files(file), files))
  return sorted(output_files)


def _filter_output_files(file):
  if '.gzip.h5' in file:
    return file
