import subprocess


def run_sumilation():
  stream = subprocess.Popen(['./Doodzi_RiftingPaulineMD6', 'RiftingPaulineMD6.txt'], cwd='mdoodz_source')
  print(stream.stdout)
