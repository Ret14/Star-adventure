REM C:\gbdk\bin\lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -c -o main.o main.c
REM C:\gbdk\bin\lcc -Wa-l -Wl-m -Wl-j -DUSE_SFR_FOR_REG -o main.gb main.o
C:\gbdk\bin\lcc -Wa-l -Wf-ba0 -c -o savestate.o savestate.c
C:\gbdk\bin\lcc -Wa-l -Wf-ba0 -c -o short.o short.c
C:\gbdk\bin\lcc -Wa-l -Wf-ba0 -c -o FinalTiles.o FinalTiles.c
C:\gbdk\bin\lcc -Wa-l -Wf-ba0 -c -o level800.o level800.c
C:\gbdk\bin\lcc -Wa-l -Wf-ba0 -c -o newKey.o newKey.c
C:\gbdk\bin\lcc -Wa-l -Wf-ba0 -c -o records.o records.c
C:\gbdk\bin\lcc -Wa-l -Wf-ba0 -c -o starstar_data.o starstar_data.c
C:\gbdk\bin\lcc -Wa-l -Wf-ba0 -c -o starstar_map.o starstar_map.c
C:\gbdk\bin\lcc -Wa-l -Wf-ba0 -c -o menu.o menu.c
C:\gbdk\bin\lcc -Wa-l -Wf-ba0 -c -o deadscreen.o deadscreen.c
C:\gbdk\bin\lcc -Wa-l -c -o main.o main.c
C:\gbdk\bin\lcc -Wl-yt3 -Wl-yo4 -Wl-ya4 -o main.gb main.o savestate.o short.o FinalTiles.o level800.o newKey.o records.o starstar_data.o starstar_map.o menu.o deadscreen.o
