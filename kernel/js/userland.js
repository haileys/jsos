(function cb() {
    log("--> " + processName + "\n");
    yield(cb);
})();