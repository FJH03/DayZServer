@echo off

cls

set version=1.0

set wat=Dayz SA

title %wat% Watchdog

:watchdog

echo (%time%) %wat% started.

start "Datz_SA" /wait /affinity FF /high "DayZServer_x64.exe" -config=chernarusplus.cfg -mod=@Better-Snap-Sounds;@CF;@Dabs_Framework;@DayZ-Expansion-AI;@DayZ-Expansion-Book;@DayZ-Expansion-Core;@DayZ-Expansion-Groups;@DayZ-Expansion-Navigation;@ViewInventoryAnimation -profiles=O:\SteamLibrary\steamapps\common\DayZServer\Profiles\survive

echo (%time%) %wat% closed or crashed, restarting.

goto watchdog