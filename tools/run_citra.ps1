param(
    [ValidateSet('doom','byoc')][string]$Mode = 'doom',
    [ValidateRange(10,600)][int]$Seconds = 30,
    [switch]$Gdb,
    [switch]$PlayTest,
    [switch]$InputTest,
    [switch]$ControlsTest,
    [switch]$BottomTest,
    [switch]$ReturnLauncherTest,
    [switch]$RoundTest,
    [switch]$CombatTest,
    [switch]$AudioTest,
    [switch]$QuietDiagnostics,
    [switch]$DoomListTest,
    [switch]$HttpTest,
    [switch]$BrowserTest,
    [switch]$DownloadTest,
    [switch]$DownloadReuse,
    [switch]$SingleDownload,
    [switch]$LargeDownloadOnly,
    [switch]$GenericLauncherTest,
    [switch]$JoinTest,
    [switch]$NoBYOC,
    [string]$NetworkTestAddress,
    [string[]]$NetworkModFiles,
    [string]$ExtraWad,
    [string[]]$ExtraWads,
    [switch]$FatalTest,
    [switch]$MenuTest,
    [switch]$AnimatedTitle,
    [switch]$LegacyTextureClock,
    [switch]$StopTitleMusic,
    [switch]$AudioDecoderTest,
    [switch]$ShotgunTest,
    [switch]$QuitTest,
    [switch]$SleepTest,
    [switch]$LauncherTest,
    [switch]$TouchTest,
    [switch]$TitleTest,
    [ValidateRange(0,240)][int]$TitleSoakSeconds = 0,
    [switch]$TransitionTest,
    [switch]$RestoreArgsOnly,
    [string]$PlayerClass,
    [string]$Map = 'MAP01',
    [switch]$ShowWindow,
    [switch]$HoldAfterCapture,
    [switch]$TextureCache,
    [switch]$ResourceCache,
    [ValidateSet('3dsx','cia')][string]$Format = '3dsx',
    [string]$Citra = 'citra.exe',
    [string]$Python = 'python'
)
$ErrorActionPreference = 'Stop'
$portRoot = Split-Path -Parent $PSScriptRoot
$portBuild = Join-Path $portRoot 'build-3ds'
$portSource = Join-Path $portBuild 'sdmc\3ds\zandronum'
$portRom = Join-Path $portBuild "dist\zandronum.$Format"
if (-not (Test-Path -LiteralPath $portRom)) { throw 'Build the game 3DSX first.' }
$portSdRoot = [IO.Path]::GetFullPath((Join-Path $env:APPDATA 'Citra\sdmc'))
$portDestination = [IO.Path]::GetFullPath((Join-Path $portSdRoot '3ds\zandronum'))
if (-not $portDestination.StartsWith($portSdRoot + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid SD destination.' }
New-Item -ItemType Directory -Path $portDestination -Force | Out-Null
$portNormalArgs = if ($Mode -eq 'doom') { '' } else { '-file sdmc:/3ds/zandronum/BYOCv2.0.pk3' }
if ($RestoreArgsOnly) {
    Set-Content -LiteralPath (Join-Path $portDestination 'args.txt') -Value $portNormalArgs -Encoding ascii
    return
}
$portPreviousArgs = $portNormalArgs
if (Test-Path -LiteralPath (Join-Path $portDestination 'args.txt')) {
    $portExistingArgs = Get-Content -LiteralPath (Join-Path $portDestination 'args.txt') -Raw
    if ($portExistingArgs -notmatch '-3ds-test') { $portPreviousArgs = $portExistingArgs }
}
foreach ($portName in @('zandronum.pk3','skulltag_actors.pk3','DOOM2.WAD','BYOCv2.0.pk3')) {
    $portInput = Join-Path $portSource $portName
    $portOutput = Join-Path $portDestination $portName
    if ((Test-Path -LiteralPath $portInput) -and ($portName -in @('zandronum.pk3','skulltag_actors.pk3') -or -not (Test-Path -LiteralPath $portOutput))) {
        Copy-Item -LiteralPath $portInput -Destination $portOutput -Force
    }
}
foreach ($portName in @('zandronum.pk3','BYOCv2.0.pk3')) {
    $portCacheOutput = Join-Path $portDestination ($portName + '.3ds-cache')
    $portCacheDisabled = $portCacheOutput + '.disabled'
    if ($TextureCache) {
        $portCacheInput = Join-Path $portSource ($portName + '.3ds-cache')
        if (Test-Path -LiteralPath $portCacheInput) { Copy-Item -LiteralPath $portCacheInput -Destination $portCacheOutput -Force }
    } elseif (Test-Path -LiteralPath $portCacheOutput) {
        Move-Item -LiteralPath $portCacheOutput -Destination $portCacheDisabled -Force
    }
}
foreach ($portName in @('zandronum.pk3','DOOM2.WAD','BYOCv2.0.pk3')) {
    $portCacheOutput = Join-Path $portDestination ($portName + '.3ds-resources')
    if ($ResourceCache) {
        $portCacheInput = Join-Path $portSource ($portName + '.3ds-resources')
        if (Test-Path -LiteralPath $portCacheInput) { Copy-Item -LiteralPath $portCacheInput -Destination $portCacheOutput -Force }
    } elseif (Test-Path -LiteralPath $portCacheOutput) {
        Move-Item -LiteralPath $portCacheOutput -Destination ($portCacheOutput + '.disabled') -Force
    }
}
New-Item -ItemType Directory -Path (Join-Path $portDestination 'screenshots') -Force | Out-Null
foreach($portExtraSource in (@($ExtraWad)+@($ExtraWads))) {
    if(-not $portExtraSource) { continue }
    $portExtraPath=[IO.Path]::GetFullPath($portExtraSource)
    if (-not $portExtraPath.StartsWith($portBuild+'\',[StringComparison]::OrdinalIgnoreCase) -or [IO.Path]::GetExtension($portExtraPath) -ine '.wad') { throw 'Extra test WAD must be inside build-3ds.' }
    Copy-Item -LiteralPath $portExtraPath -Destination (Join-Path $portDestination (Split-Path $portExtraPath -Leaf)) -Force
}
$portArgs = if ($Mode -eq 'doom') { '+map MAP01 -nosound -3ds-test' } else { '-file sdmc:/3ds/zandronum/BYOCv2.0.pk3 -nosound -3ds-test +map MAP01' }
if ($NetworkModFiles) {
    if ($NetworkModFiles | Where-Object {$_ -match '\s|"'}) { throw 'Test args.txt paths must not contain spaces or quotes; use the native launcher for those paths.' }
    $portArgs='-file '+($NetworkModFiles -join ' ')+' -nosound -3ds-test +map MAP01'
}
$portArgs = $portArgs.Replace('+map MAP01', '+map ' + $Map)
if ($TitleTest) { $portArgs = $portArgs.Replace('+map ' + $Map,''); $portArgs += ' -3ds-test-title' }
if ($TitleSoakSeconds) { $portArgs += ' -3ds-test-title-soak ' + $TitleSoakSeconds }
if ($AnimatedTitle) { $portArgs += ' +native_titlemap_3ds 1' } else { $portArgs += ' +native_titlemap_3ds 0' }
if ($LegacyTextureClock) { $portArgs += ' +native_texture_clock_3ds 0' }
if ($StopTitleMusic) { $portArgs += ' -3ds-test-title-stopmusic' }
if ($MenuTest) { $portArgs=$portArgs.Replace('+map ' + $Map,''); $portArgs += ' -3ds-test-menu'; $HoldAfterCapture=$true }
if ($TransitionTest) { $portArgs += ' -3ds-test-transition' }
if ($PlayTest) { $portArgs += ' -3ds-test-play' }
if ($InputTest) { $portArgs += ' -3ds-test-input +use_joystick 0' }
if ($ControlsTest) { $portArgs += ' -3ds-test-controls -nomonsters' }
if ($BottomTest) { $portArgs += ' -3ds-test-bottom -nomonsters +native_touch_layout_3ds 0'; $HoldAfterCapture=$true; $QuitTest=$true }
if ($ReturnLauncherTest) { $portArgs += ' -3ds-test-return-launcher -3ds-test-quit -nomonsters'; $HoldAfterCapture=$true }
if ($RoundTest) { $portArgs += ' -3ds-test-round +cl_startasspectator 0' }
if ($CombatTest) { $portArgs += ' -3ds-test-combat' }
if ($AudioTest) { $portArgs = $portArgs.Replace('-nosound','') }
if ($QuietDiagnostics) { $portArgs += ' -3ds-quiet' }
if ($HttpTest -or $DownloadTest -or $BrowserTest) { $portArgs += ' -3ds-test-http -3ds-test-utc '+[DateTimeOffset]::UtcNow.ToUnixTimeSeconds() }
if ($BrowserTest) { $portArgs += ' -3ds-test-browser' }
if ($DownloadTest) { $portArgs += ' -3ds-test-download' }
if ($DownloadReuse) { $portArgs += ' -3ds-test-download-reuse' }
if ($SingleDownload) { $portArgs += ' -3ds-download-single' }
if ($LargeDownloadOnly) { $portArgs += ' -3ds-test-download-large' }
if ($GenericLauncherTest) { $portArgs += ' -3ds-test-launcher-generic' }
if ($JoinTest) { $portArgs += ' -3ds-test-join +cl_startasspectator 1' }
if ($DoomListTest) {
    $portArgs += ' -3ds-test-doomlist'
    Copy-Item -LiteralPath (Join-Path $portBuild 'doomlist-full.json') -Destination (Join-Path $portDestination 'doomlist-test.json') -Force
}
if ($NetworkTestAddress) { $portArgs=$portArgs.Replace('+map '+$Map,''); $portArgs += ' -port 10668 -3ds-test-network '+$NetworkTestAddress }
if ($FatalTest) { $portArgs += ' -3ds-test-fatal'; $HoldAfterCapture=$true }
if ($AudioDecoderTest) {
    Copy-Item -LiteralPath (Join-Path $portSource 'audio-fixture.wad') -Destination $portDestination
    $portArgs=$portArgs.Replace('-nosound','')+' -file sdmc:/3ds/zandronum/audio-fixture.wad -3ds-test-audio'
    $HoldAfterCapture=$true
}
if ($ShotgunTest) { $portArgs += ' -3ds-test-shotgun'; $HoldAfterCapture=$true }
if ($QuitTest) { $portArgs += ' -3ds-test-quit' }
if ($SleepTest) { $portArgs += ' -3ds-test-sleep' }
if ($LauncherTest) { $portArgs += ' -3ds-test-launcher' }
if ($TouchTest) { $portArgs += ' -3ds-test-touch +touch_sensitivity_3ds 12' }
if ($LauncherTest) {
    $portChooserFixture = Join-Path $portDestination 'Chooser test.wad'
    if (Test-Path -LiteralPath $portChooserFixture) { throw 'Launcher fixture path already exists.' }
    [IO.File]::WriteAllBytes($portChooserFixture, [byte[]](80,87,65,68,0,0,0,0,12,0,0,0))
}
$portClassFixture=$null
if ($PlayerClass) {
    if($PlayerClass -match '\s') {
        if($PlayerClass -match '["\\\r\n]') { throw 'Unsupported test class name quoting.' }
        $portClassFixture=Join-Path $portDestination '3ds-test-playerclass.cfg'
        if(Test-Path -LiteralPath $portClassFixture) { throw 'Class fixture already exists.' }
        Set-Content -LiteralPath $portClassFixture -Value ('playerclass "'+$PlayerClass+'"') -Encoding utf8
        $portArgs+=' +exec sdmc:/3ds/zandronum/3ds-test-playerclass.cfg'
    } else { $portArgs += ' +playerclass ' + $PlayerClass }
}
Set-Content -LiteralPath (Join-Path $portDestination 'args.txt') -Value $portArgs -Encoding ascii
$portLabel = "$Mode-$Format"
$portStdout = Join-Path $portBuild "citra-$portLabel-stdout.log"
$portStderr = Join-Path $portBuild "citra-$portLabel-stderr.log"
if ($Format -eq 'cia') {
    $portInstall = Start-Process -FilePath $Citra -ArgumentList @('-i',('"'+$portRom+'"')) -WindowStyle Hidden -RedirectStandardOutput (Join-Path $portBuild 'citra-install-stdout.log') -RedirectStandardError (Join-Path $portBuild 'citra-install-stderr.log') -PassThru
    if (-not $portInstall.WaitForExit(30000)) { Stop-Process -Id $portInstall.Id; throw 'Citra CIA installation timed out.' }
    $portInstalled = Get-ChildItem -LiteralPath (Join-Path $portSdRoot 'Nintendo 3DS') -Directory -Filter '0fd73300' -Recurse | Select-Object -First 1
    if (-not $portInstalled) { throw 'Installed Zandronum CIA title was not found.' }
    $portRom = (Get-ChildItem -LiteralPath (Join-Path $portInstalled.FullName 'content') -Filter '*.app' | Sort-Object Name | Select-Object -First 1).FullName
    if (-not $portRom) { throw 'Installed CIA content was not found.' }
}
$portStartedUtc = [DateTime]::UtcNow
$portQuotedRom = '"' + $portRom + '"'
$portCitraArguments = if ($Gdb) { @('-g','24689',$portQuotedRom) } else { @($portQuotedRom) }
$portWindowStyle = if ($ShowWindow) { 'Normal' } else { 'Hidden' }
$portProcess=$null
$portHiddenMods=@()
try {
    if($NoBYOC) {
        if($Mode -ne 'doom') { throw 'NoBYOC requires the base-game test mode.' }
        foreach($portName in @('BYOCv2.0.pk3','BYOCv2.0.pk3.3ds-cache','BYOCv2.0.pk3.3ds-resources')) {
            $portOriginal=Join-Path $portDestination $portName
            $portHidden=$portOriginal+'.no-mod-test'
            if(Test-Path -LiteralPath $portHidden) { throw 'Diagnostic backup already exists; restore it first.' }
            if(Test-Path -LiteralPath $portOriginal) {
                Move-Item -LiteralPath $portOriginal -Destination $portHidden
                $portHiddenMods+=@{Original=$portOriginal;Hidden=$portHidden}
            }
        }
    }
    $portProcess = Start-Process -FilePath $Citra -ArgumentList $portCitraArguments -WindowStyle $portWindowStyle -RedirectStandardOutput $portStdout -RedirectStandardError $portStderr -PassThru
    $portProcess.Id | Set-Content -LiteralPath (Join-Path $portBuild 'citra-game.pid')
    if ($Gdb) {
         $portPython = $Python
        & $portPython (Join-Path $PSScriptRoot 'debug_citra.py')
    }
    $portDeadline = $portStartedUtc.AddSeconds($Seconds)
    while (-not $portProcess.HasExited -and [DateTime]::UtcNow -lt $portDeadline) {
        Start-Sleep -Seconds 1
        $portProcess.Refresh()
        $portFrame = Join-Path $portDestination $(if ($InputTest) {'screenshots\test-input.png'} elseif ($PlayTest) {'screenshots\test-play.png'} elseif ($TitleTest -and -not $TransitionTest) {'screenshots\test-title.png'} else {'screenshots\test-map.png'})
        if (-not $QuitTest -and -not $HoldAfterCapture -and (Test-Path -LiteralPath $portFrame) -and (Get-Item -LiteralPath $portFrame).LastWriteTimeUtc -ge $portStartedUtc) { break }
        $portFatal = Join-Path $portDestination 'fatal.txt'
        if (-not $FatalTest -and (Test-Path -LiteralPath $portFatal) -and (Get-Item -LiteralPath $portFatal).LastWriteTimeUtc -ge $portStartedUtc) { break }
    }
} finally {
    $portNaturalExit = $portProcess -and $portProcess.HasExited
    if ($portProcess -and -not $portProcess.HasExited) { Stop-Process -Id $portProcess.Id }
    foreach($portHiddenMod in $portHiddenMods) { Move-Item -LiteralPath $portHiddenMod.Hidden -Destination $portHiddenMod.Original }
    if($portClassFixture) { Remove-Item -LiteralPath $portClassFixture }
    Set-Content -LiteralPath (Join-Path $portDestination 'args.txt') -Value $portPreviousArgs -Encoding ascii
    if ($LauncherTest) { Remove-Item -LiteralPath $portChooserFixture }
    if ($AudioDecoderTest) { Remove-Item -LiteralPath (Join-Path $portDestination 'audio-fixture.wad') }
}
foreach ($portName in @('engine.log','startup.log','fatal.txt','fatal-bottom.rgb565','ndsp.txt','video.txt','performance.txt','video-top.raw','doomlist-test.txt','doomlist-http.txt','network.txt','download-test.txt','download.txt','launcher-test.txt','font-colors-test.txt','return-launcher.txt','ui-return-launcher-top.rgb565','ui-return-launcher-bottom.rgb565','ui-bottom-walk-bottom.rgb565','ui-bottom-scores-bottom.rgb565','ui-bottom-log-bottom.rgb565','ui-browser-top.rgb565','ui-browser-bottom.rgb565','ui-search-top.rgb565','ui-search-bottom.rgb565','screenshots\test-menu-main.png','screenshots\test-menu-class.png','screenshots\test-menu-move.png','screenshots\test-map.png','screenshots\test-play.png','screenshots\test-input.png','screenshots\test-title.png','apt-exit.txt','transition.txt','quit.txt','save\test.zds')) {
    $portOutput = Join-Path $portDestination $portName
    if ((Test-Path -LiteralPath $portOutput) -and (Get-Item -LiteralPath $portOutput).LastWriteTimeUtc -ge $portStartedUtc) {
        $portResultName = "$portLabel-" + (Split-Path $portName -Leaf)
        Copy-Item -LiteralPath $portOutput -Destination (Join-Path $portBuild $portResultName)
    }
}
$portLatestLog = Get-ChildItem -LiteralPath $portDestination -Filter 'engine.log*.log' | Where-Object {$_.LastWriteTimeUtc -ge $portStartedUtc} | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
if ($portLatestLog) {
    Copy-Item -LiteralPath $portLatestLog.FullName -Destination (Join-Path $portBuild "$portLabel-engine.log")
    Get-Content -LiteralPath $portLatestLog.FullName -Tail 30
}
Get-Content -LiteralPath $portStderr -Tail 12
if ($QuitTest -or $ShotgunTest -or $AudioDecoderTest -or $MenuTest -or $NetworkTestAddress -or $ReturnLauncherTest) {
    if (-not $portNaturalExit) { throw 'Game did not finish its normal exit before the deadline.' }
    if ($QuitTest -and (-not $portLatestLog -or -not (Select-String -LiteralPath $portLatestLog.FullName -Pattern '3DS QUIT TEST: normal menu exit path' -Quiet))) { throw 'Emulator exited before the gameplay quit marker; this is not a passing replay.' }
    if ($NetworkTestAddress -and -not $ReturnLauncherTest -and (-not $portLatestLog -or -not (Select-String -LiteralPath $portLatestLog.FullName -Pattern '3DS NETWORK TEST: normal exit after connection attempt' -Quiet))) { throw 'Emulator exited before the network quit marker.' }
    if (Select-String -LiteralPath $portStderr -Pattern 'unmapped (Read|Write)|Fatal Error|Assertion Failed' -Quiet) { throw 'Guest error during quit test.' }
    Write-Output '3DS QUIT TEST: natural emulator exit without unmapped memory errors.'
    if($ReturnLauncherTest) {
        $portReturnReport=Join-Path $portBuild "$portLabel-return-launcher.txt"
        if(-not (Test-Path -LiteralPath $portReturnReport) -or -not (Select-String -LiteralPath $portReturnReport -Pattern '^PASS: two game quits' -Quiet)) { throw 'Game to launcher to game to launcher to HOME replay failed.' }
        Get-Content -LiteralPath $portReturnReport
    }
    if($BottomTest) {
        if(-not (Select-String -LiteralPath $portLatestLog.FullName -Pattern '3DS BOTTOM TEST PASS:' -Quiet)) { throw 'Bottom UI replay did not pass.' }
        & $Python (Join-Path $PSScriptRoot 'verify_bottom_capture.py') (Join-Path $portBuild $portLabel) (Join-Path $portBuild "$portLabel-bottom-preview.png")
        if($LASTEXITCODE -ne 0) { throw 'Bottom LCD capture verification failed.' }
    }
    if($ControlsTest) {
        if($TouchTest -and -not (Select-String -LiteralPath $portLatestLog.FullName -Pattern '3DS TOUCH TEST:' -Quiet)) { throw 'Touch replay did not generate camera motion.' }
        & $Python (Join-Path $PSScriptRoot 'verify_native_input.py') (Join-Path $portBuild "$portLabel-engine.log")
        if($LASTEXITCODE -ne 0) { throw 'Native control semantics verification failed.' }
    }
}
