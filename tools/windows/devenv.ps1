param(
  [string]$Arch = "x64"
)

$VcVars = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"

cmd /c "`"$VcVars`" $Arch && set" |
  ForEach-Object {
    if ($_ -match '^(.*?)=(.*)$') {
      [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
    }
  }
