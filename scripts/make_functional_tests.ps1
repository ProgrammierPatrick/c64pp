if (-Not (Test-Path 6502_65C02_functional_tests)) {
    Write-Output "clone github.com/Klaus2m5/6502_65C02_functional_tests"
    git clone https://github.com/Klaus2m5/6502_65C02_functional_tests.git
}

Write-Output "make sure that qtCreator Debug build is current!"
Push-Location
Set-Location ..\..\build-c64pp-*-Debug

$env:Path += ";C:\Qt\6.2.1\mingw81_64\bin"
& '.\c64pp.exe' 'functional_test' '..\c64pp\6502_65C02_functional_tests/65C02_extended_opcodes_test.bin'

Pop-Location

Read-Host -Prompt "Press any key to continue"