# Earfun protocol

# Set name
```
0000   ff 04 00 10 00 0a 03 16 45 61 72 46 75 6e 20 41   ........EarFun A
0010   69 72 20 50 72 6f 20 34                           ir Pro 4
```

# Set game mode
```
FF 04 00 01 00 0A 03 12 01 # Turn on
  resp: FF 03 00 02 00 0A 03 13 00 01
FF 04 00 01 00 0A 03 12 00 # Turn off
  resp: FF 03 00 02 00 0A 03 13 00 00
```
