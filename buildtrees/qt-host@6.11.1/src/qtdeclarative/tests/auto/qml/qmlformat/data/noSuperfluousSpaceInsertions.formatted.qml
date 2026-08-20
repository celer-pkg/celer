pragma /**/ ComponentBehavior /**/ : /**/ Bound

import /**/ QtQml /**/ as /**/ QQml /**/
import /**/ "hello_world"

/**/ QtObject {
    function /**/ f/**/(/**/ a /**/ : /**/ int /**/ ) /**/ : /**/ int /**/ {
        let /**/ a = /**/ 1; /**/
        return /**/ ~ /**/ a /**/ [c /**/ ] /**/ << /**/ 1; /**/
    }

    /**/
    readonly /**/ property /**/ int /**/ i /**/ : 9
    Component.onCompled /**/ : /**/ console.log /**/ ()

    signal /**/ s
}
/**/
