(function cb() {
    log(processName);
    yield(cb);
})();

