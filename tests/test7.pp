pindi abs(pidi x){
    pidi r = 0;
    nok x < 0 {
        r = 0 - x;
    } :umbi {
        r = x;
    };
    poda r;
}

pindi sumRec(pidi n){
    pidi r = 0;
    nok n <= 0 {
        r = 0;
    } :umbi {
        r = n + sumRec(n - 1);
    };
    poda r;
}

pindi factLimit(pidi n){
    pidi r = 0;
    nok n <= 1 {
        r = 1;
    } :umbi {
        pidi sub = factLimit(n - 1);

        nok sub > 50 {
            r = sub;
        } :umbi {
            r = n * sub;
        };
    };
    poda r;
}

pindi makeToggle(pidi start){
    pidi state = start;
    pindi flip(){
        pidi r = 0;
        nok state == 0 {
            state = 1;
        } :umbi {
            state = 0;
        };
        r = state;
        poda r;
    }
    poda flip;
}

pindi makeEvenOdd(){
    pidi evenRef = 0;
    pidi oddRef = 0;

    pindi even(pidi n){
        pidi r = 0;
        nok n == 0 {
            r = sheri;
        } :umbi {
            r = oddRef(n - 1);
        };
        poda r;
    }

    pindi odd(pidi n){
        pidi r = 0;
        nok n == 0 {
            r = thettu;
        } :umbi {
            r = evenRef(n - 1);
        };
        poda r;
    }

    evenRef = even;
    oddRef = odd;

    pindi entry(pidi n){
        pidi r = evenRef(n);
        poda r;
    }
    poda entry;
}

pindi deepLoop(pidi n){
    pidi i = 0;
    pidi sum = 0;
    pidi r = 0;

    ittuthiri i < n {
        pidi j = 0;
        pidi inner = 0;

        ittuthiri j <= i {
            nok j % 2 == 0 {
                inner = inner + j;
            } :umbi {
                inner = inner - j;
            };

            j = j + 1;
        }
        sum = sum + inner;
        i = i + 1;
    }
    poda sum;
}

pidi a = abs(-7);
para a koode "\n";

pidi b = sumRec(5);
para b koode "\n";

pidi c = factLimit(6);
para c koode "\n";

pidi t = makeToggle(0);
para t() koode "\n";
para t() koode "\n";
para t() koode "\n";

pidi eo = makeEvenOdd();
para eo(4) koode "\n";
para eo(7) koode "\n";

pidi d = deepLoop(6);
para d koode "\n";

