## Turn off in ear detection
```
55 60 01 04 F0 03 00 1F 01 01 00 BE 4C
# turn off:
55 60 01 04 F0 03 00 20 01 01 01 73 98
```
## Ultra bass
```
# level 4
4
5
4
3
2
1
3
1
3
x
off
55 60 01 51 F0 02 00 10 01 08 26 FE # 4
55 60 01 51 F0 02 00 11 01 0A F6 FF # 5
55 60 01 51 F0 02 00 12 01 08 87 3E # 4
55 60 01 51 F0 02 00 13 01 06 57 3A # 3
55 60 01 51 F0 02 00 14 01 04 67 3A # 2
55 60 01 51 F0 02 00 15 01 02 B6 F8 # 1
55 60 01 51 F0 02 00 16 01 06 47 3B # 3
55 60 01 51 F0 02 00 17 01 02 17 38 # 1
55 60 01 51 F0 02 00 18 01 06 26 F8 # 3
55 60 01 51 F0 02 00 19 00 06 76 A8 # off
```
# Low lag mode
```
turn onL
55 60 01 40 F0 01 00 1A 01 5A C2
turn off:
55 60 01 40 F0 01 00 1B 02 1B 53
```
# Fw update thing?
```
55 60 01 0A C0 00 00 1C 41 17
```
# Noise cancellation
```
55 60 01 0F F0 03 00 25 01 03 00 F2 47 # low
55 60 01 0F F0 03 00 26 01 02 00 F3 93 # mid
55 60 01 0F F0 03 00 27 01 01 00 F2 9F # high
55 60 01 0F F0 03 00 28 01 04 00 F2 DB # adaptive
55 60 01 0F F0 03 00 29 01 07 00 F3 D7 # transparency mode
55 60 01 0F F0 03 00 2A 01 05 00 F2 F3 # noise cancellation off
55 60 01 0F F0 03 00 2B 01 04 00 F2 9F # set to 'noise cancellation' 
```
# Equalizer presets
```
55 60 01 1D F0 02 00 13 00 00 5B 6C # DIRAC OPTEO
55 60 01 1D F0 02 00 14 01 00 EB 3D # Rock
55 60 01 1D F0 02 00 15 05 00 B8 3D # Classical
55 60 01 1D F0 02 00 16 03 00 4B 9D # Pop
55 60 01 1D F0 02 00 17 04 00 18 6D # Enhance vocals
55 60 01 1D F0 02 00 18 02 00 2B CE # Electronic
55 60 01 1D F0 02 00 19 03 00 7B 9E # Pop
```
## Touch settings
```
55 60 01 18 C0 00 00 3A 78 CE # double tap left = play/pause
55 60 01 18 C0 00 00 3C F8 CC # double tap left = skip back
55 60 01 18 C0 00 00 3E 79 0D # double tap left = skip forward
55 60 01 18 C0 00 00 40 F9 2D # double tap left = voice assistant
55 60 01 18 C0 00 00 42 78 EC # double tap left = no extra action
55 60 01 18 C0 00 00 45 39 2E # triple tap left = voice assistant
```


```
0000   55 60 01 18 40 3d 00 07 0f 02 01 02 01 02 01 03
0010   01 02 01 07 01 02 01 09 01 03 01 02 09 03 01 03
0020   08 03 01 07 0a 03 01 09 01 04 01 01 01 04 01 02
0030   01 04 09 02 01 04 01 03 01 04 09 03 01 04 01 07
0040   01 04 01 0a 17 a6 93

```
