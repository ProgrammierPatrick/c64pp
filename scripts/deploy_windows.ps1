$env:Path += ";C:\Qt\6.2.1\mingw81_64\bin"  # qt directory
$env:Path += ";C:\Qt\Tools\mingw810_64\bin" # mingw directory

if (Test-Path release_windows) {
    Remove-Item release_windows -r
}

mkdir release_windows
Copy-Item ..\..\build-c64pp-*-Release\c64pp.exe release_windows

& 'windeployqt' 'release_windows\c64pp.exe' '--no-translations'