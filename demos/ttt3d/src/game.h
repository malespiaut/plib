
#define STILL_PLAYING 0
#define HUMAN_WIN     1
#define COMPUTER_WIN  2
#define DRAWN_GAME    3

void game_init       () ;
char get_board_entry ( int x, int y, int z ) ;
int  game_update     ( int x, int y, int z ) ;

