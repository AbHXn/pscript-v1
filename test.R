# Print the board
printBoard <- function(board) {
  for (i in 1:3) {
    for (j in 1:3) {
      cat(board[i, j], " ")
    }
    cat("\n")
  }
  cat("\n")
}

# Check if two boards are equal
boardsEqual <- function(a, b) {
  all(a == b)
}

# Heuristic: count of misplaced tiles (not used in BFS, but keeping)
heuristic <- function(board, goal) {
  sum((board != "_" & board != goal))
}

# Copy a board
copyBoard <- function(board) {
  board[,]  # returns a copy
}

# Find the blank position
findBlank <- function(board) {
  pos <- which(board == "_", arr.ind = TRUE)
  return(as.numeric(pos[1,]))
}

# Swap two positions
swap <- function(board, r1, c1, r2, c2) {
  temp <- board[r1, c1]
  board[r1, c1] <- board[r2, c2]
  board[r2, c2] <- temp
  return(board)
}

# Generate neighbors
getNeighbors <- function(board) {
  neighbors <- list()
  pos <- findBlank(board)
  r <- pos[1]
  c <- pos[2]
  
  index <- 1
  
  if (r > 1) {
    nb <- copyBoard(board)
    nb <- swap(nb, r, c, r-1, c)
    neighbors[[index]] <- nb
    index <- index + 1
  }
  
  if (r < 3) {
    nb <- copyBoard(board)
    nb <- swap(nb, r, c, r+1, c)
    neighbors[[index]] <- nb
    index <- index + 1
  }
  
  if (c > 1) {
    nb <- copyBoard(board)
    nb <- swap(nb, r, c, r, c-1)
    neighbors[[index]] <- nb
    index <- index + 1
  }
  
  if (c < 3) {
    nb <- copyBoard(board)
    nb <- swap(nb, r, c, r, c+1)
    neighbors[[index]] <- nb
    index <- index + 1
  }
  
  return(neighbors)
}

# Check if a board has been visited
isVisited <- function(visited, board) {
  for (v in visited) {
    if (boardsEqual(v, board)) return(TRUE)
  }
  return(FALSE)
}

# Add to visited
addVisited <- function(visited, board) {
  visited[[length(visited)+1]] <- copyBoard(board)
  return(visited)
}

# BFS Solver
solve <- function(start, goal) {
  queue <- list(start)
  visited <- list()
  head <- 1
  
  while (head <= length(queue)) {
    current <- queue[[head]]
    
    cat("Current State:\n")
    printBoard(current)
    
    if (boardsEqual(current, goal)) {
      cat("Solved\n")
      return()
    }
    
    visited <- addVisited(visited, current)
    
    neighbors <- getNeighbors(current)
    
    for (nb in neighbors) {
      if (!isVisited(visited, nb)) {
        queue[[length(queue)+1]] <- nb
      }
    }
    
    head <- head + 1
  }
  
  cat("Cannot solve\n")
}

# Initial and goal boards
start <- matrix(c("1","3","6",
                  "5","_","2",
                  "4","7","8"), nrow=3, byrow=TRUE)

goal <- matrix(c("1","2","3",
                 "4","5","6",
                 "7","8","_"), nrow=3, byrow=TRUE)

cat("Initial Board:\n")
printBoard(start)

solve(start, goal)
