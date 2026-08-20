pragma Strict
import QtQml

CategorizerBase {
    id: root

    property list<double> nnn: {
        var result = [];
        result[0] = 10;
        return result;
    }

    function sum() : list<double> {
        var numbers = root.numbers;

        var cat1Sum = 0;
        var cat2Sum = 0;
        var cat3Sum = 0;
        var huge = 0;
        for (var i = 0; i < CategorizerBase.Iterations; ++i) {
            for (var j = 0; j < CategorizerBase.Length; ++j) {
                var num = numbers[j] & CategorizerBase.Mask;
                if (num < CategorizerBase.Category0)
                    cat1Sum += num;
                else if (num < CategorizerBase.Category1)
                    cat2Sum += num;
                else if (num < CategorizerBase.Category2)
                    cat3Sum += num;
                else
                    huge += num;
            }
        }

        return [cat1Sum, cat2Sum, cat3Sum, huge];
    }
}
