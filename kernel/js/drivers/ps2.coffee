class PS2
    constructor: ->
        @irq = 33
        @port = 0x60
    
    enable: ->
        Kernel.isrs[@irq] = =>
            @onInterrupt()
    
    disable: ->
        Kernel.isrs[@irq] = null
    
    onInterrupt: ->
        scanCode = Kernel.inb @port
        @onScanCode?(scanCode)

Drivers.PS2 = PS2