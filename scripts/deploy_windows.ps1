# qt directory
$env:Path += ";C:\Qt\6.3.0\mingw_64\bin"

# mingw directory
$env:Path += ";C:\Qt\Tools\mingw1120_64\bin"

if ($args.Length -ge 1) {
    $build_type = $args[0]
}
else {
    $build_type = 'Release'
}

if (Test-Path release_win_$build_type) {
    Remove-Item release_win_$build_type -r
}

mkdir release_win_$build_type
Copy-Item ..\..\build-c64pp-*-$build_type\c64pp.exe release_win_$build_type

& 'windeployqt' "release_win_$build_type\c64pp.exe" '--no-translations'