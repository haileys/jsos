(function() {
    function BiosHDD(biosDriveNumber) {
        this.driveNumber = biosDriveNumber;
    }
    
    BiosHDD.prototype.readSectors = function(lba, count) {
        var buff = new Buffer(count * 512);
        for(var i = 0; i < count; i++) {
            buff.append(this.readSector(lba + i));
        }
        return buff.getContents();
    }
    
    BiosHDD.prototype.readSector = function(lba) {
        if(typeof lba !== "number") {
            throw new TypeError("expected lba to be a number");
        }
        // construct LBA packet
        Kernel.poke8(0x500 + 0, 16 /* packet size */);
        Kernel.poke8(0x500 + 1, 0 /* reserved */);
        Kernel.poke16(0x500 + 2, 1 /* number of sectors */);
        Kernel.poke32(0x500 + 4, 0x1000 /* read buffer address */);
        Kernel.poke32(0x500 + 8, lba);
        Kernel.poke32(0x500 + 12, 0 /* upper part of 48 bit lba - ignore */);
        
        var code =  /* mov ah, 0x42 */   "\xB4\x42" +
                    /* mov si, 0x500 */  "\xBE\x00\x05" +
                    /* mov bx, 0x55aa */ "\xBB\xAA\x55" +
                    /* mov dl, 0x80 */   "\xB2" + String.fromCharCode(this.driveNumber) +
                    /* int 0x13 */       "\xCD\x13" +
                    /* ret */            "\xC3";
        Kernel.realExec(code);
        
        return Kernel.readMemory(0x1000, 512);
    };
    
    BiosHDD.prototype.writeSector = function() {
        throw new Error("not implemented!");
    };
    
    Drivers.BiosHDD = BiosHDD;
})();