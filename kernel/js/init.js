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

Console.write("VM is a: " + typeof VM + "\n");
Console.write("VM.self() is a: " + typeof VM.self() + "\n");
Console.write("My VM id is: " + VM.self().id() + "\n\n");

var vm = new VM(function() {
    Console.write("Hello world from a new VM!\n");
    Console.write("My VM id is: " + VM.self().id() + "\n\n");
});

Console.write("Back to the kernel VM, with VM id: " + VM.self().id());

