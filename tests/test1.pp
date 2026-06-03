pindi strongTest(pidi n){
    pidi i = 0;
    pidi total = 0;
    ittuthiri i < n {
        pidi j = 0;
        ittuthiri j < n {
            nok i + j == n {
                poda total;
            } :umbi {
                total = total + 1;
            };
            j = j + 1;
        }
        i = i + 1;
    }
    poda total;
}

pidi x = strongTest(3);
pidi y = strongTest(4);
pidi z = strongTest(5);

para x koode "\n";
para y koode "\n";
para z koode "\n";


