@echo off

cls

set version=1.0

set wat=Dayz SA

title %wat% Watchdog

:watchdog

echo (%time%) %wat% started.

start "Datz_SA" /wait /affinity FF /high "DayZServer_x64.exe" -config=chernarusplus_snow.cfg -servermod=@DayZ-Dynamic-AI-Addon -mod=@Better-Snap-Sounds;@CF;@Dabs_Framework;@DayZ-Expansion-AI;@DayZ-Expansion-Book;@DayZ-Expansion-Core;@DayZ-Expansion-Groups;@DayZ-Expansion-Navigation;@GoreZ;@InediaInfectedAI;@ViewInventoryAnimation;@VPP;@Winter_Chernarus_V2;@ZenSleep -profiles=.\Profiles\survive

echo (%time%) %wat% closed or crashed, restarting.

goto watchdog