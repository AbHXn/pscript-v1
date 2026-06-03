pindi _dataHolder(){
    pidi let kootam = {};
    pidi value kootam = {};
	pidi first_letter kootam = {};

    pindi addToLet( pidi char, pidi val ){
        pidi ascii_value = char:THA_ASCII;
        nok ascii_value >= let:valupam {
            pidi i = let:valupam;
            ittuthiri i <= ascii_value {
                let[i] = -1;
                i = i + 1;
            }
        };
        let[ ascii_value ] = val;
    }
    pindi addToValue( pidi val, pidi char ){
        nok val >= value:valupam {
            pidi i = value:valupam;
            ittuthiri i <= val {
                value[i] = "";
                i = i + 1;
            }
        };
        value[ val ] = char;
    }

    pindi print_result( ){
        pidi index = 0;
        ittuthiri index < let:valupam {
            nok let[index] != -1{
                pidi x = index:PO_ASCII;
                para x koode let[index] koode "\n";
            };
            index = index + 1;        
        }
    }
    pidi raw_list kootam = { let, value, first_letter };
    pidi functions kootam = { addToLet, addToValue, print_result };
    pidi final kootam = { raw_list, functions };
    poda final;
}

pindi in_list(pidi list kootam, pidi item){
    pidi index = 0;
    ittuthiri index < list:valupam {
        nok list[index] == item {
            poda sheri;
        };
        index = index + 1;
    }
    poda thettu;
}

pidi ENV kootam = {};
pidi LISTS kootam = {};
pidi FUNCS kootam = {};

pindi _reset( ){
	ENV = _dataHolder();
	LISTS = ENV[0];
	FUNCS = ENV[1];
}

pindi get_value_from_char( pidi char ){
    pidi ascii_val = char:THA_ASCII;
    nok ascii_val >= LISTS[0]:valupam {
        poda -1;
    };
    poda LISTS[0][ascii_val];
}

pindi get_char_from_value( pidi val ){
    nok val >= LISTS[1]:valupam {
        poda "";
    };
    poda LISTS[1][val];
}

pindi solver(pidi x_pair kootam, pidi y_pair kootam, pidi cur_status kootam){
    pidi cur_layer = cur_status[0];
    pidi cur_let   = cur_status[1];
    pidi cur_sum   = cur_status[2];
    pidi carry     = cur_status[3];

    nok cur_layer >= x_pair:valupam {
        poda carry == 0;
    };

    nok x_pair[cur_layer]:valupam <= cur_let {
        pidi cur_y = y_pair[cur_layer];
        pidi y_sum = (cur_sum + carry) % 10;
        pidi new_carry = (cur_sum + carry) / 10;
        new_carry := INT;

        nok in_list(LISTS[2], cur_y) um y_sum == 0 {
            poda thettu;
        };

        pidi old_val = get_value_from_char(cur_y);
        nok old_val != -1 um old_val != y_sum {
            poda thettu;
        };

        pidi old_char = get_char_from_value(y_sum);
        nok old_char != "" um old_char != cur_y {
            poda thettu;
        };

        pidi addLet = FUNCS[0];
        pidi addVal = FUNCS[1];

        addLet(cur_y, y_sum);
        addVal(y_sum, cur_y);

        pidi temp kootam = { cur_layer + 1, 0, 0, new_carry };

        nok solver(x_pair, y_pair, temp){
            poda sheri;
        };

        addLet(cur_y, old_val);
        addVal(y_sum, old_char);

        poda thettu;
    };

    pidi c_let = x_pair[cur_layer][cur_let];
    pidi val = get_value_from_char(c_let);

    pidi ddd = 0;

    nok val != -1 {
        pidi temp kootam = {
            cur_layer, cur_let + 1, cur_sum + val, carry
        };
        ddd = solver(x_pair, y_pair, temp);
        poda ddd;
    };

    pidi i = 0;
    ittuthiri i < 10 {
        nok get_char_from_value(i) != "" {
            i = i + 1;
            pinnava;
        };
        nok in_list(LISTS[2], c_let) um i == 0 {
            i = i + 1;
            pinnava;
        };
        pidi addLet = FUNCS[0];
        pidi addVal = FUNCS[1];

        addLet(c_let, i);
        addVal(i, c_let);

        pidi new_status kootam = {
            cur_layer, cur_let + 1, cur_sum + i, carry
        };
        nok solver(x_pair, y_pair, new_status) {
            poda sheri;
        };
        addLet(c_let, -1);
        addVal(i, "");
        i = i + 1;
    };
    poda thettu;
}

pindi make_pair(pidi x_pair kootam, pidi y_pair kootam){
    pidi i = 0;
    pidi pair_list kootam = {};
    pidi y_list kootam = {};

    ittuthiri i < y_pair[0]:kanam {
        pidi temp kootam = {};
        pidi j = 0;
        pidi k = 0;

        ittuthiri j < x_pair:valupam {
            nok x_pair[j]:kanam > i {
                temp[k] = x_pair[j][i];
                k = k + 1;
            };
            j = j + 1;
        };
        pair_list[i] = temp;
        y_list[i] = y_pair[0][i];
        i = i + 1;
    };
    pidi final kootam = { pair_list, y_list };
    poda final;
}

pindi reverse_string(pidi str){
    pidi L = 0;
    pidi R = str:kanam - 1;

    ittuthiri L < R {
        pidi t = str[L];
        str[L] = str[R];
        str[R] = t;
        L = L + 1;
        R = R - 1;
    };
    poda str;
}

? MAIN RUNNER FOR CRYPT SOLVER ?
ittuthiri {
	para "Enter terms: ";
	pidi N = edukku;
	N := INT;

	pidi i = 0;
	pidi x_pair kootam = {};
	pidi y_pair kootam = {};
	pidi first_letters kootam = {};

	ittuthiri i < N {
    	pidi str = edukku;
    	first_letters[i] = str[0];
    	str = reverse_string(str);

    	nok i == N - 1 {
       		y_pair[0] = str;
    	} :umbi {
       		x_pair[i] = str;
    	};

    	i = i + 1;
	};

	pidi pair kootam = make_pair(x_pair, y_pair);

	_reset();
	LISTS[2] = first_letters;

	pidi status kootam = {0,0,0,0};

	nok solver(pair[0], pair[1], status) {
    	para "Solved\n";
    	pidi p_result = FUNCS[2];
    	p_result();
	} :umbi {
    	para "No Solution\n";
	};
}
