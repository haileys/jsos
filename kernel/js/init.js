Kernel.loadImage(Kernel.modules["/kernel/keyboard.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/keymaps.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/drivers.jmg"]);

Drivers.loadDriver("ps2kb");
var ps2kb = new Drivers.PS2Keyboard();
var keyboard = new Keyboard(ps2kb, "US");
keyboard.onKeyDown = function(key, scanCode) {
    Kernel.runGC();
    Console.write(key + ", " + scanCode + "\n");
}

Console.clear();