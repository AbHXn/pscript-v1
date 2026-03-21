is_safe <- function(board, row, col, num) {
  # Check row
  if (num %in% board[row, ]) return(FALSE)
  
  # Check column
  if (num %in% board[, col]) return(FALSE)
  
  # Check 3x3 box
  start_row <- ((row - 1) %/% 3) * 3 + 1
  start_col <- ((col - 1) %/% 3) * 3 + 1
  
  if (num %in% board[start_row:(start_row+2), start_col:(start_col+2)]) {
    return(FALSE)
  }
  
  return(TRUE)
}

solve_sudoku <- function(board) {
  for (row in 1:9) {
    for (col in 1:9) {
      
      if (board[row, col] == 0) {
        
        for (num in 1:9) {
          
          if (is_safe(board, row, col, num)) {
            
            board[row, col] <- num
            
            result <- solve_sudoku(board)
            
            # ✅ if solution found (not FALSE)
            if (!is.logical(result)) {
              return(result)
            }
            
            # backtrack
            board[row, col] <- 0
          }
        }
        
        return(FALSE)
      }
    }
  }
  
  return(board)
}

# Example Sudoku (0 = empty)
board <- matrix(c(
  5,3,0,0,7,0,0,0,0,
  6,0,0,1,9,5,0,0,0,
  0,9,8,0,0,0,0,6,0,
  8,0,0,0,6,0,0,0,3,
  4,0,0,8,0,3,0,0,1,
  7,0,0,0,2,0,0,0,6,
  0,6,0,0,0,0,2,8,0,
  0,0,0,4,1,9,0,0,5,
  0,0,0,0,8,0,0,7,9
), nrow = 9, byrow = TRUE)

# Solve
solution <- solve_sudoku(board)

# Print result
print(solution)
