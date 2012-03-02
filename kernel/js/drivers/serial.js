(function() {
    var REG_DATA        = 0;
    var REG_INT_ENABLE  = 1;
    var REG_BAUD_LSB    = 0;
    var REG_BAUD_MSB    = 1;
    var REG_FIFO_CTRL   = 2;
    var REG_LINE_CTRL   = 3;
    var REG_MODEM_CTRL  = 4;
    var REG_LINE_STAT   = 5;
    var REG_MODEM_STAT  = 6;
    var REG_SCRATCH     = 7;
    
    function Serial(port) {
        this.port = port;
    }
    
    Serial.COM1 = 0x3f8;
    Serial.COM2 = 0x2f8;
    Serial.COM3 = 0x3e8;
    Serial.COM4 = 0x2e8;
    
    Serial.prototype.init = function() {
        // disable interrupts
        Kernel.outb(this.port + REG_INT_ENABLE, 0);
        
        this.setBaudRate(38400);
        this.setDataBits(8);
        this.setStopBits(1);
        this.setParity(Serial.parity.none);
        
        // enable fifo, set buffer to 14 bytes:
        Kernel.outb(this.port + REG_FIFO_CTRL, 0xc7);
        
        // enable interrupts
        Kernel.outb(this.port + REG_MODEM_CTRL, 0x0b);
    };
    
    Serial.prototype.setBaudRate = function(baudRate) {
        var divisor = 115200 / baudRate;
        // set DLAB
        var lineCtrl = Kernel.inb(this.port + REG_LINE_CTRL);
        lineCtrl |= 128;
        Kernel.outb(this.port + REG_LINE_CTRL, lineCtrl);
        // send lsb of divisor
        Kernel.outb(this.port + REG_BAUD_LSD, divisor & 0xff);
        // send msb of divisor
        Kernel.outb(this.port + REG_BAUD_MSB, (divisor >> 8) & 0xff);
        // clear DLAB
        lineCtrl &= 127;
        Kernel.outb(this.port + REG_LINE_CTRL, lineCtrl);
    };
    
    Serial.prototype.setDataBits = function(dataBits) {
        var lineCtrl = Kernel.inb(this.port + REG_LINE_CTRL);
        lineCtrl &= 0xfc;
        lineCtrl |= (dataBits - 5) & 3;
        Kernel.outb(this.port + REG_LINE_CTRL, lineCtrl);
    };
    
    Serial.prototype.setStopBits = function(stopBits) {
        if(stopBits !== 1 && stopBits !== 1.5 && stopBits !== 2) {
            throw new RangeError("stopBits must be one of 1, 1.5, 2");
        }
        var lineCtrl = Kernel.inb(this.port + REG_LINE_CTRL);
        lineCtrl &= 0xfb;
        if(stopBits !== 1) {
            lineCtrl |= 1 << 2;
        }
        Kernel.outb(this.port + REG_LINE_CTRL, lineCtrl);
    };
    
    Serial.parity = {
        none:   0,
        odd:    1,
        even:   3,
        mark:   5,
        space:  7
    };
    
    Serial.prototype.setParity = function(parity) {
        parity &= 7;
        var lineCtrl = Kernel.inb(this.port + REG_LINE_CTRL);
        lineCtrl |= parity << 3;
        Kernel.outb(this.port + REG_LINE_CTRL, lineCtrl);
    };
    
    Serial.prototype.isTransmitEmpty = function() {
        return (Kernel.inb(this.port + REG_LINE_STAT) & 0x20) > 0;
    };
    
    Serial.prototype.write = function(byte) {
        while(!this.isTransmitEmpty()) ;
        Kernel.outb(this.port, byte);
    };
    
    Serial.prototype.writeString = function(str) {
        for(var i = 0; i < str.length; i++) {
            this.write(BinaryUtils.readU8(str, i));
        }
    };
    
    Drivers.Serial = Serial;
})();