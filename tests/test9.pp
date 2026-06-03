pindi clamp(pidi x, pidi lo, pidi hi){
    pidi r = 0;
    nok x < lo {
        r = lo;
    } :umbi {
        nok x > hi {
            r = hi;
        } :umbi {
            r = x;
        };
    };
    poda r;
}

pindi sumDigits(pidi n){
    pidi s = 0;
    pidi t = n;
    ittuthiri t > 0 {
        s = s + (t % 10);
        t = t / 10;
    }
    poda s;
}

pindi makeAccumulator(pidi start){
    pidi total = start;
    pindi add(pidi x){
        pidi r = 0;
        total = total + x;
        r = total;
        poda r;
    }
    pidi f = add;
    poda f;
}

pindi makepindityEngine(){
    pidi evenRef = 0;
    pidi oddRef = 0;
    pidi calls = 0;

    pindi even(pidi n){
        pidi r = 0;
        calls = calls + 1;
        nok n <= 0 {
            r = sheri;
        } :umbi {
            r = oddRef(n - 1);
        };
        poda r;
    }

    pindi odd(pidi n){
        pidi r = 0;
        calls = calls + 1;
        nok n <= 0 {
            r = thettu;
        } :umbi {
            r = evenRef(n - 1);
        };
        poda r;
    }
    evenRef = even;
    oddRef = odd;
    pindi run(pidi n){
        pidi r = evenRef(n);
        poda r;
    }
    pidi f = run;
    poda f;
}

pindi recursiveMix(pidi n){
    pidi r = 0;

    nok n <= 0 {
        r = 0;
    } :umbi {
        pidi a = recursiveMix(n - 1);
        pidi b = sumDigits(n * 13);
        r = a + b;
    };
    poda r;
}


pindi deepCompute(pidi n){
    pidi i = 0;
    pidi acc = 0;
    pidi r = 0;

    ittuthiri i < n {
        pidi j = 0;
        pidi inner = 0;

        ittuthiri j <= i {
            nok j % 3 == 0 {
                inner = inner + j;
            } :umbi {
                inner = inner - j;
            };
            j = j + 1;
        }

        acc = acc + clamp(inner, -10, 10);
        i = i + 1;
    }
    r = acc;
    poda r;
}
pidi a = clamp(15, 0, 10);
para a koode "\n";

pidi b = sumDigits(987);
para b koode "\n";

pidi acc = makeAccumulator(5);
para acc(3) koode "\n";
para acc(4) koode "\n";

pidi pe = makepindityEngine();
para pe(6) koode "\n";
para pe(9) koode "\n";

pidi c = recursiveMix(4);
para c koode "\n";

pidi d = deepCompute(7);
para d koode "\n";
