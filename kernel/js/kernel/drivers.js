(function() {
    Drivers = {
        loadDriver: function(driverName) {
            var mod = Kernel.modules["/kernel/drivers/" + driverName + ".jmg"];
            if(typeof mod !== "string") {
                throw new Error("Could not load driver '" + driverName + "'");
            }
            Kernel.loadImage(mod);
        }
    };
})();