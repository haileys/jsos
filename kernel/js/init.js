console.log("Hello world from JavaScript!");

Kernel.loadImage(Kernel.modules["/kernel/keyboard.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/drivers.jmg"]);
Drivers.loadDriver("ps2");
Keyboard.addDriver(new Drivers.PS2());

