(function() {
    function FAT16(partition) {
        this.partition = partition;
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
        var sector = this.firstDataSector + (this.bpb.rootEntryCount / 16) + (cluster * this.bpb.sectorsPerCluster) - 2;
        return this.partition.readSectors(sector, this.bpb.sectorsPerCluster);
    };
    
    FAT16.prototype.init = function() {        
        this.bpb = new FAT16.BPB(this.partition.readSector(0));
        
        this.rootDirSectors = (this.bpb.rootEntryCount * 32 + 511) / 512;
        this.firstDataSector = this.bpb.reservedSectors + this.bpb.fatCount * this.bpb.sectorsPerFat;
        this.firstFatSector = this.bpb.reservedSectors;
        this.dataSectors = this.bpb.sectorCount - (this.bpb.reservedSectors + (this.bpb.fatCount * this.bpb.sectorsPerFat) + this.rootDirSectors);
        this.totalClusters = this.dataSectors / this.bpb.sectorsPerCluster;
        
        this.fatData = this.partition.readSectors(this.firstFatSector, this.bpb.sectorsPerFat);
    };
    
    FAT16.prototype.readRootEntries = function() {
        var data = this.partition.readSectors(this.firstDataSector, this.bpb.rootEntryCount / (512/32));
        return this.readDirectoryEntries(data);
    };
    
    FAT16.prototype.readDirectoryEntries = function(buff) {
        var entries = [];
        for(var i = 0; i < buff.length; i += 32) {
            var firstByte = BinaryUtils.readU8(buff, i);
            if(firstByte === 0) {
                // end of directory
                break;
            }
            if(firstByte === 0xe5) {
                // deleted file
                continue;
            }
            var entry = new FAT16.Entry(buff.substr(i, 32));
            if(entry.attributes & (FAT16.attributes.device | FAT16.attributes.volumeID | FAT16.attributes.unused)) {
                // windows 95 long file name
                continue;
            }
            if(entry.attributes & FAT16.attributes.directory) {
                entries.push(new FAT16.Directory(this, entry));
            } else {
                entries.push(new FAT16.File(this, entry));
            }
        }
        return entries;
    }
    
    FAT16.prototype.find = function(path) {
        if(path[0] !== "/") return null;
        var parts = path.toLowerCase().split("/");
        var entries = this.readRootEntries();
        for(var i = 1; i < parts.length; i++) {
            var found = null;
            for(var j = 0; j < entries.length; j++) {
                if(entries[j].name.toLowerCase() === parts[i]) {
                    if(i + 1 === parts.length) {
                        return entries[j];
                    } else if(entries[j] instanceof FAT16.Directory) {
                        found = entries[j];
                    } else {
                        return null;
                    }
                    break;
                }
            }
            if(!found) {
                return null;
            }
            entries = found.readEntries();
        }
        return null;
    };
    
    FAT16.prototype.readClusterChain = function(firstCluster) {
        var clusters = 0;
        var cluster = firstCluster;
        do {
            clusters++;
            cluster = BinaryUtils.readU16(this.fatData, cluster * 2);
        } while(cluster < 0xFFF7);
        
        var buff = new Buffer(clusters * this.bpb.sectorsPerCluster * 512);
        var cluster = firstCluster;
        do {
            buff.append(this.readCluster(cluster));
            cluster = BinaryUtils.readU16(this.fatData, cluster * 2);
        } while(cluster < 0xFFF7);
        
        return buff.getContents();
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
        if(buffer === undefined) {
            return;
        }
        
        this.buffer = buffer;
        
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
        this.firstCluster           = (this.firstClusterHigh << 16) | this.firstClusterLow;
        
        var filename = this.fatFilename.substr(0, 8).trimRight();
        var ext = this.fatFilename.substr(8, 3).trimRight();
        if(ext.length > 0) {
            this.filename = filename + "." + ext;
        } else {
            this.filename = filename;
        }
    };
    
    FAT16.Directory = function(fs, entry) {
        this.fs = fs;
        this.entry = entry;
        this.name = entry.filename;
    };
    
    FAT16.Directory.prototype.readEntries = function() {
        var cluster = this.fs.readClusterChain(this.entry.firstCluster);
        return this.fs.readDirectoryEntries(cluster);
    };
    
    FAT16.File = function(fs, entry) {
        this.fs = fs;
        this.entry = entry;
        this.name = entry.filename;
        this.size = entry.size;
    };
    
    FAT16.File.prototype.readAllBytes = function() {
        return this.fs.readClusterChain(this.entry.firstCluster).substr(0, this.size);
    };
    
    Drivers.FAT16 = FAT16;
})();