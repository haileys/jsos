console.log("Hello world from JavaScript!");
console.log("My bytecode is " + Kernel.modules["/kernel/init.jmg"].length + " bytes long");

Kernel.isrs[32] = function() {
    console.log("hello!");
};