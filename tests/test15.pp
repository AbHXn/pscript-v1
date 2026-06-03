? dataset ?
pidi dataset kootam = {12,5,8,21,3,7,15,2,18};

para "Dataset:" koode "\n";
para dataset koode "\n";


? find maximum value ?
pindi findMax(pidi arr kootam){

    pidi i = 1;
    pidi max = arr[0];

    ittuthiri i < arr:valupam {

        nok arr[i] > max {
            max = arr[i];
        };

        i = i + 1;
    }

    poda max;
}


? find minimum value ?
pindi findMin(pidi arr kootam){

    pidi i = 1;
    pidi min = arr[0];

    ittuthiri i < arr:valupam {

        nok arr[i] < min {
            min = arr[i];
        };

        i = i + 1;
    }

    poda min;
}


? compute average ?
pindi average(pidi arr kootam){

    pidi i = 0;
    pidi total = 0;

    ittuthiri i < arr:valupam {

        total = total + arr[i];
        i = i + 1;

    }

    poda total / arr:valupam;
}


? recursive fibonacci ?
pindi fib(pidi n){

    nok n < 2 {
        poda n;
    };

    poda fib(n-1) + fib(n-2);
}


? count even numbers ?
pindi countEven(pidi arr kootam){

    pidi i = 0;
    pidi count = 0;

    ittuthiri i < arr:valupam {

        nok arr[i] % 2 == 0 {
            count = count + 1;
        };

        i = i + 1;
    }

    poda count;
}


? reverse array ?
pindi reverseArray(pidi arr kootam){

    pidi left = 0;
    pidi right = arr:valupam - 1;

    ittuthiri left < right {

        pidi temp = arr[left];
        arr[left] = arr[right];
        arr[right] = temp;

        left = left + 1;
        right = right - 1;
    }

    poda arr;
}


? string tests ?
pidi name = "LanguageTest";

para "\nString Info" koode "\n";
para name koode "\n";

para "Length:" koode "\n";
para name:kanam koode "\n";

para "First Char ASCII:" koode "\n";
para name[0]:THA_ASCII koode "\n";


? run analysis ?

para "\nStatistics" koode "\n";

para "Maximum:" koode "\n";
para findMax(dataset) koode "\n";

para "Minimum:" koode "\n";
para findMin(dataset) koode "\n";

para "Average:" koode "\n";
para average(dataset) koode "\n";

para "Even Count:" koode "\n";
para countEven(dataset) koode "\n";


? reverse dataset ?

para "\nReversed Dataset" koode "\n";

pidi rev kootam = reverseArray(dataset);

para rev koode "\n";


? fibonacci test ?

para "\nFibonacci Sequence" koode "\n";

pidi i = 0;

ittuthiri i < 7 {

    para fib(i) koode "\n";
    i = i + 1;

}
