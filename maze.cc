#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <list>
#include <memory>
#include <vector>

enum class Direction { NONE, NORTH, SOUTH, EAST, WEST };

struct Cell {
  Cell() :dir(Direction::NONE), orig('\0'), dist(-1) {}
  Cell(char c) :dir(Direction::NONE), orig(c), dist(-1) {}

  Direction dir;  // direction to predecessor
  char orig;      // original character here
  int dist;       // distance rom the start
};

class Maze {
public:
  Maze() :nrows_(0), ncols_(0) {}

  bool InBounds(int row, int col) {
    return row >= 0 && col >= 0 && row < nrows_ && col < ncols_;
  }

  char At(int row, int col) {
    // TODO: bounds-checking
    const int offset = row * ncols_ + col;
    return cells_[offset].orig;
  }

  bool Empty(int row, int col) {
    return isspace(At(row, col));
  }

  int Read(std::ifstream &in) {
    std::string line;
    std::getline(in, line);

    // TODO: error checking on getline call
    nrows_ = 1;
    for (const auto &c : line) {
      if (c != '\n') {
        ncols_++;
      }
    }
    while (std::getline(in, line)) {
      nrows_++;
    }

    const size_t tot = ncols_ * nrows_;
    cells_.reset(new Cell[tot]);

    in.clear();
    in.seekg(0);

    size_t ix = 0;
    while (std::getline(in, line)) {
      for (const auto & c : line) {
        if (c == '\n') {
          break;
        }
        cells_[ix++].orig = c;
      }
    }
    if (ix != tot) {
      std::cerr << "got ix = " << ix << ", expected ix = " << tot << "\n";
      return 1;
    }
    return 0;
  }

  void TravelBorder(int *row, int *col) {
    if (*row == 0) {
      if (*col < ncols_ - 1) {
        // travel right along top row
        ++*col;
        return;
      } else {
        // move from top row to right edge
        ++*row;
        return;
      }
    } else if (*row == nrows_ - 1) {
      if (*col) {
        // travel left along bottom edge
        --*col;
        return;
      } else {
        // move from bottom row to left edge
        --*row;
      }
    } else if (*col == ncols_ - 1) {
      // travel down along right edge
      ++*row;
    } else if (*col == 0) {
      // travel up along left edge
      --*row;
    } else {
      *row = -1;
      *col = -1;
      std::cerr << "not reached!\n";
    }
  }

#if 0
  void Print(const Path &path) {
    const size_t iterations = static_cast<size_t>(nrows_) * static_cast<size_t>(ncols_);
    std::cout << iterations << std::endl;
    for (size_t i = 0; i < iterations; i++) {
      int row = i / ncols_;
      int col = i % ncols_;
      //std::cout << i << " " << row << " " << col << std::endl;
      if (path.Contains(row, col)) {
        std::cout << "@";
      } else {
        std::cout << Cell(row, col);
      }

      if (col == ncols_ - 1) {
        std::cout << std::endl;
      }
      std::cout << std::flush;
    }
  }
#endif

  int Solve() {
    const size_t border_size = ncols_ * 2 + nrows_ * 2 - 4;
    std::pair<int, int> start;

    int row = 0;
    int col = 0;
    bool found = false;
    for (size_t i = 0; i < border_size; i++) {
      TravelBorder(&row, &col);
      if (Empty(row, col)) {
        start = std::make_pair(row, col);
        found = true;
        break;
      }
    }
    if (!found) {
      std::cerr << "failed to find start\n";
      return 1;
    }

    // finish consuming the adjacent blank spots
    while (Empty(row, col)) {
      TravelBorder(&row, &col);
    }

    found = false;
    for (size_t i = 0; i < border_size; i++) {
      TravelBorder(&row, &col);
      if (Empty(row, col)) {
        end_ = std::make_pair(row, col);
        if (end_ == start) {
          continue;
        }
        found = true;
        break;
      }
    }
    if (!found) {
      std::cerr << "failed to find end\n";
      return 1;
    }

    std::list<Cell> bfs_queue;
    if (Explore(&bfs_queue, start.first, start.second)) {
      std::cout << "Success!\n";
      //Print(path);
    } else {
      std::cout << "Fail :-(\n";
    }
    return 0;
  }

  bool Explore(std::list<Cell> *cells, int row, int col) {
    std::cout << "> exploring: " << row << ", " << col << "\n";
    if (!InBounds(row, col)) {
      std::cout << "< not in bounds\n";
      return false;
    }
    if (!Empty(row, col)) {
      std::cout << "< not empty\n";
      return false;
    }
    #if 0
    if (path->Contains(row, col)) {
      return false;
    }
    path->Push(row, col);
    if (end_.first == row && end_.second == col) {
      return true;
    }

    if (Explore(path, row - 1, col) ||
        Explore(path, row + 1, col) ||
        Explore(path, row, col - 1) ||
        Explore(path, row, col + 1)) {
      return true;
    }

    path->Pop();
    #endif
    return false;
  }

private:
  int nrows_;
  int ncols_;
  std::unique_ptr<Cell[]> cells_;
  std::pair<int, int> end_;
};

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    std::ifstream f(argv[i]);
    Maze maze;
    maze.Read(f);
    maze.Solve();
  }
  return 0;
}
