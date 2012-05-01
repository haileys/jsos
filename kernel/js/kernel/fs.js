(function() {
    function Filesystem() {
        this.mountpoints = [];
    };
    
    Filesystem.prototype.mount = function(path, fs) {
        path = new Path(path);
        if(typeof fs.init === "function") {
            fs.init();
        }
        this.mountpoints.push({ path: path, fs: fs });
    };
    
    Filesystem.prototype.unmount = function(path) {
        var mountpoint = this.findMountpoint(path);
        for(var i = 0; i < this.mountpoints.length; i++) {
            if(this.mountpoints[i] === mountpoint) {
                if(typeof mountpoint.fs.close === "function") {
                    mountpoint.fs.close();
                }
                this.mountpoints.splice(i, 1);
                return true;
            }
        }
        return false;
    };
    
    Filesystem.prototype.read = function(path) {
        var file = this.find(path);
        if(file === null || file.getType() !== "file") {
            return null;
        }
        return file.readAllBytes();
    };
    
    Filesystem.prototype.find = function(path) {
        path = new Path(path);
        var mountpoint = this.findMountpoint(path);
        if(mountpoint === null) {
            return null;
        }
        return mountpoint.fs.find(path.removePrefix(mountpoint.path).toString());
    };
    
    Filesystem.prototype.findMountpoint = function(path) {
        path = new Path(path);
        var bestMatch = null;
        for(var i = 0; i < this.mountpoints.length; i++) {
            if(bestMatch === null || this.mountpoints[i].path.length > bestMatch.path.length) {
                if(this.mountpoints[i].path.includes(path)) {
                    bestMatch = this.mountpoints[i];
                }
            }
        }
        return bestMatch;
    };
    
    Filesystem.FileDescriptor = function(file) {
        this.file = file;
        this.offset = 0;
    }
    Filesystem.FileDescriptor.prototype.ioctl = {};
    Filesystem.FileDescriptor.prototype.read = function() {
        
    };
    
    function Path(path) {
        if(path instanceof Path) {
            return path;
        }
        if(typeof path !== "string") {
            throw new TypeError("path parameter must be a string or Path");
        }
        if(path[path.length - 1] === "/") {
            path = path.substr(0, path.length - 1);
        }
        var parts = path.split("/");
        if(parts[0] && parts[0].length !== 0) {
            throw new Error("path provided is not absolute");
        }
        this.parts = parts.slice(1);
        this.length = this.parts.length;
    }
    
    Path.prototype.toString = function() {
        return "/" + this.parts.join("/");
    };
    
    Path.prototype.includes = function(path) {
        path = new Path(path);
        for(var i = 0; i < this.length; i++) {
            if(this.parts[i] !== path.parts[i]) {
                return false;
            }
        }
        return true;
    };
    
    Path.prototype.equals = function(path) {
        path = new Path(path);
        if(this.length !== path.length) {
            return false;
        }
        return this.includes(path);
    };
    
    Path.prototype.removePrefix = function(prefix) {
        prefix = new Path(prefix);
        if(!prefix.includes(this)) {
            return null;
        }
        var parts = this.parts.slice(prefix.length);
        return new Path("/" + parts.join("/"));
    };
    
    this.Filesystem = Filesystem;
    this.Path = Path;
}).call(this);