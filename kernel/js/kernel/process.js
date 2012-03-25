Process = (function() {
    
    var yieldQueue = new Queue();
    var pidIncrement = 1;
    
    function Process(opts) {
        opts = opts || {};
        this._vm = new VM();
        this._running = true;
        this.setupEnvironment();
        this.id = pidIncrement++;
        this.fds = [new Pipe(), new Pipe(), new Pipe()];
        this.parent = opts.parent || null;
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
    
    Process.prototype.createSystemError = function(message) {
        return new this.systemErrorClass(message);
    };
    
    Process.prototype.appendFileDescriptor = function(pipe) {
        var fd = this.fds.length;
        this.fds[fd] = pipe;
        return fd;
    };

    Process.prototype.setupEnvironment = function() {
        var vm = this._vm, g = vm.globals, self = this;
        
        this.systemErrorClass = vm.exposeFunction(function(message) {
            this.message = message;
        });
        this.systemErrorClass.prototype = new g.Error();
        g.SystemError = this.systemErrorClass;
        
        g.OS = vm.createObject();
        g.OS.stdin = 0;
        g.OS.stdout = 1;
        g.OS.stderr = 2;
        g.OS.yield = vm.exposeFunction(function(callback) {
            self.enqueueCallback(callback);
        });
        g.OS.log = vm.exposeFunction(function(msg) {
            Console.write("[#" + self.id + "] " + msg + "\n");
        });
        g.OS.pid = vm.exposeFunction(function() {
            return self.id;
        });
        g.OS.parentPid = vm.exposeFunction(function() {
            if(self.parent) {
                return self.parent.id;
            } else {
                return null;
            }
        });
        g.OS.read = vm.exposeFunction(function(fd, callback) {
            if(typeof fd !== "number" || typeof callback !== "function") {
                throw self.createSystemError("expected 'fd' to be a number and 'callback' to be a function");
            }
            if(!self.fds[fd]) {
                throw self.createSystemError("bad file descriptor");
            }
            self.fds[fd].read(function(err, data) {
                self.enqueueCallback(callback, [err, data]);
            });
        });
        g.OS.write = vm.exposeFunction(function(fd, data) {
            if(typeof fd !== "number" || typeof data !== "string") {
                throw self.createSystemError("expected 'fd' to be a number and 'data' to be a string");
            }
            if(!self.fds[fd]) {
                throw self.createSystemError("bad file descriptor");
            }
            self.fds[fd].write(data);
        });
        g.OS.spawnChild = vm.exposeFunction(function(callback) {
            if(typeof callback !== "function" && typeof callback !== "undefined") {
                throw self.createSystemError("expected 'callback' to be either a function or undefined");
            }
            var child = new Process({ parent: self });
            child.enqueueCallback(callback, []);
            return {
                pid: child.id,
                stdin: self.appendFileDescriptor(child.fds[0]),
                stdout: self.appendFileDescriptor(child.fds[1]),
                stderr: self.appendFileDescriptor(child.fds[2]),
            };
        });
        g.OS.ioctl = vm.exposeFunction(function(fd, method, args) {
            if(typeof fd !== "number" || typeof method !== "string") {
                throw self.createSystemError("expected 'fd' to be a number and 'method' to be a string");
            }
            if(!self.fds[fd]) {
                throw self.createSystemError("bad file descriptor");
            }
            if(typeof self.fds[fd].ioctl !== "object" || !self.fds[fd].ioctl.hasOwnProperty(method)) {
                throw self.createSystemError("file descriptor does not support this operation");
            }
            return self.fds[fd].ioctl[method](self, args);
        });
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
            Console.write("[#" + yielder.process.id + "] Unhandled exception: " + String(e) + ", killed.\n");
            if(typeof e === "object" && e.stack) {
                Console.write(e.stack + "\n");
            }
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