C0 = 16.35
D0 = 18.35
E0 = 20.60
F0 = 21.83
G0 = 24.50
A0 = 27.50
B0 = 30.87
C1 = 32.70
D1 = 36.71
E1 = 41.20
F1 = 43.65
G1 = 49.00
A1 = 55.00
B1 = 61.74
C2 = 65.41
D2 = 73.42
E2 = 82.41
F2 = 87.31
G2 = 98.00
A2 = 110.00
B2 = 123.47
C3 = 130.81
D3 = 146.83
E3 = 164.81
F3 = 174.61
G3 = 196.00
A3 = 220.00
B3 = 246.94
C4 = 261.63
D4 = 293.66
E4 = 329.63
F4 = 349.23
G4 = 392.00
A4 = 440.00
B4 = 493.88
C5 = 523.25
D5 = 587.33
E5 = 659.26
F5 = 698.46
G5 = 783.99
A5 = 880.00
B5 = 987.77
C6 = 1046.50
D6 = 1174.66
E6 = 1318.51
F6 = 1396.91
G6 = 1567.98
A6 = 1760.00
B6 = 1975.53
C7 = 2093.00
D7 = 2349.32
E7 = 2637.02
F7 = 2793.83
G7 = 3135.96
A7 = 3520.00
B7 = 3951.07
C8 = 4186.01
D8 = 4698.64
rest = null

BUFFER = []
BPM = undefined

bpm = (bpm) -> BPM = bpm

play = (denominator, note) -> BUFFER.push [60 / (BPM * denominator), note]

fin = -> throw "fin"

whole       = (note) -> play 0.25, note
half        = (note) -> play 0.5, note
quarter     = (note) -> play 1, note
eighth      = (note) -> play 2, note
sixteenth   = (note) -> play 4, note

song = (callback) ->
    OS.open "/dev/sound", (err, fd) ->
        try
            do callback
        catch e
            throw e unless e is "fin"
        i = 0
        playNext = ->
            if i == BUFFER.length
                do OS.exit
            else
                [time, freq] = BUFFER[i]
                if freq
                    OS.ioctl fd, "play", freq
                else
                    OS.ioctl fd, "stop"
                i += 1
                OS.alarm time * 1000, playNext

## bounce... ##
song ->
    bpm 128
    
    eighth  F2
    eighth  D3
    eighth  C3
    quarter B2
    quarter C3
    quarter G2
    eighth  A2
    quarter E2
    quarter F2
    quarter C3
    eighth  rest
    eighth  E2
    eighth  F2
    quarter G2
    quarter A2
    quarter E2
    eighth  D2
    quarter B1
    quarter A1
    
    fin