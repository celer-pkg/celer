import QtQml

QtObject {
    id: d

    function dummy() : var {
        let reg = /href="(.*?)"/g;
        return reg.exec("foos");
    }
}
