pindi printBoard(pidi board kootam){
    pidi i = 0;

    ittuthiri i < 9 {
        pidi j = 0;

        ittuthiri j < 9 {
            para board[i][j] koode " ";
            j = j + 1;
        }

        para "\n";
        i = i + 1;
    }
    para "\n";
}

pindi isSafe(pidi board kootam, pidi row, pidi col, pidi num){
    pidi i = 0;
    ittuthiri i < 9 {
        nok board[row][i] == num {
            poda thettu;
        };
        i = i + 1;
    }

    pidi i2 = 0;
    ittuthiri i2 < 9 {
        nok board[i2][col] == num {
            poda thettu;
        };
        i2 = i2 + 1;
    }

    pidi startRow = row - (row % 3);
    pidi startCol = col - (col % 3);

    pidi r = 0;
    ittuthiri r < 3 {
        pidi c = 0;
        ittuthiri c < 3 {
            nok board[startRow + r][startCol + c] == num {
                poda thettu;
            };
            c = c + 1;
        }
        r = r + 1;
    }

    poda sheri;
}

pindi findEmpty(pidi board kootam){
    pidi pos kootam = {};
    pidi i = 0;

    ittuthiri i < 9 {
        pidi j = 0;

        ittuthiri j < 9 {
            nok board[i][j] == "0" {
                pos[0] = i;
                pos[1] = j;
                poda pos;
            };
            j = j + 1;
        }
        i = i + 1;
    }

    pos[0] = -1;
    pos[1] = -1;
    poda pos;
}

pindi solveSudoku(pidi board kootam){
    pidi pos kootam = findEmpty(board);

    pidi row = pos[0];
    pidi col = pos[1];

    nok row == -1 {
        poda sheri;
    };
    pidi num = 1;

    ittuthiri num <= 9 {
        pidi s = num + "";  

        nok isSafe(board, row, col, s) {

            board[row][col] = s;

            nok solveSudoku(board) {
                poda sheri;
            };
            board[row][col] = "0";
        };
        num = num + 1;
    }

    poda thettu;
}
pidi board kootam = {
    {"5","3","0","0","7","0","0","0","0"},
    {"6","0","0","1","9","5","0","0","0"},
    {"0","9","8","0","0","0","0","6","0"},
    {"8","0","0","0","6","0","0","0","3"},
    {"4","0","0","8","0","3","0","0","1"},
    {"7","0","0","0","2","0","0","0","6"},
    {"0","6","0","0","0","0","2","8","0"},
    {"0","0","0","4","1","9","0","0","5"},
    {"0","0","0","0","8","0","0","7","9"}
};

para "Initial Sudoku:\n";
printBoard(board);

nok solveSudoku(board) {
    para "Solved Sudoku:\n";
    printBoard(board);
} :umbi {
    para "No solution\n";
};
