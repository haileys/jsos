/*
VM.prototype.createClass = function(name, opts) {
    var fn = this.exposeFunction(opts.constructor || function() { });
    if(typeof opts.methods === "object") {
        for(var method in opts.methods) {
            fn.prototype[method] = this.exposeFunction(opts.methods[method]);
        }
    }
    if(typeof opts.classMethods === "object") {
        for(var method in opts.methods) {
            fn[method] = this.exposeFunction(opts.classMethods[method]);
        }
    }
    return fn;
};
*/

VM.prototype.safeCall = function(fn, args) {
    return fn.apply(this.globals, args);
};