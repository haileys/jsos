function Process(path) {
    this._vm = new VM();
    var userlib = Kernel.filesystem.find("/kernel/userlib.jmg");
    if(!(userlib instanceof Drivers.FAT16.File)) {
        throw new Process.LoadError("Could not load /kernel/userlib.jmg");
    }
    this._vm.execute(userlib.readAllBytes());
    if(path) {
        this.load(path);
    }
}

Process.prototype.loadImage = function(image) {
    this._vm.execute(image);
};

Process.prototype.load = function(path) {
    var file = Kernel.filesystem.find(path);
    if(!(file instanceof Drivers.FAT16.File)) {
        throw new Process.LoadError("Could not load '" + path + "' into process");
    }
    this._vm.execute(file.readAllBytes());
};

Process.LoadError = function(str) {
    Error.call(this, str);
};
Process.LoadError.prototype = new Error();

Process.yieldQueue = new Queue();

Process.yield = function(callback) {
    Process.yieldQueue.push(callback);
};

Process.tick = function() {
    if(Process.yieldQueue.isEmpty()) {
        return;
    }
    var callback = Process.yieldQueue.pop();
    callback();
};