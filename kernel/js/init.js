console.log("Hello world from JavaScript!");
console.log("My bytecode is " + Kernel.modules["/kernel/init.jmg"].length + " bytes long");

var tick = 0;
Kernel.isrs[32] = function() {
    console.log("tick " + tick++);
};