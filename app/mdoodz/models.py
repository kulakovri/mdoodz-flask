import subprocess


def compile_mdoodz():
  stream = subprocess.Popen(['make', 'all', 'MODEL=RiftingPaulineMD6', 'OPT=yes', 'MKL=yes'], cwd='mdoodz_source')
  print(stream.stdout)


def clean():
  stream = subprocess.Popen(['make', 'clean'], cwd='mdoodz_source')
  print(stream.stdout)


def run_sumilation():
  stream = subprocess.Popen(['./Doodzi_RiftingPaulineMD6', 'RiftingPaulineMD6.txt'], cwd='mdoodz_source')
  print(stream.stdout)
