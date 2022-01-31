# run this in WSL from this directory

# VICE path
PATH=$PATH:/mnt/c/portable/GTK3VICE-3.5-win64/bin

PRG="C:\share\roms\C64\demo\are we there yet.prg"
ADDR="080D"

rm -f monitor.log
echo "trace exec 0000 ffff" > trace_cmd.txt

echo "run VICE"
x64sc.exe -monlog -moncommands "trace_cmd.txt" -autostartprgmode 1 -autostart "$PRG"

echo "filter VICE logfile"
cat monitor.log | grep "\.C:" | tr '[:lower:]' '[:upper:]' | sed "0, /$ADDR/d" > monitor_vice.log
rm monitor.log

echo "run c64++"
release_win_Release/c64pp.exe trace "$PRG" "$ADDR" | grep "\.C:" | tr -d '\r' > monitor_c64pp.log

echo "compare results"
paste monitor_c64pp.log monitor_vice.log | while IFS=$'\t' read c v; do
    cc="${c:3:4}"
    vv="${v:3:4}"
    if [ "$cc" != "$vv" ]; then
        echo "! c64++ $c"
        echo "! vice  $v"

        if [ -z "$c" ]; then
            exit
        fi
        if [ -z "$v" ]; then
            exit
        fi
    else
        echo "  c64++ $c | vice $v"
    fi
# done | grep -C 30 '!' | tee monitor_diff.log
done | tee monitor_diff.log

