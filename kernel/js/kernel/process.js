Process = (function() {
    
    var yieldQueue = new Queue();
    var pidIncrement = 1;
    
    function Process(path) {
        this._vm = new VM();
        this._running = true;
        this.setupEnvironment();
        this.id = pidIncrement++;
    }

    Process.prototype.isRunning = function() {
        return this._running;
    };
    
    Process.prototype.kill = function() {
        this._running = false;
    };

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

    Process.prototype.setupEnvironment = function() {
        var vm = this._vm, g = vm.globals, self = this;
        g.OS = vm.createObject();
        g.OS.yield = vm.exposeFunction(function(callback) {
            self.enqueueCallback(callback);
        });
        g.OS.log = vm.exposeFunction(function(msg) {
            Console.write("[#" + self.id + "] " + msg + "\n");
        });
        g.OS.pid = vm.exposeFunction(function(msg) {
            return self.id;
        });
        
        var userlib = Kernel.filesystem.find("/kernel/userlib.jmg");
        if(!(userlib instanceof Drivers.FAT16.File)) {
            throw new Process.LoadError("Could not load /kernel/userlib.jmg");
        }
        vm.execute(userlib.readAllBytes());
    };

    Process.prototype.enqueueCallback = function(callback, args) {
        yieldQueue.push({ process: this, callback: callback, args: args });
    };
    
    Process.prototype.safeCall = function(callback, args) {
        return this._vm.safeCall(callback, args || []);
    };
    
    Process.scheduleNext = function() {
        // @TODO check for processes waiting on I/O etcetera
        
        if(yieldQueue.isEmpty()) {
            return false;
        }
        var yielder = yieldQueue.pop();
        if(!yielder.process.isRunning()) {
            return Process.scheduleNext();
        }
        try {
            yielder.process.safeCall(yielder.callback, yielder.args || []);
        } catch(e) {
            // user process threw exception
            Console.write("[#" + yielder.process.id + "] Unhandled exception: " + e.toString() + ", killed.\n");
            yielder.process.kill();
        }
        return true;
    };

    Process.LoadError = function(str) {
        Error.call(this, str);
    };
    Process.LoadError.prototype = new Error();
    
    return Process;
})();