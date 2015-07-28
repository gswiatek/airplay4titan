# AirPlay for TitanNit

## Introduction

AirPlay support for TitanNit receivers.

Using AirPlay you can stream videos from your iOS devices (iPad, iPhone) or iTunes 
to your sat or cable receiver with TitanNit image. It means you receive becomes AirPlay server like AppleTV.

For more details about TitanNit please see [TitanNit-Wiki](http://sbnc.dyndns.tv/trac/wiki) or
read the [AAF-forum](http://www.aaf-digital.info/forum/forumdisplay.php?278-TitanNit).

The implemtation is based on the [Unofficial AirPlay Protocol Specification](http://nto.github.io/AirPlay.html).

## Manual Installation

Depending on your receiver you need `bonjour` and `airplay` from the `sh4` or `mips` directory.
You should copy the raw files with FTP client to the receiver and store both for example under `/mnt/bin`.

Please ensure that both files are marked as executable. Log on  with telnet on the receiver and execute following commands:
```
chmod +x /mnt/bin/bonjour
chmod +x /mnt/bin/airplay
```

Please add following lines to the file `/mnt/config/usercmd.sh`:
```
/mnt/bin/bonjour&
/mnt/bin/airplay
```

