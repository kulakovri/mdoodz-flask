import h5py as h5py
import numpy as np
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure


def run_visualisation():
  file = h5py.File('/Users/romankulakov/PycharmProjects/mdoodz-web/mdoodz_source/Output00010.gzip.h5', 'r')

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
  ax = fig.add_subplot(111)
  pcm = ax.pcolormesh(P, cmap='gist_heat')
  fig.colorbar(pcm)

  canvas.print_figure('example.png')


run_visualisation()
