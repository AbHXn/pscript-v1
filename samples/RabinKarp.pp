pidi DMAX = 256;

pindi RabinKarp(pidi mainString, pidi search, pidi q){
    pidi curHash = 0;
    pidi patHash = 0;
    pidi maxPow = 1;
    pidi pSize = search:kanam;
    pidi sSize = mainString:kanam;

    pidi x = 0;
    ittuthiri x < pSize {
        curHash = (curHash * DMAX + mainString[x]:THA_ASCII) % q;
        patHash = (patHash * DMAX + search[x]:THA_ASCII) % q;
        x = x + 1;
    }
    x = 0;
    ittuthiri x < pSize - 1 {
        maxPow = (maxPow * DMAX) % q;
        x = x + 1;
    }
    x = 0;
    ittuthiri x < sSize - pSize + 1{
        nok ( curHash == patHash ){
            pidi match = sheri;
            pidi y = x;
            ittuthiri y < x + pSize {
                nok ( mainString[y] != search[y - x] ){
                    match = thettu;
                    theku;
                };
                y = y + 1;
            }
            nok ( match ) {
                poda x;
            };
        };
        nok x < sSize - pSize {
            curHash = (curHash - (mainString[x]:THA_ASCII * maxPow) % q + q) % q;
            curHash = (curHash * DMAX + mainString[x + pSize]:THA_ASCII) % q;
        };
        x = x + 1;
    }
    poda -1;
}

pidi x = "ABCDEOFLELSOE";
pidi y = "LELS";
para RabinKarp(x, y, 10000000) koode "\n";


