@echo off

cls

set version=1.0

set wat=Dayz SA

title %wat% Watchdog

:watchdog

echo (%time%) %wat% started.

start "Datz_SA" /wait /affinity FF /high "DayZServer_x64.exe" -config=enoch.cfg -servermod=@DayZ-Dynamic-AI-Addon -mod=@Better-Snap-Sounds;@CF;@Dabs_Framework;@DayZ-Expansion-AI;@DayZ-Expansion-Book;@DayZ-Expansion-Core;@DayZ-Expansion-Groups;@DayZ-Expansion-Navigation;@GoreZ;@InediaInfectedAI;@ViewInventoryAnimation;@VPP;@ZenSleep -profiles=.\Profiles\survive

echo (%time%) %wat% closed or crashed, restarting.

goto watchdog