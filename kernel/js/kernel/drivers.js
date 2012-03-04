(function() {
    var driverLoader = function(name) {
        return Kernel.modules[name];
    };
    
    Drivers = {
        loadDriver: function(driverName) {
            var mod = driverLoader("/kernel/drivers/" + driverName + ".jmg");
            if(typeof mod !== "string") {
                throw new Error("Could not load driver '" + driverName + "'");
            }
            Kernel.loadImage(mod);
        },
        setLoader: function(loader) {
            driverLoader = loader;
        }
    };
})();