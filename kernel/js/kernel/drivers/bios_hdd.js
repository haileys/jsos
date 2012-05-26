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
        Kernel.poke8(0xa000 + 0, 16 /* packet size */);
        Kernel.poke8(0xa000 + 1, 0 /* reserved */);
        Kernel.poke16(0xa000 + 2, 1 /* number of sectors */);
        Kernel.poke32(0xa000 + 4, 0xb000 /* read buffer address */);
        Kernel.poke32(0xa000 + 8, lba);
        Kernel.poke32(0xa000 + 12, 0 /* upper part of 48 bit lba - ignore */);
        
        var code =  /* mov ah, 0x42 */   "\xB4\x42" +
                    /* mov si, 0xa000 */ "\xBE\x00\xa0" +
                    /* mov bx, 0x55aa */ "\xBB\xAA\x55" +
                    /* mov dl, 0x80 */   "\xB2" + String.fromCharCode(this.driveNumber) +
                    /* int 0x13 */       "\xCD\x13" +
                    /* ret */            "\xC3";
        if(code.length !== 13) {
            Kernel.panic("In BiosHDD.prototype.readSector, code.length !== 13");
        }
        Kernel.memset(0xb000, 0, 512);
        Kernel.realExec(code);
        return Kernel.readMemory(0xb000, 512);
    };
    
    BiosHDD.prototype.writeSector = function() {
        throw new Error("not implemented!");
    };
    
    Drivers.BiosHDD = BiosHDD;
})();