(function() {
    function MBR(drive) {
        this.drive = drive;
        this.partitions = [];
        var mbr = drive.readSector(0);
        for(var i = 0; i < 4; i++) {
            this.partitions.push(new MBR.Partition(drive, mbr.substr(0x1be + i * 16, 16)));
        }
    }
    
    MBR.systemIDs = {
        0x00: "Empty",
        0x01: "FAT12",
        0x04: "FAT16",
        // listed as 'Hidden DOS 16-bit FAT <32M'
        // seems to be in use for FAT16 though...
        0x0e: "FAT16",
    };
    
    MBR.Partition = function(drive, buffer) {
        this.drive = drive;
        
        this.bootable       = BinaryUtils.readU8(buffer, 0) === 0x80;
        this.startHead      = BinaryUtils.readU8(buffer, 1);
        var sectorCylinder  = BinaryUtils.readU16(buffer, 2);
        this.startSector    = sectorCylinder & 63;
        this.startCylinder  = sectorCylinder >> 6;
        this.systemID       = BinaryUtils.readU8(buffer, 4);
        this.endHead        = BinaryUtils.readU8(buffer, 5);
        sectorCylinder      = BinaryUtils.readU16(buffer, 6);
        this.endSector      = sectorCylinder & 63;
        this.endCylinder    = sectorCylinder >> 6;
        this.relativeSector = BinaryUtils.readU32(buffer, 8);
        this.totalSectors   = BinaryUtils.readU32(buffer, 12);
    };
    MBR.Partition.prototype.readSector = function(sector) {
        return this.drive.readSector(sector + this.relativeSector);
    };
    MBR.Partition.prototype.readSectors = function(sector, count) {
        return this.drive.readSectors(sector + this.relativeSector, count);
    };
    MBR.Partition.prototype.writeSector = function(sector, buff) {
        return this.drive.writeSector(sector + this.relativeSector, buff);
    };
    
    Drivers.MBR = MBR;
})();