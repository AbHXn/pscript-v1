pindi absDiff(pidi a, pidi b){
    nok a > b {
        poda a - b;
    } :umbi {
        poda b - a;
    };
}

pindi sumUpto(pidi n){
    nok n <= 0 {
        poda 0;
    };
    poda n + sumUpto(n - 1);
}

pindi weirdFact(pidi n){
    nok n <= 1 {
        poda 1;
    };
    pidi sub = weirdFact(n - 1);
    nok sub > 500 {
        poda sub;
    };
    poda n * sub;
}

pindi zigzag(pidi n){
    pidi i = 0;
    pidi total = 0;

    ittuthiri i < n {
        nok i % 2 == 0 {
            total = total + i;
        } :umbi {
            total = total - i;
        };
        nok total > 30 {
            poda total;
        };
        i = i + 1;
    }
    poda total;
}


pindi recursiveMixer(pidi n){
    nok n <= 0 {
        poda 0;
    };
    pidi a = zigzag(n);
    pidi b = sumUpto(n % 5);
    pidi c = absDiff(a, b);
    nok c > 20 {
        poda c;
    };
    poda c + recursiveMixer(n - 1);
}


pindi deepLoop(pidi n){
    pidi i = 0;
    pidi result = 0;

    ittuthiri i < n {
        pidi j = 0;
        pidi inner = 0;

        ittuthiri j <= i {
            nok j % 3 == 0 {
                inner = inner + j;
            } :umbi {
                inner = inner - j;
            };
            nok inner < -20 {
                poda inner;
            };
            j = j + 1;
        }
        result = result + inner;
        i = i + 1;
    }

    poda result;
}

pindi masterTest(pidi x){
    pidi r1 = recursiveMixer(x);
    pidi r2 = deepLoop(x);
    pidi r3 = weirdFact(x % 6);
    pidi t1 = absDiff(r1, r2);
    pidi t2 = absDiff(t1, r3);

    nok t2 > 100 {
        poda t2;
    };
    poda t2 + absDiff(r2, r3);
}

pidi out1 = masterTest(5);
pidi out2 = masterTest(6);
pidi out3 = masterTest(7);

para out1 koode "\n";
para out2 koode "\n";
para out3 koode "\n";
