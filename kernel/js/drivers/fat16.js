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
    
    FAT16.prototype.readSectors = function(sector, count) {
        if(sector + count > this.partitionLength) {
            throw new RangeError("tried to read beyond partition. sector: " + sector + ", count: " + count);
        }
        return this.drive.readSectorsPIO(this.partitionOffset + sector, count);
    }
    
    FAT16.prototype.readCluster = function(cluster) {
        var sector = this.firstDataSector + (this.rootEntryCount / 16) + (cluster * this.sectorsPerCluster) - 2;
        return this.readSectors(sector, this.sectorsPerCluster);
    };
    
    FAT16.prototype.init = function() {        
        this.bpb = new FAT16.BPB(this.readSectors(0, 1));
        
        this.rootDirSectors = (this.bpb.rootEntryCount * 32 + 511) / 512;
        this.firstDataSector = this.bpb.reservedSectors + this.bpb.fatCount * this.bpb.sectorsPerFat;
        this.firstFatSector = this.bpb.reservedSectors;
        this.dataSectors = this.bpb.sectorCount - (this.bpb.reservedSectors + (this.bpb.fatCount * this.bpb.sectorsPerFat) + this.rootDirSectors);
        this.totalClusters = this.dataSectors / this.bpb.sectorsPerCluster;
        
        Console.write("hello!\n");
        this.fatData = this.readSectors(this.firstFatSector, this.bpb.sectorsPerFat);
    };
    
    FAT16.prototype.readRootEntries = function() {
        this.rootEntryData = this.readSectors(this.firstDataSector, this.bpb.rootEntryCount / (512/32));
        this.rootEntries = [];
        for(var i = 0; i < this.rootEntryData.length; i += 32) {
            var firstByte = BinaryUtils.readU8(this.rootEntryData, i);
            if(firstByte === 0) {
                // end of directory
                break;
            }
            if(firstByte === 0xe5) {
                // deleted file
                continue;
            }
            var entry = new FAT16.Entry(this.rootEntryData.substr(i, 32));
            if(entry.attributes & (FAT16.attributes.device | FAT16.attributes.volumeID | FAT16.attributes.unused)) {
                // windows 95 long file name
                continue;
            }
            this.rootEntries.push(entry);
        }
        return this.rootEntries;
    };
    
    FAT16.BPB = function(buffer) {        
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
    
    FAT16.Entry = function(buffer) {
        // raw attributes:
        this.fatFilename            = buffer.substr(0, 11);
        this.attributes             = BinaryUtils.readU8(buffer, 11);
        this.ntReserved             = BinaryUtils.readU8(buffer, 12);
        this.creationTenthsSecond   = BinaryUtils.readU8(buffer, 13);
        this.creationHMS            = BinaryUtils.readU16(buffer, 14);
        this.creationYMD            = BinaryUtils.readU16(buffer, 16);
        this.accessYMD              = BinaryUtils.readU16(buffer, 18);
        this.firstClusterHigh       = BinaryUtils.readU16(buffer, 20);
        this.modifiedHMS            = BinaryUtils.readU16(buffer, 22);
        this.modifiedYMD            = BinaryUtils.readU16(buffer, 24);
        this.firstClusterLow        = BinaryUtils.readU16(buffer, 26);
        this.size                   = BinaryUtils.readU32(buffer, 28);
        
        // synthesized attributes:
        this.filename       = this.fatFilename.substr(0, 8) + "." + this.fatFilename.substr(8, 3);
        this.firstCluster   = (this.firstClusterHigh << 16) | this.firstClusterLow;
    };
    
    Drivers.FAT16 = FAT16;
})();