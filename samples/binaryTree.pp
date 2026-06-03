pindi TreeNode(pidi root){
	???
		0 -> node_value 
		1 -> left 
		2 -> right
		3 -> add_node
		4 -> in_order
	???
	pidi node_value = root;
	pidi left kootam, right kootam = {}, {};
	
	pindi add_node( pidi data ){
		nok data < node_value {
			nok left:valupam == 0 {
				left = TreeNode(data);
			}: umbi {
				pidi add_node_func = left[3];
				add_node_func( data );
			};
		}: umbi {
			nok right:valupam == 0 {
				right = TreeNode( data );
			}: umbi {
				pidi add_node_func = right[3];
				add_node_func( data );
			};
		};
	}
	pindi in_order( ){
		nok left:valupam != 0 {
			pidi in_order_left = left[4];
			in_order_left();
		};
		para node_value koode "\n";
		nok right:valupam != 0 {
			pidi in_order_right = right[4];
			in_order_right();
		};
	}
	poda {node_value, left, right, add_node, in_order};
}

pindi create_bin_tree(pidi data kootam, pidi add_node_callback){
	pidi x = 0;
	ittuthiri x < data:valupam {
		add_node_callback( data[x] );
		x = x + 1;
	}
}

pidi nodes kootam = { 4, 6, 3, 8, 2, 6 };
pidi Root kootam  = TreeNode(5);

pidi node_value   = Root[0];
pidi left kootam  = Root[1];
pidi right kootam = Root[2];
pidi add_node 	  = Root[3];
pidi in_order 	  = Root[4];

create_bin_tree( nodes, add_node );
in_order();



