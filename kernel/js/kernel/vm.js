VM.prototype.safeCall = function(fn, args) {
    return fn.apply(this.globals, args);
};