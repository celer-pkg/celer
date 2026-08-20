function testReturn() {
    if (1) {
        return
        42
    } else {
        return 42
    }
}

loop1: for (let i = 0; i < 3; ++i) {
    break
    loop1
}

outer: while (true) {
    continue
    outer
}
