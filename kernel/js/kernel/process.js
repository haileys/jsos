Process = (function() {
    
    var yieldQueue = new Queue();
    var pidIncrement = 1;
    
    function Process(opts) {
        opts = opts || {};
        this._vm = new VM();
        this._running = true;
        this.id = pidIncrement++;
        Process.processes[this.id] = this;
        this.fds = {};
        this.parent = opts.parent || null;
        this.argv = opts.argv || [];
        this.env = {};
        this.callbacksPending = 0;
        this.waiters = [];
        this.exitStatus = null;
        if(this.parent) {
            for(var k in this.parent.env) {
                this.env[k] = this.parent.env[k];
            }
            for(var i in this.parent.fds) {
                this.fds[i] = this.parent.fds[i];
            }
        }
        this.setupEnvironment();
    }

    Process.prototype.isRunning = function() {
        return this._running;
    };
    
    Process.prototype.kill = function(exitStatus) {
        this.exitStatus = exitStatus || "";
        this._running = false;
        for(var i = 0; i < this.waiters.length; i++) {
            var waiter = this.waiters[i];
            waiter.process.enqueueCallback(waiter.callback, [this.exitStatus]);
        }
        if(this.waiters.length) {
            delete Process.processes[this.id];
        }
    };

    Process.prototype.loadImage = function(image) {
        return this._vm.execute(image);
    };

    Process.prototype.load = function(path) {
        var file = Kernel.filesystem.find(path);
        if(file === null || file.getType() !== "file") {
            throw new Process.LoadError("Could not load '" + path + "' into process");
        }
        return this.loadImage(file.readAllBytes());
    };
    
    Process.prototype.createSystemError = function(message) {
        return new this.systemErrorClass(message);
    };
    
    Process.prototype.appendFileDescriptor = function(pipe) {
        for(var fd = 3; this.fds[fd]; fd++) ;
        this.fds[fd] = pipe;
        return fd;
    };

    Process.prototype.setupEnvironment = function() {
        var vm = this._vm, g = vm.globals, self = this;
        
        function SystemError(message) {
            this.message = message;
        }
        this.systemErrorClass = vm.exposeFunction(SystemError);
        this.systemErrorClass.prototype = new g.Error();
        this.systemErrorClass.prototype.name = "SystemError";
        g.SystemError = this.systemErrorClass;
        
        g.OS = vm.createObject();
        g.OS.stdin = 0;
        g.OS.stdout = 1;
        g.OS.stderr = 2;
        
        function expectType(name, val) {
            for(var i = 2; i < arguments.length; i++) {
                if(typeof val === arguments[i]) {
                    return;
                }
            }
            throw self.createSystemError("expected '" + name + "' to be of type " + arguments.slice(2).join(" or "));
        }
        
        // Yields control to another process
        // 
        // callback: A function the operating system should return control to
        g.OS.yield = vm.exposeFunction(function(callback) {
            expectType("callback", callback, "function");
            self.enqueueCallback(callback);
        });
        g.OS.argv = vm.createArray();
        for(var i = 0; i < this.argv.length; i++) {
            g.OS.argv[i] = this.argv[i];
        }
        
        // Logs a message to the text console along with the process's ID
        // 
        // msg: The message to log
        g.OS.log = vm.exposeFunction(function(msg) {
            Console.write("[#" + self.id + "] " + msg + "\n");
        });
        
        // Returns the current process's ID
        g.OS.pid = vm.exposeFunction(function() {
            return self.id;
        });
        
        // Returns the process ID of the current process's parent process
        g.OS.parentPid = vm.exposeFunction(function() {
            if(self.parent) {
                return self.parent.id;
            } else {
                return null;
            }
        });
        
        // Reads at most `size` bytes from the file identified by `fd`
        // 
        // fd:          The file descriptor to be read from
        // size:        The maximum number of bytes to be read
        // callback:    The function to be called upon completion of the read.
        //              The callback will be passed two arguments: `error` and `buff`
        g.OS.read = vm.exposeFunction(function(fd, size, callback) {
            if(typeof fd !== "number" || typeof size !== "number" || typeof callback !== "function") {
                throw self.createSystemError("expected 'fd' to be a number, 'size' to be a number and 'callback' to be a function");
            }
            if(!self.fds[fd]) {
                throw self.createSystemError("bad file descriptor");
            }
            self.callbacksPending++;
            self.fds[fd].read(size, function(err, data) {
                self.enqueueCallback(callback, [err, data]);
            });
        });
        
        // Writes data to an open file
        // 
        // fd:      The file descriptor to be written to
        // data:    The data to write
        g.OS.write = vm.exposeFunction(function(fd, data) {
            if(typeof fd !== "number" || typeof data !== "string") {
                throw self.createSystemError("expected 'fd' to be a number and 'data' to be a string");
            }
            if(!self.fds[fd]) {
                throw self.createSystemError("bad file descriptor");
            }
            self.fds[fd].write(data);
        });
        
        // Spawns a child process and returns and object containing the child
        // ID (`pid`)
        // 
        // image: The image as a binary buffer to load and execute in the new process
        g.OS.spawnChild = vm.exposeFunction(function(image) {
            if(typeof image !== "string") {
                throw self.createSystemError("expected 'image' to be a string");
            }
            var argv = [];
            for(var i = 1; i < arguments.length; i++) {
                argv.push(arguments[i]);
            }
            var child = new Process({ parent: self, argv: argv });
            child.enqueueCallback(function() {
                child.loadImage(image);
            }, []);
            return {
                pid: child.id
            };
        });
        
        // Loads and executes an image in the current process
        // 
        // image: The image as a binary buffer
        g.OS.loadImage = vm.exposeFunction(function(image) {
            if(typeof image !== "string") {
                throw self.createSystemError("expected 'image' to be a string");
            }
            return self.loadImage(image);
        });
        
        // Calls a device specific method on an open file
        // 
        // fd:      The file descriptor to call the method on
        // method:  The method to call. This is a device-specific string
        // args:    An argument to pass to the method. May be undefined.
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
            return self.fds[fd].ioctl[method](self, self.fds[fd], args);
        });
        
        // Reads all entries from a directory. Each entry is represented as an
        // object with a `name` property and `type` property.
        // 
        // path:        The directory to list
        // callback:    The function to be called upon completion of the operation.
        //              The callback will be passed two arguments: `error` and `entries`
        g.OS.readDirectory = vm.exposeFunction(function(path, callback) {
            if(typeof path !== "string") {
                throw self.createSystemError("expected 'path' to be a string");
            }
            if(typeof callback !== "function") {
                throw self.createSystemError("expected 'callback' to be a function");
            }
            var dir = Kernel.filesystem.find(path);
            self.callbacksPending++;
            if(dir === null) {
                self.enqueueCallback(callback, ["'" + path + "' not found"]);
            }
            if(dir.getType() !== "directory") {
                self.enqueueCallback(callback, ["'" + path + "' is not a directory"]);
            }
            var entries = dir.readEntries();
            var arr = vm.createArray();
            for(var i = 0; i < entries.length; i++) {
                var obj = vm.createObject();
                obj.name = entries[i].name;
                obj.type = entries[i].getType();
                arr.push(obj);
            }
            self.enqueueCallback(callback, [false, arr]);
        });
        
        // Opens a file
        // 
        // path:        The absolute path to the file to be opened
        // callback:    The function to be called upon completion of the operation.
        //              The callback will be passed two arguments: `error` and `fd`
        g.OS.open = vm.exposeFunction(function(path, callback) {
            if(typeof path !== "string") {
                throw self.createSystemError("expected 'path' to be a string");
            }
            if(typeof callback !== "function") {
                throw self.createSystemError("expected 'callback' to be a function");
            }
            var file = Kernel.filesystem.find(path);
            self.callbacksPending++;
            if(file === null) {
                self.enqueueCallback(callback, ["'" + path + "' not found"]);
                return;
            }
            if(file.getType() !== "file") {
                self.enqueueCallback(callback, ["'" + path + "' is not a file"]);
                return;
            }
            self.enqueueCallback(callback, [false, self.appendFileDescriptor(new Filesystem.FileDescriptor(file))]);
        });
        
        // Closes an open file
        // 
        // fd: The file descriptor to close
        g.OS.close = vm.exposeFunction(function(fd) {
            if(typeof fd !== "number") {
                throw self.createSystemError("expected 'fd' to be a number");
            }
            if(!self.fds[fd]) {
                throw self.createSystemError("invalid fd");
            }
            // @TODO
            //self.fds[fd].close();
            delete self.fds[fd];
        });
        
        // Returns an object with metadata about a file. The object will have
        // the properties `size` (the size of the file in bytes), `name` (the
        // name of the file), `type` (one of "file" or "directory"), and `path`
        // (the absolute path to the file)
        // 
        // path:        The absolute path to the file
        // callback:    The function to be called upon completion of the operation.
        //              The callback will be passed two parameters: `error` and `stat`
        g.OS.stat = vm.exposeFunction(function(path, callback) {
            if(typeof path !== "string") {
                throw self.createSystemError("expected 'path' to be a string");
            }
            if(typeof callback !== "function") {
                throw self.createSystemError("expected 'callback' to be a function");
            }
            var file = Kernel.filesystem.find(path);
            self.callbacksPending++;
            if(file === null) {
                self.enqueueCallback(callback, ["'" + path + "' not found"]);
                return;
            }
            if(file.getType() === "file") {
                var stat = vm.createObject();
                stat.size = file.size;
                stat.name = file.name;
                stat.type = "file";
                stat.path = path;
                self.enqueueCallback(callback, [false, stat]);
            } else if(file.getType() === "directory") {
                var stat = vm.createObject();
                stat.name = file.name;
                stat.type = "directory";
                stat.path = path;
                self.enqueueCallback(callback, [false, stat]);
            } else {
                self.enqueueCallback(callback, ["'" + path + "' is of unknown type"]);
            }
        });
        
        // Gets or sets an environment variable for the current process
        // 
        // name:    The name of the variable to get/set
        // value:   Optional. If omitted, `OS.env` will return the current
        //          value of the environment variable `name`. If set, the
        //          environment variable will be set to this value
        g.OS.env = vm.exposeFunction(function(name, value) {
            if(typeof name !== "string") {
                throw self.createSystemError("expected 'name' to be a string");
            }
            if(typeof value !== "undefined" && typeof value !== "string") {
                throw self.createSystemError("expected 'value' to be a string");
            }
            if(typeof value === "undefined") {
                return self.env[name];
            } else {
                return self.env[name] = value;
            }
        });
        
        // Waits for a process to terminate
        // 
        // pid:         The ID of the process to wait on
        // callback:    The function to call when the process identified by
        //              `pid` exits. This function will be passed the exit status
        //              of the process as its only parameter
        g.OS.wait = vm.exposeFunction(function(pid, callback) {
            if(typeof pid !== "number") {
                throw self.createSystemError("expected 'pid' to be a number");
            }
            if(typeof callback !== "function") {
                throw self.createSystemError("expected 'callback' to be a function");
            }
            if(self.id === pid) {
                throw self.createSystemError("cannot wait on self");
            }
            var proc = Process.processes[pid];
            if(!proc) {
                throw self.createSystemError("no such process: " + pid);
            }
            self.callbacksPending++;
            if(proc.isRunning()) {
                proc.waiters.push({ process: self, callback: callback });
            } else {
                self.enqueueCallback(callback, [proc.exitStatus]);
                delete Process.processes[pid];
            }
        });
        
        // Terminates the current process. This system call will return, but 
        // no more callbacks will be scheduled by the system for this process
        g.OS.exit = vm.exposeFunction(function() {
            self.kill();
        });
        
        // Duplicates an open file descriptor. This creates an alias for the
        // same open file on a different descriptor number.
        // 
        // src:     The existing file descriptor to duplicate
        // dest:    Optional. If set, this will be the descriptor number for
        //          alias. This will overwrite any existing file descriptor on
        //          this number. If omitted, a descriptor number will be picked
        //          by the OS.
        // returns: The descriptor number of the alias
        g.OS.dup = vm.exposeFunction(function(src, dest) {
            if(typeof src !== "number") {
                throw self.createSystemError("expected 'src' to be a number");
            }
            if(!self.fds[src]) {
                throw self.createSystemError("bad file descriptor");
            }
            if(typeof dest === "undefined") {
                return self.appendFileDescriptor(self.fds[src]);
            }
            if(typeof dest !== "number") {
                throw self.createSystemError("expected 'dest' to be a number");
            }
            self.fds[dest] = self.fds[src];
            return dest;
        });
        
        // Creates a new bi-directional pipe
        // 
        // returns: The file descriptor the pipe is open on
        g.OS.pipe = vm.exposeFunction(function() {
            return self.appendFileDescriptor(new Pipe());
        });
    };

    Process.prototype.enqueueCallback = function(callback, args) {
        yieldQueue.push({ process: this, callback: callback, args: args });
    };
    
    Process.prototype.safeCall = function(callback, args) {
        return this._vm.safeCall(callback, args || []);
    };
    
    Process.processes = {};
    
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
            return;
        }
        if(--yielder.process.callbacksPending === 0) {
            //Console.write("yielder.process.callbacksPending == 0 for PID " + yielder.process.id + "\n");
            //yielder.process.kill();
        }
        return true;
    };

    Process.LoadError = function(str) {
        Error.call(this, str);
    }
    Process.LoadError.prototype = new Error();
    Process.LoadError.prototype.name = "LoadError";
    
    return Process;
})();