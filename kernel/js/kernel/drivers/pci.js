(function() {
    var CONFIG_ADDRESS  = 0xcf8;
    var CONFIG_DATA     = 0xcfc;
    
    function PCI() {
    }
    
    PCI.prototype.init = function() {
        Kernel.outl(CONFIG_ADDRESS, 0x80000000);
        if(Kernel.inl(CONFIG_ADDRESS) !== 0x80000000) {
            throw new Error("PCI not found");
        }
    };
    
    PCI.prototype.readRegister = function(bus, slot, func, reg) {
        var address = (bus << 16) | (slot << 11) | (func << 8) | (reg & 0xfc) | 0x80000000;
        Kernel.outl(CONFIG_ADDRESS, address);
        return (Kernel.inl(CONFIG_DATA) >> ((reg & 2) * 8)) & 0xffff;
    };
    
    PCI.prototype.writeRegister = function(bus, slot, func, reg, data) {
        var address = (bus << 16) | (slot << 11) | (func << 8) | (reg & 0xfc) | 0x80000000;
        Kernel.outl(CONFIG_ADDRESS, address);
        Kernel.outl(CONFIG_DATA, data);
    };
    
    PCI.prototype.probeDevice = function(bus, slot) {
        var vendor = this.readRegister(bus, slot, 0, 0);
        if(vendor === 0xffff) {
            return null;
        }
        var device = this.readRegister(bus, slot, 0, 2);
        var revProg = this.readRegister(bus, slot, 0, 8);
        var devClass = this.readRegister(bus, slot, 0, 10);
        return {
            vendorID:   vendor,
            deviceID:   device,
            classCode:  devClass >> 8,
            subClass:   devClass & 0xff,
            progIF:     revProg >> 8,
            revisionID: revProg & 0xff
        };
    };
    
    PCI.prototype.enumerateDevices = function() {
        var devices = [];
        for(var bus = 0; bus < 256; bus++) {
            for(var slot = 0; slot < 32; slot++) {
                var data = this.probeDevice(bus, slot);
                if(data) {
                    devices.push({ bus: bus, slot: slot, data: data });
                }
            }
        }
        return devices;
    };
    
    PCI.classCodes = {
        0x00:   "?",
        0x01:   "Mass Storage Controller",
        0x02:   "Network Controller",
        0x03:   "Display Controller",
        0x04:   "Multimedia Controller",
        0x05:   "Memory Controller",
        0x06:   "Bridge Device",
        0x07:   "Simple Communication Controllers",
        0x08:   "Base System Peripherals",
        0x09:   "Input Devices",
        0x0A:   "Docking Stations",
        0x0B:   "Processors",
        0x0C:   "Serial Bus Controllers",
        0x0D:   "Wireless Controllers",
        0x0E:   "Intelligent I/O Controllers",
        0x0F:   "Satellite Communication Controllers",
        0x10:   "Encryption/Decryption Controllers",
        0x11:   "Data Acquisition and Signal Processing Controllers",
        0xFF:   "Miscellaneous Device"
    };
    
    Drivers.PCI = PCI;
})();