# AirPlay for TitanNit

## Introduction

Using AirPlay you can stream videos from iOS devices (iPad, iPhone) or iTunes program
to your STB (sat or cable) with TitanNit image. It means your STB will act as AirPlay server like AppleTV.

Please see official [German](http://www.apple.com/de/airplay/) and [English](http://www.apple.com/airplay/) introduction to AirPlay.

For more details about TitanNit please read [TitanNit-Wiki](http://sbnc.dyndns.tv/trac/wiki) or
the [AAF-forum](http://www.aaf-digital.info/forum/forumdisplay.php?278-TitanNit).

The implemtation is based on the [Unofficial AirPlay Protocol Specification](http://nto.github.io/AirPlay.html).

Current implementation supports only video streaming.

Not supported are:
* Music
* Photos
* DRM protected videos
* Mirroring

## Manual Installation

Depending on your receiver you need `bonjour` and `airplay` from the `sh4` or `mips` directory.
You should copy the raw files with FTP client to the receiver and store both for example under `/mnt/bin`.

Please ensure that both files are marked as executables. Log on  with `telnet` client on the receiver and execute following commands:
```
chmod +x /mnt/bin/bonjour
chmod +x /mnt/bin/airplay
```

Please add following lines to the file `/mnt/config/usercmd.sh` if you want to start the programs automatically:
```
/mnt/bin/bonjour&
/mnt/bin/airplay
```

Optionally you can pass a name to the first program, e.g. `/mnt/bin/bonjour NemesisTV&` or `/mnt/bin/bonjour AM510HV&`. This name identifies your device. The default is `TitanNitTV`.

## Plug-in Installation

After successfull user tests the Plug-in will be prepared, but currently you should use the manual installation.

## Current Status (by developer)

Here is a list with working clients:
* iTunes on Windows 8.1
* Synology DSM Video Station
* Safari browser

And here some details about tested sites with Safari browser:

Site | Status | Comment
----- | ----- | ----
[YouTube](https://m.youtube.com/) | working | 
[Das Erste](http://mediathek.daserste.de/) | working (except m3u) | Please change the quality setting from automatic to other value
[ORF TVthek](http://tvthek.orf.at/) | not working | box crashes during m3u playback
[Filmon](http://www.filmon.com) | not workin | box crashes during m3u playback

 
