
def test():
  for name in ["9x9-1","9x9-2","9x9-3","16x9-1","16x9-2","22x13-1", "32x18-1"]:
    board1 = solve([row.split() for row in open("board" + name + ".in", "r")])
    board2 = [row.split() for row in open("board" + name + ".out", "r")]
    for i in range(len(board1)):
      for j in range(len(board1[0])):
        if board1[i][j] != board2[i][j]:
          print("error in " + name)
          return 1