pragma Strict
import QtQml

TakeNumber {
    id: foo

    function literal0() {
        foo.takeInt(0)
        foo.propertyInt = 0;
        foo.takeNegativeInt(0)
        foo.propertyNegativeInt = 0;
        foo.takeQSizeType(0)
        foo.propertyQSizeType = 0;
        foo.takeQLongLong(0)
        foo.propertyQLongLong = 0;
        foo.takeNumbers({i:0, n:0, s:0, l:0});
        foo.propertyNumbers = {i:0, n:0, s:0, l:0};
    }

    function literal56() {
        foo.takeInt(56)
        foo.propertyInt = 56;
        foo.takeNegativeInt(-56)
        foo.propertyNegativeInt = -56;
        foo.takeQSizeType(56)
        foo.propertyQSizeType = 56;
        foo.takeQLongLong(56)
        foo.propertyQLongLong = 56;
        foo.takeNumbers({i:56, n:-56, s:56, l:56});
        foo.propertyNumbers = {i:56, n:-56, s:56, l:56};
    }

    function variable0() {
        var a = 0
        foo.takeInt(a)
        foo.propertyInt = a;
        foo.takeNegativeInt(-a)
        foo.propertyNegativeInt = -a;
        foo.takeQSizeType(a)
        foo.propertyQSizeType = a;
        foo.takeQLongLong(a)
        foo.propertyQLongLong = a;
        foo.takeNumbers({i:a, n:-a, s:a, l:a});
        foo.propertyNumbers = {i:a, n:-a, s:a, l:a};
    }

    function variable484() {
        var a = 484
        foo.takeInt(a)
        foo.propertyInt = a;
        foo.takeNegativeInt(-a)
        foo.propertyNegativeInt = -a;
        foo.takeQSizeType(a)
        foo.propertyQSizeType = a;
        foo.takeQLongLong(a)
        foo.propertyQLongLong = a;
        foo.takeNumbers({i:a, n:-a, s:a, l:a});
        foo.propertyNumbers = {i:a, n:-a, s:a, l:a};
    }

    function literal3B() {
        foo.takeInt(3000000000)
        foo.propertyInt = 3000000000;
        foo.takeNegativeInt(-3000000000)
        foo.propertyNegativeInt = -3000000000;
        foo.takeQSizeType(3000000000)
        foo.propertyQSizeType = 3000000000;
        foo.takeQLongLong(3000000000)
        foo.propertyQLongLong = 3000000000;
        foo.takeNumbers({i:3000000000, n:-3000000000, s:3000000000, l:3000000000});
        foo.propertyNumbers = {i:3000000000, n:-3000000000, s:3000000000, l:3000000000};
    }

    function variable3B() {
        var a = 3000000000
        foo.takeInt(a)
        foo.propertyInt = a;
        foo.takeNegativeInt(-a)
        foo.propertyNegativeInt = -a;
        foo.takeQSizeType(a)
        foo.propertyQSizeType = a;
        foo.takeQLongLong(a)
        foo.propertyQLongLong = a;
        foo.takeNumbers({i:a, n:-a, s:a, l:a});
        foo.propertyNumbers = {i:a, n:-a, s:a, l:a};
    }
}
