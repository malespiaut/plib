
#define MAX_GRID     4


class Puzzle 
{
  sgCoord pos ;

  int sel  [ 3 ] ;

  ssgTransform *spin ;
  ssgTransform *selector ;
  ssgTransform *ground ;

  Cell *cell [ MAX_GRID ][ MAX_GRID ][ MAX_GRID ] ;

  int game_state ;

public:
   Puzzle () ;
  ~Puzzle () ;

  void reset () ;
  void update () ;

  int  getGameState () { return game_state ; }
  void setGameState ( int gs ) { game_state = gs ; }

  int gameOver () { return game_state != STILL_PLAYING ; }

  void cursor ( int x, int y, int z )
  {
    sel[0] = x & 3 ;
    sel[1] = y & 3 ;
    sel[2] = z & 3 ;
  }

  int getCx () { return sel[0] ; }
  int getCy () { return sel[1] ; }
  int getCz () { return sel[2] ; }

  void step_up    () { pos . xyz [ 1 ] += 0.5f ; spin->setTransform(&pos); }
  void step_down  () { pos . xyz [ 1 ] -= 0.5f ; spin->setTransform(&pos); }
  void spin_up    () { pos . hpr [ 2 ] += 5.0f ; spin->setTransform(&pos); }
  void spin_down  () { pos . hpr [ 2 ] -= 5.0f ; spin->setTransform(&pos); }
  void spin_left  () { pos . hpr [ 0 ] -= 5.0f ; spin->setTransform(&pos); }
  void spin_right () { pos . hpr [ 0 ] += 5.0f ; spin->setTransform(&pos); }

  void cursor_up    () { cursor(sel[0],sel[1],sel[2]+1) ; }
  void cursor_down  () { cursor(sel[0],sel[1],sel[2]-1) ; }
  void cursor_in    () { cursor(sel[0],sel[1]+1,sel[2]) ; }
  void cursor_out   () { cursor(sel[0],sel[1]-1,sel[2]) ; }
  void cursor_left  () { cursor(sel[0]-1,sel[1],sel[2]) ; }
  void cursor_right () { cursor(sel[0]+1,sel[1],sel[2]) ; }

  void put_empty() { cell[sel[0]][sel[1]][sel[2]]->set(WIRE_CELL) ; }
  void put_X    () { cell[sel[0]][sel[1]][sel[2]]->set(   X_CELL) ; }
  void put_O    () { cell[sel[0]][sel[1]][sel[2]]->set(   O_CELL) ; }

  void put_empty ( int x, int y, int z ) { cell[x][y][z] -> set (WIRE_CELL);}
  void put_X     ( int x, int y, int z ) { cell[x][y][z] -> set ( X_CELL) ; }
  void put_O     ( int x, int y, int z ) { cell[x][y][z] -> set ( O_CELL) ; }

  ssgTransform *getSSG () { return spin ; }
} ;


