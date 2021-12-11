Push-Location
if (-Not (Test-Path functional_tests)) {
    mkdir functional_tests
}
Set-Location functional_tests

if (-Not (Test-Path 6502_65C02_functional_tests)) {
    Write-Output "clone github.com/Klaus2m5/6502_65C02_functional_tests"
    git clone https://github.com/Klaus2m5/6502_65C02_functional_tests.git
}

if (-Not (Test-Path as65_142)) {
    Write-Output "unzip assembler"
    Expand-Archive -Path 6502_65C02_functional_tests\as65_142.zip -DestinationPath as65_142
}

Push-Location
Set-Location as65_142
if (-Not (Test-Path ../functional_test.bin) -or -Not (Test-Path ../interrupt_test.bin)) {
    Write-Output "assemble tests"
    # & '.\as65.exe' '--help'
    & '.\as65.exe' '-l../functional_test.lst' '-h0' '-w140' '-m' '-o../functional_test.bin' '../6502_65C02_functional_tests/6502_functional_test.a65'
    & '.\as65.exe' '-l../interrupt_test.lst' '-h0' '-w140' '-m' '-o../interrupt_test.bin' '../6502_65C02_functional_tests/6502_interrupt_test.a65'
    & '.\as65.exe' '-l../decimal_test.lst' '-h0' '-w140' '-m' '-o../decimal_test.bin' '../6502_65C02_functional_tests/6502_decimal_test.a65'
}
Pop-Location

Write-Output ""
Write-Output "!! make sure that the qtCreator Debug build is current !!"
Write-Output ""
Push-Location
Set-Location ..\..\..\build-c64pp-*-Debug

$env:Path += ";C:\Qt\6.2.1\mingw81_64\bin"

Write-Output "===== run test: FUNCTIONAL_TEST (HEAD) ====="
& '.\c64pp.exe' 'functional_test' '..\c64pp\scripts\functional_tests\functional_test.bin' '3469'
if ($LASTEXITCODE -eq 0) {
    Write-Output ""
    Write-Output "======= TEST SUCCESSFUL ========"
} else {
    Write-Output "======= TEST FAILED ========"
    Write-Output "c64pp returned $LASTEXITCODE"
}
if ($LASTEXITCODE -eq 0) {
    Write-Output ""
    Write-Output "===== run test: INTERRUPT_TEST (HEAD) ====="
    & '.\c64pp.exe' 'functional_test' '..\c64pp\scripts\functional_tests\interrupt_test.bin' '06f5'
    if ($LASTEXITCODE -eq 0) {
        Write-Output ""
        Write-Output "======= TEST SUCCESSFUL ========"
    } else {
        Write-Output "======= TEST FAILED ========"
        Write-Output "c64pp returned $LASTEXITCODE"
    }
}
if ($LASTEXITCODE -eq 0) {
    Write-Output ""
    Write-Output "===== run test: DECIMAL_TEST (HEAD) ====="
    & '.\c64pp.exe' 'functional_test' '..\c64pp\scripts\functional_tests\decimal_test.bin' '024b'
    if ($LASTEXITCODE -eq 0) {
        Write-Output ""
        Write-Output "======= TEST SUCCESSFUL ========"
    } else {
        Write-Output "======= TEST FAILED ========"
        Write-Output "c64pp returned $LASTEXITCODE"
    }
}
if ($LASTEXITCODE -eq 0) {
    Write-Output ""
    Write-Output "===== run test: FUNCTIONAL_TEST (BIN) ====="
    & '.\c64pp.exe' 'functional_test' '..\c64pp\scripts\functional_tests\6502_65C02_functional_tests\bin_files\6502_functional_test.bin' '3469'
    if ($LASTEXITCODE -eq 0) {
        Write-Output "======= TEST SUCCESSFUL ========"
    } else {
        Write-Output "======= TEST FAILED ========"
        Write-Output "c64pp returned $LASTEXITCODE"
    }
}

Pop-Location
Pop-Location

if ($myInvocation.line -match "ExecutionPolicy") {
    Read-Host -Prompt "Press any key to continue"
}