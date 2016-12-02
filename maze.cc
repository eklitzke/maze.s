#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
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
  int row;        // the row
  int col;        // the column
};

class Maze {
public:
  Maze() :nrows_(0), ncols_(0), start_(nullptr), end_(nullptr) {}

  bool InBounds(int row, int col) {
    return row >= 0 && col >= 0 && row < nrows_ && col < ncols_;
  }

  Cell *Neighbor(Cell *orig, Direction dir) {
    Direction opp = Direction::NONE;
    int row = orig->row;
    int col = orig->col;
    switch (dir) {
    case Direction::NORTH:
      row--;
      opp = Direction::SOUTH;
      break;
    case Direction::SOUTH:
      row++;
      opp = Direction::NORTH;
      break;
    case Direction::EAST:
      col++;
      opp = Direction::WEST;
      break;
    case Direction::WEST:
      col--;
      opp = Direction::EAST;
      break;
    default:
      throw std::runtime_error("invalid direction");
      break;
    }
    if (!InBounds(row, col)) {
      return nullptr;
    }
    Cell *cell = At(row, col);
    if (cell->dist != -1 || !isspace(cell->orig)) {
      return nullptr;
    }
    cell->dir = opp;
    cell->dist = orig->dist + 1;
    return cell;
  }

  Cell* At(int row, int col) {
    // TODO: bounds-checking
    const int offset = row * ncols_ + col;
    return &cells_[offset];
  }

  bool IsVisitable(int row, int col) {
    const Cell *cell = At(row, col);
    return (cell->dist == -1) && isspace(cell->orig);
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
    int row = 0;
    while (std::getline(in, line)) {
      int col = 0;
      for (const auto & c : line) {
        if (c == '\n') {
          break;
        }

        Cell &cell = cells_[ix++];
        cell.orig = c;
        cell.row = row;
        cell.col = col++;
      }
      row++;
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
      throw std::runtime_error("not reached");
    }
  }

  void Print() {
    std::set<std::pair<int, int> > path;

    Cell *cell = end_;
    int row = end_->row;
    int col = end_->col;
    path.insert(std::make_pair(row, col));
    while (cell != start_) {
      switch (cell->dir) {
      case Direction::NORTH:
        row--;
        break;
      case Direction::SOUTH:
        row++;
        break;
      case Direction::EAST:
        col++;
        break;
      case Direction::WEST:
        col--;
        break;
      default:
        throw std::runtime_error("invalid direction");
        break;
      }
      path.insert(std::make_pair(row, col));
      cell = At(row, col);
    }

    const size_t iterations = static_cast<size_t>(nrows_) * static_cast<size_t>(ncols_);
    for (size_t i = 0; i < iterations; i++) {
      row = i / ncols_;
      col = i % ncols_;
      Cell *cell = At(row, col);
      if (cell == start_) {
        std::cout << "S";
      } else if (cell == end_) {
        std::cout << "E";
      } else if (path.find(std::make_pair(row, col)) != path.end()) {
        std::cout << "@";
      } else {
        std::cout << cell->orig;
      }

      if (col == ncols_ - 1) {
        std::cout << std::endl;
      }
      std::cout << std::flush;
    }
  }

  int Solve() {
    const size_t border_size = ncols_ * 2 + nrows_ * 2 - 4;
    int row = 0;
    int col = 0;
    for (size_t i = 0; i < border_size; i++) {
      TravelBorder(&row, &col);
      if (IsVisitable(row, col)) {
        start_ = At(row, col);
        std::cout << "start at " << row << ", " << col << std::endl;
        break;
      }
    }
    if (!start_) {
      std::cerr << "failed to find start\n";
      return 1;
    }

    // finish consuming the adjacent blank spots
    while (IsVisitable(row, col)) {
      TravelBorder(&row, &col);
    }

    // Mark the distance from the start; must be done after consuming adjacent
    // blank spots.
    start_->dist = 0;

    for (size_t i = 0; i < border_size; i++) {
      TravelBorder(&row, &col);
      if (IsVisitable(row, col)) {
        Cell *cell = At(row, col);
        if (cell == start_) {
          continue;
        }
        end_ = cell;
        std::cout << "end at " << end_->row << ", " << end_->col << std::endl;
        break;
      }
    }
    if (!end_) {
      std::cerr << "failed to find end\n";
      return 1;
    }

    std::list<Cell*> bfs_queue{start_};

    if (Explore(&bfs_queue)) {
      std::cout << "Success!\n";
      Print();
    } else {
      std::cout << "Fail :-(\n";
    }
    return 0;
  }

  bool Explore(std::list<Cell*> *cells) {
    if (cells->empty()) {
      return false;
    }

    // get the front cell
    Cell* cell = cells->front();
    if (cell == end_) {
      return true;
    }
    cells->pop_front();

    Cell *neighbor;
    if ((neighbor = Neighbor(cell, Direction::NORTH))) {
      cells->push_back(neighbor);
    }
    if ((neighbor = Neighbor(cell, Direction::SOUTH))) {
      cells->push_back(neighbor);
    }
    if ((neighbor = Neighbor(cell, Direction::EAST))) {
      cells->push_back(neighbor);
    }
    if ((neighbor = Neighbor(cell, Direction::WEST))) {
      cells->push_back(neighbor);
    }
    return Explore(cells);
  }

private:
  int nrows_;
  int ncols_;
  std::unique_ptr<Cell[]> cells_;
  Cell* start_;
  Cell* end_;
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
