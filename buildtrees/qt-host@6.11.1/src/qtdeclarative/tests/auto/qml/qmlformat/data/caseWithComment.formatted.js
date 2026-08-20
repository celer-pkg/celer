function f() {
    switch (something) {
    case 0: // Hello World
        return;
    case /**/ 1 /**/ :
        return;
    case /**/ 2:
        return;
    }
}

function g() {
    switch (0) {
    case /*1*/ 0 /*2*/:
    }
    switch (0) {
    case 0:/*3*/
    }
    switch (0) {
    case 0:/*4*/
        return;
    }
    switch (0) {
    case 0:/*5*/
        return;
    }
    switch (0) {
    case 0:/*6*/
        return;
    }
    switch (0) {
    case 0://7
        return;
    }
    switch (0) {
    case 0/*8*/:/*9*/
    case 1:/*10*/
    }
}
