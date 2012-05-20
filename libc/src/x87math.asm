global sin
global cos
global tan
global sqrt

sin:
    fld qword [esp+4]
    fsin
    ret

cos:
    fld qword [esp+4]
    fcos
    ret

tan:
    fld qword [esp+4]
    fsincos
    fdivp
    ret

sqrt:
    fld qword [esp+4]
    fsqrt
    ret