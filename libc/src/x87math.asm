global sin
global cos
global sqrt

sin:
    fld qword [esp+4]
    fsin
    ret

cos:
    fld qword [esp+4]
    fcos
    ret

sqrt:
    fld qword [esp+4]
    fsqrt
    ret