pindi createHashMap(pidi size){
    pidi buckets kootam = {};
	buckets[size] = 0; ????? this will init array of size to 0 ?????

    pindi hash(pidi key){
        pidi h, index = 0, 0;
        ittuthiri index < key:kanam {
            h = h + key[index]:THA_ASCII; ??? return ascii values ???
			index = index + 1;
		}
        poda h % size;
	}
	
	pindi add_or_update(pidi key){
		pidi hash_key = hash(key);
		nok hash_key >= buckets:valupam yo buckets[hash_key]:jaadi != "KOOTAM" {
			buckets[hash_key] = {key, 1};
		}: umbi {
			nok key == buckets[hash_key][0] {
				buckets[hash_key][1] = buckets[hash_key][1] + 1;
			}: umbi {
				pidi cur = hash_key;
				ittuthiri cur < size um buckets[cur]:jaadi != "INT" {
					cur = (cur + 1) % size;
				}
				buckets[cur] = {key, 1};
			};
		};
	}

	pindi para_buckets(){
		para buckets koode "\n";
	}
	
	poda { add_or_update, para_buckets };	
}

???
	this function is for converting statement to words
???
pindi get_word_list(pidi statement){
	pidi x, y = 0, 0;
	pidi word, words_list kootam = "", {};
	
	ittuthiri x < statement:kanam {
		nok statement[x] != " " {
			word = word + statement[x];
		}: umbi {
			words_list[y] = word;
			word = "";
			y = y + 1;		
		};
		x = x + 1;
	}
	nok word:kanam != 0 {
		words_list[y] = word;
	};
	poda words_list;
}

pidi statement = edukku;

pidi lists kootam = get_word_list(statement);
pidi hashMapRes kootam = createHashMap(lists:valupam);
pidi add, print = hashMapRes[0], hashMapRes[1];

pidi x = 0;
ittuthiri x < lists:valupam {
	pidi word = lists[x];
	add(word);
	x = x + 1;
};

print();