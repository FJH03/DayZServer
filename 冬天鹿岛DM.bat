@echo off

cls

set version=1.0

set wat=Dayz SA

title %wat% Watchdog

:watchdog

echo (%time%) %wat% started.

start "Datz_SA" /wait /affinity FF /high "DayZServer_x64.exe" -config=enoch_DM_winter.cfg -servermod=@CrimsonZamboniDeathmatch;@DayZ-Dynamic-AI-Addon -mod=@Air_Strike;@Better-Snap-Sounds;@BulletStacks;@CF;@Dabs_Framework;@DayZ-Expansion-AI;@DayZ-Expansion-Book;@DayZ-Expansion-Core;@DayZ-Expansion-Groups;@DayZ-Expansion-Navigation;@Drones;@GoreZ;@InediaInfectedAI;@ViewInventoryAnimation;@VPP;@Winter_Chernarus_V2;@Winter_Livonia -profiles=.\Profiles\DM

echo (%time%) %wat% closed or crashed, restarting.

goto watchdog
