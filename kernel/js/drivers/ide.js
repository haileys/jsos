(function() {
    var ATA_REG_DATA       = 0x00;
    var ATA_REG_ERROR      = 0x01;
    var ATA_REG_FEATURES   = 0x01;
    var ATA_REG_SECCOUNT0  = 0x02;
    var ATA_REG_LBA0       = 0x03;
    var ATA_REG_LBA1       = 0x04;
    var ATA_REG_LBA2       = 0x05;
    var ATA_REG_HDDEVSEL   = 0x06;
    var ATA_REG_COMMAND    = 0x07;
    var ATA_REG_STATUS     = 0x07;
    var ATA_REG_SECCOUNT1  = 0x08;
    var ATA_REG_LBA3       = 0x09;
    var ATA_REG_LBA4       = 0x0A;
    var ATA_REG_LBA5       = 0x0B;
    var ATA_REG_CONTROL    = 0x0C;
    var ATA_REG_ALTSTATUS  = 0x0C;
    var ATA_REG_DEVADDRESS = 0x0D;
    
    var ATA_CMD_READ_PIO        = 0x20;
    var ATA_CMD_READ_PIO_EXT    = 0x24;
    var ATA_CMD_READ_DMA        = 0xC8;
    var ATA_CMD_READ_DMA_EXT    = 0x25;
    var ATA_CMD_WRITE_PIO       = 0x30;
    var ATA_CMD_WRITE_PIO_EXT   = 0x34;
    var ATA_CMD_WRITE_DMA       = 0xCA;
    var ATA_CMD_WRITE_DMA_EXT   = 0x35;
    var ATA_CMD_CACHE_FLUSH     = 0xE7;
    var ATA_CMD_CACHE_FLUSH_EXT = 0xEA;
    var ATA_CMD_PACKET          = 0xA0;
    var ATA_CMD_IDENTIFY_PACKET = 0xA1;
    var ATA_CMD_IDENTIFY        = 0xEC;
    
    function IDE(bus, slave) {
        if(typeof bus !== "number") {
            throw new TypeError("expected bus to be a number");
        }
        if(typeof slave !== "number") {
            throw new TypeError("expected slave to be a number");
        }
        this.bus = bus;
        this.slave = slave;
    }
    
    IDE.MASTER = 0;
    IDE.SLAVE = 1;
    
    IDE.prototype.readSectorPIO = function(lba) {
        if(typeof lba !== "number") {
            throw new TypeError("expected lba to be a number");
        }
        
        Kernel.outb(this.bus + ATA_REG_FEATURES, 0x00);
        Kernel.outb(this.bus + ATA_REG_SECCOUNT0, 1);
        Kernel.outb(this.bus + ATA_REG_HDDEVSEL, 0xe0 | (this.slave << 4) | ((lba & 0x0f000000) >> 24));
        Kernel.outb(this.bus + ATA_REG_LBA0, (lba & 0x000000ff) >> 0);
        Kernel.outb(this.bus + ATA_REG_LBA1, (lba & 0x0000ff00) >> 1);
        Kernel.outb(this.bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 2);
        Kernel.outb(this.bus + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
        
        while(Kernel.inb(this.bus + ATA_REG_STATUS) & 0x80) ; // busy wait
        return Kernel.insw(this.bus, 256);
    };
    
    IDE.prototype.readSectorsPIO = function(lba, count) {
        var buff = "";
        for(var i = 0; i < lba; i++) {
            buff += this.readSectorPIO(lba + i);
        }
        return buff;
    };
    
    IDE.prototype.writeSectorPIO = function(lba, buffer) {
        if(typeof lba !== "number") {
            throw new TypeError("expected lba to be a number");
        }
        if(typeof buffer !== "string") {
            throw new TypeError("expected buffer to be a string");
        }
        
        Kernel.outb(this.bus + ATA_REG_FEATURES, 0x00);
        Kernel.outb(this.bus + ATA_REG_SECCOUNT0, 1);
        Kernel.outb(this.bus + ATA_REG_HDDEVSEL, 0xe0 | (this.slave << 4) | ((lba & 0x0f000000) >> 24));
        Kernel.outb(this.bus + ATA_REG_LBA0, (lba & 0x000000ff) >> 0);
        Kernel.outb(this.bus + ATA_REG_LBA1, (lba & 0x0000ff00) >> 1);
        Kernel.outb(this.bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 2);
        Kernel.outb(this.bus + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
        
        while(Kernel.inb(this.bus + ATA_REG_STATUS) & 0x80) ; // busy wait
        Kernel.outsw(this.bus, buffer, 256);
        Kernel.outb(this.buf + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    };
    
    Drivers.IDE = IDE;
})();