pindi create_board( pidi N ){
	pidi board kootam = {};
	pidi row = 0;
	ittuthiri row < N {
		board[row] = {};
		pidi col = 0;
		ittuthiri col < N {
			board[row][col] = "_";
			col = col + 1;
		}
		row = row + 1;
	}
	poda board;
}

pindi print_board( pidi board kootam ){
	pidi N = board:valupam;
	pidi row = 0;
   	ittuthiri row < N {
		pidi col = 0;
		ittuthiri col < N {
			para board[row][col] koode " ";
			col = col + 1;
		}
		para "\n";
		row = row + 1;
	}
}

pindi is_valid( 
	pidi board kootam,
	pidi row, pidi col,pidi N
){
	? current col ?
	pidi cur_row = row;
	ittuthiri cur_row >= 0 {
		nok board[cur_row][col] == "Q"{
			poda thettu;
		};
		cur_row = cur_row - 1;
	}
	? upper left diagonal ?
   	cur_row = row;
	pidi cur_col = col;
	ittuthiri cur_row >= 0 um cur_col >= 0 {
		nok board[cur_row][cur_col] == "Q" {
			poda thettu;
		};
		cur_row = cur_row - 1;
		cur_col = cur_col - 1;
	}
	? upper right diagonal ?
	cur_row = row;
	cur_col = col;
	ittuthiri cur_row >= 0 um cur_col < N {
		nok board[cur_row][cur_col] == "Q" {
			poda thettu;
		};
		cur_row = cur_row - 1;
		cur_col = cur_col + 1;
	}
	poda sheri;
}
pidi total_valid = 0;
pindi NQueen( 
	pidi Board kootam,
	pidi q_cnt,
	pidi c_row,
	pidi N
){
	nok q_cnt == N {
		total_valid = total_valid + 1;
		print_board(Board);
		para "\n";
		para total_valid koode "\n";
		poda;
	};
	nok c_row >= N {
		poda;
	};
	pidi col = 0;
	ittuthiri col < N {
		nok is_valid( Board, c_row, col, N ) {
			Board[c_row][col] = "Q";
			NQueen( Board, q_cnt + 1, c_row + 1, N );
			Board[c_row][col] = "_";
		};
		col = col + 1;		
	}
}

para "Enter N: ";
pidi N = edukku;
N := INT;

pidi board kootam = create_board( N );
NQueen( board, 0, 0, N );
