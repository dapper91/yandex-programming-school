#!/bin/env python3

from itertools import *
from collections import namedtuple
from copy import deepcopy

Island = namedtuple('Island', ['x', 'y'])

Finished = 'F'
SingleBridge = {'horizontal':'—', 'vertical':'|'}
DoubleBridge = {'horizontal':'═', 'vertical':'║'}
 
def solve(board):
  return brutforce_solve(heuristic_solve(board))  

def heuristic_solve(board):
  for x,y in product(range(len(board)), range(len(board[0]))):    
    if board[x][y] in [str(n) for n in range(1,9)]:          
      neighbours = get_neighbours(board, Island(x ,y))

      if int(board[x][y])%2 == 0 and int(board[x][y])/2 == len(neighbours):
        Bridge = DoubleBridge
      elif int(board[x][y])%2 == 1 and (int(board[x][y])+1)/2 == len(neighbours):
        Bridge = SingleBridge
      else:
        continue

      for neighbour in neighbours:
        if set_bridge(board, Island(x, y), neighbour, Bridge) is None:            
          return None
      return solve(deepcopy(board))

  return board      

def brutforce_solve(board):
  if board is None:
    return None

  for x,y in product(range(len(board)), range(len(board[0]))):    
    if board[x][y] in [str(n) for n in range(1,9)]:           
      for neighbour in get_neighbours(board, Island(x ,y)): 
        result = solve(set_bridge(deepcopy(board), Island(x, y), neighbour, SingleBridge))
        if result:
          return result
        else: 
          continue
   
    if board[x][y] in [str(n) for n in range(1,9)]:
      return None

  return board

def get_neighbours(board, island):
  neighbors = []
  for xdirection, ydirection in [(0,-1),(0,1),(-1,0),(1,0)]:
    coordinates = ((island.x+xdirection*step, island.y+ydirection*step) for step in range(1, max(len(board[0]), len(board))))  

    for x, y in coordinates:
      if not (-1<x<len(board) and -1<y<len(board[0])):
        break        
      if board[x][y] in [DoubleBridge['vertical'], DoubleBridge['horizontal'], Finished]:        
        break
      if xdirection == 0 and board[x][y] == SingleBridge['vertical']:
        break
      if ydirection == 0 and board[x][y] == SingleBridge['horizontal']:        
        break
      if board[x][y] not in ['0', SingleBridge['horizontal'], SingleBridge['vertical']]:        
        neighbors.append(Island(x, y))
        break

  return neighbors

def set_bridge(board, island1, island2, bridge): 
  island1_new_value = int(board[island1.x][island1.y]) - (1 if bridge == SingleBridge else 2)
  island2_new_value = int(board[island2.x][island2.y]) - (1 if bridge == SingleBridge else 2)

  if island1_new_value < 0 or island2_new_value < 0:    
    return None

  board[island1.x][island1.y] = Finished if island1_new_value == 0 else str(island1_new_value)
  board[island2.x][island2.y] = Finished if island2_new_value == 0 else str(island2_new_value)

  if island1.x == island2.x:
    for y in range(min(island1.y, island2.y)+1, max(island1.y, island2.y)): 
      board[island1.x][y] = DoubleBridge['horizontal'] if board[island1.x][y] == SingleBridge['horizontal'] else bridge['horizontal']

  if island1.y == island2.y:
    for x in range(min(island1.x, island2.x)+1, max(island1.x, island2.x)):      
      board[x][island1.y] = DoubleBridge['vertical'] if board[x][island1.y] == SingleBridge['vertical'] else bridge['vertical']

  return board

def main():
  try:
    board = solve([row.split() for row in open(input("input board file: "), "r")])  
    open(input("output board file: "), "w+").write("\n".join([" ".join(row) for row in board]))
  except IOError:
    print("file not found")
  except TypeError:
    print("solution not found")

if __name__ == '__main__':
  main()