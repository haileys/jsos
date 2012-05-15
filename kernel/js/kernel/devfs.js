(function() {
    Kernel.devfs = {
        devices: {},
        register: function(name, file) {
            file.name       = name;
            file.size       = file.size || 0;
            file.getType    = file.getType || function() { return "file"; };
            Kernel.devfs.devices[name] = file;
        },
        unregister: function(name) {
            delete Kernel.devfs.devices[name];
        }
    };
    Kernel.filesystem.mount("/dev", {
        find: function(path) {
            if(path === "/") {
                return {
                    name: "dev",
                    getType: function() {
                        return "directory";
                    },
                    readEntries: function() {
                        var ents = [];
                        for(var k in Kernel.devfs.devices) {
                            ents.push(Kernel.devfs.devices[k]);
                        }
                        return ents;
                    }
                };
            } else {
                return Kernel.devfs.devices[path.substr(1)] || null;
            }
        }
    });
})();