(function cb() {
    OS.log(processName);
    OS.yield(cb);
})();