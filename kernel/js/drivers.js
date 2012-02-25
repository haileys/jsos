(function() {
    Drivers = {
        loadDriver: function(driverName) {
            Kernel.loadImage(Kernel.modules["/kernel/drivers/" + driverName + ".jmg"]);
        }
    };
})();