import h5py as h5py
import numpy as np
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure
import os


def run_visualisation():
  output_files = _get_list_of_output_files()

  for file_name in output_files:
    file = h5py.File(f'mdoodz_source/{file_name}', 'r')

    P = file['Centers/P']
    T = file['Centers/T']
    xc = file['/Model/xc_coord']
    zc = file['/Model/zc_coord']
    data = file['/Model/Params']
    data[0]
    data[1]
    data[2]
    Nx = data[3].astype(int)
    Nz = data[4].astype(int)
    Ncx = Nx - 1
    Ncz = Nz - 1

    P = np.reshape(P, (Ncz, Ncx))
    T = np.reshape(T, (Ncz, Ncx))

    fig = Figure()
    canvas = FigureCanvas(fig)
    pressure_ax = fig.add_subplot(211)
    pcm = pressure_ax.pcolormesh(P, cmap='gist_heat')
    fig.colorbar(pcm)

    temperature_ax = fig.add_subplot(212)
    tcm = temperature_ax.pcolormesh(T, cmap='gist_heat')
    fig.colorbar(tcm)

    canvas.print_figure(f'{file_name}.png')


def _get_list_of_output_files():
  files = os.listdir('mdoodz_source')
  output_files = list(filter(lambda file: _filter_output_files(file), files))
  return sorted(output_files)


def _filter_output_files(file):
  if '.gzip.h5' in file:
    return file
