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

The other limitation is that the playing starts always at the beginning of the stream (no resume).
You can't also seek to a desired position using the AirPlay client, but if you want fast foward the stream (except live stream), you can use the keys 3, 6 and 9 of the remote control.

## Prerequisites

You need TitanNit STB with image version **not older than 1.59**. For installation you need about **200KB** free space on the disk.
The `airplay` program writes debug logs into the file `/tmp/airplay.log`. If the log file reaches 200KB a roll-up will be performed (current log will be moved to `/tmp/airplay.log.1` and new log messages will be written to the fresh `/tmp/airplay.log` file). It means about **400KB** free space is required for the log files during run-time.


## Manual Installation

Depending on your receiver you need `bonjour` and `airplay` from the `sh4` respectively `mips` directory.
You should copy the raw files with FTP client to the receiver and store both for example under `/mnt/bin`.

Please ensure that the files are marked as executables. Log on  with `telnet` client on the receiver and execute following commands:
```
chmod +x /mnt/bin/bonjour
chmod +x /mnt/bin/airplay
```

Please add following two lines to the file `/mnt/config/usercmd.sh` if you want to start the programs automatically:
```
/mnt/bin/bonjour&
/mnt/bin/airplay
```

Optionally you can pass a name to the first program, 
e.g. `/mnt/bin/bonjour NemesisTV&` or `/mnt/bin/bonjour AM510HV&`. 
This name identifies your device. The default is `TitanNitTV`.

## Plug-in Installation

After successfull user tests the Plug-in will be prepared, but currently you should use the manual installation.

## Current Status (by developer)

Here is a list with working clients:
* iTunes 12.2 on Windows 8.1
* Synology DS213+ with DSM 5.2 Video Station
* Safari browser unter iOS

And here some details about tested video portal services with Safari browser:

Site | Status | Comment
----- | ----- | ----
[YouTube](https://m.youtube.com/) | working | 
[TED](https://www.ted.com/) | working |
[Das Erste](http://mediathek.daserste.de/) | working (except m3u) | Please change the quality setting from automatic to other value (e.g. highest)
[ARD](http://www.ardmediathek.de/tv) | working (except m3u) | Please change the quality setting from automatic to other value (e.g. highest)
[Dailymotion](www.dailymotion.com/) | working |
[Vimeo](https://vimeo.com/) | not working | Playing rate is not ok (slow motion)
[ZDF](http://www.zdf.de/ZDFmediathek) | not working | Playing rate is not ok (slow motion)
[ServusTV](http://www.servustv.com) | not working | high stutter
[ORF TVthek](http://tvthek.orf.at/) | not working | box crashes during m3u playback
[Filmon](http://www.filmon.com) | not working | box crashes during m3u playback

The developer tests were performed with AirPlay installed on **Atemio510HD** with the latest stable 1.67 image.

## Information for Android Users

You can install an AirPlay app on your Android device and can also stream videos from your device to TianNit STB.
There are many apps available. I have tested [AllCast](https://play.google.com/store/apps/details?id=com.koushikdutta.cast&hl=en).

Here are steps required for streaming YouTube videos:

1. Use share icon for desired video in YouTube app
   ![YouTube](https://cloud.githubusercontent.com/assets/3924713/9145367/018f2068-3d54-11e5-8f9a-c8ad3e0e44cf.png)
2. Select AllCast app to be used for sharing
   ![Select App](https://cloud.githubusercontent.com/assets/3924713/9145375/14000348-3d54-11e5-912f-04bd5b29f807.png)
3. Select the AirPlay device in AllCast
   ![Select Device](https://cloud.githubusercontent.com/assets/3924713/9145382/25168c92-3d54-11e5-9542-eca0392457b0.png)
4. Pause or stop the video using AllCast control
  ![Control](https://cloud.githubusercontent.com/assets/3924713/9145385/28f04aba-3d54-11e5-91e0-a6e622551de1.png)
