(function() {
    var ADDRESS_PORT        = 0x70;
    var DATA_PORT           = 0x71;
    
    var SECONDS_REG         = 0x00;
    var MINUTES_REG         = 0x02;
    var HOURS_REG           = 0x04;
    var WEEKDAY_REG         = 0x06;
    var DAY_OF_MONTH_REG    = 0x07;
    var MONTH_REG           = 0x08;
    var YEAR_REG            = 0x09;
//    var CENTURY_REG         = 0x32;
    var STATUS_A_REG        = 0x0a;
    var STATUS_B_REG        = 0x0b;
    
    var UPDATING_FLAG       = 0x80;
    
    var rtc = {
        updateInProgress: function() {
            return (Kernel.outb(ADDRESS_PORT, STATUS_A_REG) & UPDATING_FLAG) > 0;
        },
        readRegister: function(reg) {
            Kernel.outb(ADDRESS_PORT, reg);
            return Kernel.inb(DATA_PORT);
        },
        readTime: function() {
            // wait until update in progress becomes set and cleared
            //while(!rtc.updateInProgress()) ;
            //while(rtc.updateInProgress()) ;
            
            var seconds = rtc.readRegister(SECONDS_REG);
            var minutes = rtc.readRegister(MINUTES_REG);
            var hours   = rtc.readRegister(HOURS_REG);
            var day     = rtc.readRegister(DAY_OF_MONTH_REG);
            var month   = rtc.readRegister(MONTH_REG);
            var year    = rtc.readRegister(YEAR_REG);
            
            var register_b = rtc.readRegister(STATUS_B_REG);
            
            if((register_b & 0x04) === 0) {
                // values are in bcd...
                seconds = (seconds & 0x0f) + (Math.floor(seconds / 16) * 10);
                minutes = (minutes & 0x0f) + (Math.floor(minutes / 16) * 10);
                hours   = (hours & 0x0f) + (Math.floor((hours & 0x70) / 16) * 10) | (hours & 0x80);
                day     = (day & 0x0f) + (Math.floor(day / 16) * 10);
                month   = (month & 0x0f) + (Math.floor(month / 16) * 10);
                year    = (year & 0x0f) + (Math.floor(year / 16) * 10);
            }
            
            return {
                seconds:    seconds,
                minutes:    minutes,
                hours:      hours,
                day:        day,
                month:      month,
                year:       year
            };
        }
    };
    
    Drivers.RTC = rtc;
})();