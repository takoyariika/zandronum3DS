param([string]$Server='zandronum.exe',[string]$Citra='citra.exe',[string]$Python='python',[switch]$JoinTest,[switch]$ControlsTest,[switch]$RoundTest,[switch]$ReturnLauncherTest,[ValidateSet('doom','byoc')][string]$Mode='doom',[string]$PlayerClass='DoomGuy')
$ErrorActionPreference='Stop'
$portRoot=Split-Path -Parent $PSScriptRoot
$portBuild=Join-Path $portRoot 'build-3ds'
$portConfig=Join-Path $portBuild '011-server.ini'
$portLog=Join-Path $portBuild '011-server.log'
$portStarted=[DateTime]::UtcNow
# All server state belongs to this workspace; never advertise the test server.
Set-Content -LiteralPath $portConfig -Value '[GlobalSettings]' -Encoding ascii
$portArgs=@('-host','-port','10667','-useip','127.0.0.1','-nobroadcast','-noautoload','-config',('"'+$portConfig+'"'),'-iwad',('"'+(Join-Path $portBuild 'sdmc\3ds\zandronum\DOOM2.WAD')+'"'),'+sv_updatemaster','0','+sv_broadcast','0','+sv_maxclients','4','+sv_hostname','3DS-test','+logfile',('"'+$portLog+'"'),'+map','MAP01')
$portNativeMods=@()
if($Mode -eq 'byoc') {
    $portSkulltag=Join-Path $env:APPDATA 'Citra\sdmc\3ds\zandronum\download-test\skulltag_content-3.2-beta2.pk3'
    $portMods=@($portSkulltag,(Join-Path $portBuild 'DWANGO5.WAD'),(Join-Path $portBuild 'sdmc\3ds\zandronum\BYOCv2.0.pk3'),(Join-Path $portBuild 'tspg_connectioncolumn_r1.wad'))
    foreach($portMod in $portMods) { if(-not(Test-Path -LiteralPath $portMod)) { throw 'Missing round test MOD fixture.' } }
    $portArgs=@('-file')+@($portMods | ForEach-Object {'"'+$_+'"'})+$portArgs
    $portNativeMods=@('sdmc:/3ds/zandronum/download-test/skulltag_content-3.2-beta2.pk3','sdmc:/3ds/zandronum/DWANGO5.WAD','sdmc:/3ds/zandronum/BYOCv2.0.pk3','sdmc:/3ds/zandronum/tspg_connectioncolumn_r1.wad')
}
if($RoundTest) { $portArgs+=@('+deathmatch','1','+timelimit','0.2','+fraglimit','0','+sv_maprotation','1','+addmap','MAP01','+addmap','MAP02') }
if($RoundTest -or $ControlsTest) { $portArgs=@('-nomonsters')+$portArgs }
$portProcess=Start-Process -FilePath $Server -WorkingDirectory $portBuild -ArgumentList $portArgs -WindowStyle Hidden -PassThru
try {
    $portDeadline=[DateTime]::UtcNow.AddSeconds($(if($Mode -eq 'byoc'){180}else{30}))
    while([DateTime]::UtcNow -lt $portDeadline) {
        if($portProcess.HasExited) { throw 'Desktop server exited during startup' }
        $portActualLog=Get-ChildItem -LiteralPath $portBuild -Filter '011-server.log*' | Where-Object { $_.LastWriteTimeUtc -ge $portStarted } | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
        if($portActualLog -and (Select-String -LiteralPath $portActualLog.FullName -Pattern 'MAP01|entryway' -Quiet)) { $portLog=$portActualLog.FullName; break }
        Start-Sleep -Milliseconds 500
    }
    if($ReturnLauncherTest) {
        & (Join-Path $PSScriptRoot 'run_citra.ps1') -Citra $Citra -Python $Python -Mode doom -Format cia -Seconds 240 -NetworkTestAddress '127.0.0.1:10667' -ReturnLauncherTest -AudioTest -TextureCache -ResourceCache
    } elseif($RoundTest) {
        $portReplay=@{Mode=$Mode;Format='cia';Seconds=600;NetworkTestAddress='127.0.0.1:10667';RoundTest=$true;QuitTest=$true;AudioTest=$true;QuietDiagnostics=$true;HoldAfterCapture=$true;TextureCache=$true;ResourceCache=$true}
        if($Mode -eq 'byoc') { $portReplay.NetworkModFiles=$portNativeMods; $portReplay.ExtraWads=@((Join-Path $portBuild 'DWANGO5.WAD'),(Join-Path $portBuild 'tspg_connectioncolumn_r1.wad')); $portReplay.PlayerClass=$PlayerClass }
        & (Join-Path $PSScriptRoot 'run_citra.ps1') -Citra $Citra -Python $Python @portReplay
    } else {
        & (Join-Path $PSScriptRoot 'run_citra.ps1') -Citra $Citra -Python $Python -Mode doom -Format cia -Seconds 150 -NetworkTestAddress '127.0.0.1:10667' -InputTest -TouchTest -QuitTest -AudioTest -QuietDiagnostics -DoomListTest -HoldAfterCapture -TextureCache -ResourceCache -JoinTest:$JoinTest -ControlsTest:$ControlsTest
    }
    if(-not (Select-String -LiteralPath $portLog -Pattern 'has connected|entered the game' -Quiet)) { throw '3DS client did not enter the desktop server' }
    $portClientLog=Join-Path $portBuild "$Mode-cia-engine.log"
    if($ReturnLauncherTest) { $portClientLog=Join-Path $portBuild 'doom-cia-startup.log' }
    if(-not (Select-String -LiteralPath $portClientLog -Pattern 'Snapshot received' -Quiet)) { throw '3DS client did not receive the server map snapshot' }
    if(-not $RoundTest -and -not $ReturnLauncherTest -and -not (Select-String -LiteralPath $portClientLog -Pattern 'INPUT TEST completed' -Quiet)) { throw 'Online input replay did not finish' }
    if($RoundTest -and -not(Select-String -LiteralPath $portClientLog -Pattern 'ROUND TEST PASS' -Quiet)) { throw 'Two result/map transitions did not finish' }
    if($JoinTest -and -not (Select-String -LiteralPath (Join-Path $portBuild 'doom-cia-engine.log') -Pattern 'JOIN TEST: PASS' -Quiet)) { throw 'A-button spectator join did not finish' }
    Get-Content -LiteralPath $portLog -Tail 16
    if($ReturnLauncherTest) {
        Write-Output '3DS RETURN TEST: stock 3.2.1 online game -> launcher -> offline game -> launcher -> HOME exit passed.'
    } else {
        Write-Output '3DS NETWORK TEST: stock 3.2.1 server join, online input and normal quit passed.'
    }
} finally {
    $portProcess.Refresh()
    if(-not $portProcess.HasExited) { Stop-Process -Id $portProcess.Id }
}
