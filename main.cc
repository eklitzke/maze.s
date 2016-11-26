#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Maze {
  size_t nrows;
  size_t ncols;
  char *data;

  Maze() :nrows(0), ncols(0), data(nullptr) {}

  ~Maze() {
    free(data);
    data = nullptr;
  }

  char Cell(int row, int col) {
    // TODO: bounds-checking
    const int offset = row * ncols + col;
    return data[offset];
  }

  bool Empty(int row, int col) {
    return isspace(Cell(row, col));
  }

  int Read(std::ifstream &in) {
    std::string line;
    std::getline(in, line);

    // TODO: error checking on getline call
    nrows = 1;
    for (const auto &c : line) {
      if (c != '\n') {
        ncols++;
      }
    }
    while (std::getline(in, line)) {
      nrows++;
    }

    const size_t tot = ncols * nrows;
    data = new char[tot];
    memset(data, 0, tot);

    in.clear();
    in.seekg(0);

    size_t ix = 0;
    while (std::getline(in, line)) {
      for (const auto & c : line) {
        if (c == '\n') {
          break;
        }
        data[ix++] = c;
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
      if (*col < ncols - 1) {
        // travel right along top row
        ++*col;
        return;
      } else {
        // move from top row to right edge
        ++*row;
        return;
      }
    } else if (*row == nrows - 1) {
      if (*col) {
        // travel left along bottom edge
        --*col;
        return;
      } else {
        // move from bottom row to left edge
        --*row;
      }
    } else if (*col == ncols - 1) {
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

  int Solve() {
    const size_t border_size = ncols * 2 + nrows * 2 - 4;
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

    std::pair<int, int> end;
    found = false;
    for (size_t i = 0; i < border_size; i++) {
      TravelBorder(&row, &col);
      if (Empty(row, col)) {
        end = std::make_pair(row, col);
        if (end == start) {
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

    std::cerr << "start = " << start.first << ", " << start.second << "\n";
    std::cerr << "end = " << end.first << ", " << end.second << "\n";
    return 0;
  }
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
