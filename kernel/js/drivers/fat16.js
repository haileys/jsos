(function() {
    function FAT16(drive, partitionOffset, partitionLength) {
        this.drive = drive;
        this.partitionOffset = Number(partitionOffset);
        this.partitionLength = Number(partitionLength);
    }
    
    FAT16.attributes = {
        readOnly:   0x01,
        hidden:     0x02,
        system:     0x04,
        volumeID:   0x08,
        directory:  0x10,
        archive:    0x20,
        device:     0x40,
    };
    
    FAT16.prototype.readCluster = function(cluster) {
        var sector = this.firstDataSector + (this.rootEntryCount / 16) + (cluster * this.sectorsPerCluster) - 2;
        this.drive.readSectorsPIO(sector + this.partitionOffset, this.sectorsPerCluster);
    };
    
    FAT16.prototype.init = function() {
        this.bpb = new FAT16.BPB(this.drive.readSectorPIO(this.partitionOffset));
    };
    
    FAT16.BPB = function(buffer) {
        this._buffer = buffer;
        
        // bpb:
        this.jmpShort           = buffer.substr(0, 3);
        this.oem                = buffer.substr(3, 8);
        this.bytesPerSector     = BinaryUtils.readU16(buffer, 11);
        this.sectorsPerCluster  = BinaryUtils.readU8(buffer, 13);
        this.reservedSectors    = BinaryUtils.readU16(buffer, 14);
        this.fatCount           = BinaryUtils.readU8(buffer, 16);
        this.rootEntryCount     = BinaryUtils.readU16(buffer, 17);
        this.sectorCount        = BinaryUtils.readU16(buffer, 19);
        this.mediaDescriptor    = BinaryUtils.readU8(buffer, 21);
        this.sectorsPerFat      = BinaryUtils.readU16(buffer, 22);
        this.sectorsPerTrack    = BinaryUtils.readU16(buffer, 24);
        this.heads              = BinaryUtils.readU16(buffer, 26);
        this.hiddenSectorCount  = BinaryUtils.readU32(buffer, 28);
        this.longSectorCount    = BinaryUtils.readU32(buffer, 32);
        
        // ebpb:
        this.driveNumber        = BinaryUtils.readU8(buffer, 36);
        this.ntFlags            = BinaryUtils.readU8(buffer, 37);
        this.signature          = BinaryUtils.readU8(buffer, 38);
        this.serial             = BinaryUtils.readU32(buffer, 39);
        this.label              = buffer.substr(43, 11);
        this.sysIdent           = buffer.substr(54, 8);
    };
    
    
    Drivers.FAT16 = FAT16;
})();