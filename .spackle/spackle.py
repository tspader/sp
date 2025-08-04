import subprocess
import os

import spackle

class Paths:
  def __init__(self):
    self.spackle = os.path.realpath(os.path.dirname(__file__))
    self.project = os.path.realpath(os.path.join(self.spackle, '..'))
    self.build = os.path.join(self.project, 'build')
    self.build_ps1 = os.path.join(self.build, 'build.ps1')
    self.binary = os.path.join(self.build, 'sp.exe')

@spackle.tool
def build() -> spackle.McpResult:
  paths = Paths()
  result = spackle.wrap_subprocess(
    ['powershell.exe', '-File', paths.build_ps1],
    stdout=subprocess.PIPE, 
    stderr=subprocess.PIPE,
    text=True
  )

  return spackle.McpResult(
    return_code = result.returncode,
    stderr = result.stderr,
    stdout = result.stdout,
    response = ''
  )

@spackle.tool
def run() -> spackle.McpResult:
  return spackle.McpResult(
    return_code = 0,
    stderr = '',
    stdout = '',
    response = 'There is nothing to run in this project'
  )

@spackle.tool
def test() -> spackle.McpResult:
  paths = Paths()
  
  result = build()
  if result.return_code != 0:
    return result

  result = spackle.wrap_subprocess(
    [paths.binary],
    stdout=subprocess.PIPE, 
    stderr=subprocess.PIPE,
    text=True
  )

  return spackle.McpResult(
    return_code = result.returncode,
    stderr = result.stderr,
    stdout = result.stdout,
    response = ''
  )
