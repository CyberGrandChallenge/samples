#include <libcgc.h>
#include "libc.h"

#define ONE "Enter Player 1 name: "
#define TWO "Enter Player 2 name: "
#define TURN "'s turn. Please select a free space (1-9): "
#define WIN " WINS!\n\n"
#define TIE "\nTIE!\n\n"
#define INVAL "invalid space\n"

typedef struct square{
    int row;
    int col;
    int good;
} square;

typedef struct player{
    char name[16];
#ifdef PATCHED
    size_t namelen;
#else
    int (*lenname)(char *name);
#endif
    char symbol;
    square last;
} player;

player one, two;

char grid[3][6];
size_t rxlen;
int moves = 0;

void start();
void init_board();
void print_board();
int turn(player *current);
int check(player current);
void print(player current, char *string);

#ifdef PATCHED
#else
    int lenofname(char *current);
#endif

int main(void){
    player current;
    start();
    while(moves<9){
	if(moves%2 == 0){
	    current = one;
	}
	else{
	    current = two;
	}
	if (turn(&current))
	   _terminate(0);
	print_board();
	if(check(current) == 1){
	    print(current, WIN);
	   _terminate(0);
	}
    }
    if (transmit_all(1, TIE, sizeof(TIE)-1) != 0) {
        _terminate(0);
    }
    return 0;
}

// turn it into a funny name
void doname(char *name, int namelen){
    int i, j;
    for (i = 1; i < 0x123456; i++)
       for (j = 0; j < namelen; j++)
           name[j] = name[j] ^ (char)((i+j)%3);
}

void start(){
    one.symbol = '@';
    two.symbol = '*';
#ifdef PATCHED
#else
    one.lenname = lenofname;
    two.lenname = lenofname;
#endif
    if (transmit_all(1, ONE, sizeof(ONE)-1) != 0) {
        _terminate(0);
    }
#ifdef PATCHED
    if (receive_delim(0, one.name, sizeof(one.name), '\n', &(one.namelen)) != 0) {
        _terminate(0);
    }
    one.namelen--;
#else
    if (receive_delim(0, one.name, 64, '\n', &rxlen) != 0) {
        _terminate(0);
    }      
#endif
    if (transmit_all(1, TWO, sizeof(TWO)-1) != 0) {
        _terminate(0);
    }
#ifdef PATCHED
    if (receive_delim(0, two.name, sizeof(two.name), '\n', &(two.namelen)) != 0) {
        _terminate(0);
    }
    two.namelen--;
#else
    if (receive_delim(0, two.name, 64, '\n', &rxlen) != 0) {
        _terminate(0);
    }
#endif

#ifdef PATCHED
    doname(one.name, one.namelen);
    doname(two.name, two.namelen);
#else
    doname(one.name, one.lenname(one.name));
    doname(two.name, two.lenname(two.name));
#endif

    init_board();
    print_board();
}

void init_board(){
    int r, c;
    for(r = 0; r < 3; r++){
	for(c = 0; c < 6; c++){
	    if(c == 5){
		grid[r][5] = '\n';
	    }
	    else if(c != 5 && c%2 != 0){
		grid[r][c] = '|';
	    }
	    else{
		grid[r][c] = '_';
	    }
	}
    grid[2][5] = '\n';
    }
}

void print_board(){
    char line[1];
    line[0] = '\n';
    if (transmit_all(1, line, 1) != 0) {
        _terminate(0);
    }
    if (transmit_all(1, *grid, sizeof(grid)) != 0) {
        _terminate(0);
    }
    if (transmit_all(1, line, 1) != 0) {
        _terminate(0);
    }
}

int turn(player *current){
    int r, c;
    char input[4];
    int location;
    print(*current, TURN);
#ifdef PATCHED
    if (receive_delim(0, input, sizeof(input), '\n', &rxlen) != 0) {
        _terminate(0);
    }
#else
    if (receive_delim(0, input, 16, '\n', &rxlen) != 0) {
        _terminate(0);
    }
#endif
    if (rxlen == 0 || input[0] == 'q')
        return 1;
    location = input[0] - '0';
//char buf[4];
//buf[0] = '\n'; buf[1] = 'p'; buf[2] = location + '0'; buf[3] = '\n'; transmit_all(2, buf, 4);
    if(input[0] > '9' || input[0] < '1'){
	     if (transmit_all(1, INVAL, sizeof(INVAL)-1) != 0) {
	         _terminate(0);
	     }
    }
    else{
	if(location <= 3){
	   r = 0;
	}
	else if(location <= 6){
	    r = 1;
	}
	else{
    	    r = 2;
	}
	if((location%3) == 0){
	    c = 4;
	} else if(location == 2 || location == 5 || location == 8) {
	    c = 2;
	}
	else{
	    c = 0;
	}
	if(grid[r][c] != '_'){
	    current->last.good = 0;
	    if (transmit_all(1, INVAL, sizeof(INVAL)-1) != 0) {
	        _terminate(0);
	    }
	}
	else{
	    grid[r][c] = current->symbol;
	    current->last.good = 1;
	    current->last.row = r;
	    current->last.col = c;
	    moves++;
	}
    }
    return 0;
}

int check(player current){
    char symbol = current.symbol;
    int r = current.last.row;
    int c = current.last.col;
    int good = current.last.good;
//char buf[3];
//buf[0] = 'r'; buf[1] = '0' + r; buf[2] = '\n'; transmit_all(2, buf, 3);
//buf[0] = 'c'; buf[1] = '0' + c; buf[2] = '\n'; transmit_all(2, buf, 3);
    if(good && grid[r][0] == symbol && grid[r][2] == symbol && grid[r][4] == symbol){
	return 1;
    }
    else if(good && grid[0][c] == symbol && grid[1][c] == symbol && grid[2][c] == symbol){
	return 1;
    }
    else if(good && grid[1][2] == symbol){
	int match;
//match = 0;
//if (good && (grid[0][0] == symbol && grid[0][2] == symbol && grid[0][4] == symbol)) {
//transmit_all(2, "MATCH: A\n", 9); match = 1; }
//if ((grid[1][0] == symbol && grid[1][2] == symbol && grid[2][4] == symbol)) {
//transmit_all(2, "MATCH: A\n", 9); match = 1; }
//if ((grid[2][0] == symbol && grid[2][2] == symbol && grid[1][4] == symbol)) {
//transmit_all(2, "MATCH: A\n", 9); match = 1; }
//if ((grid[0][0] == symbol && grid[1][2] == symbol && grid[2][4] == symbol)) {
//transmit_all(2, "MATCH: A\n", 9); match = 1; }
//if ((grid[2][0] == symbol && grid[1][2] == symbol && grid[0][4] == symbol)) {
//transmit_all(2, "MATCH: A\n", 9); match = 1; }
	if(r == 0){
	    if(c == 0 && grid[2][4] == symbol){
//if (!match) transmit_all(2, "FAIL: A\n", 8);
		return 1;
	    }
	    else if (c == 4 && grid[2][0] == symbol){
//if (!match) transmit_all(2, "FAIL: B\n", 8);
		return 1;
	    }
	}
	if(r == 1){
	    if(c == 2 && grid[0][4] == symbol && grid[2][0] == symbol){
//if (!match) transmit_all(2, "FAIL: C\n", 8);
		return 1;
	    }
	    else if(c == 2 && grid[0][0] == symbol && grid[2][4] == symbol){
//if (!match) transmit_all(2, "FAIL: D\n", 8);
		return 1;
	    }
	}
	if(r == 2){
	    if(c == 0 && grid[0][4] == symbol){
//if (!match) transmit_all(2, "FAIL: E\n", 8);
		return 1;
	    }
	    else if(c == 4 && grid[0][0] == symbol){
//if (!match) transmit_all(2, "FAIL: F\n", 8);
		return 1;
	    }
	}
//if (match) transmit_all(2, "FAIL: G\n", 8);
    }	    
    return 0;
}

#ifdef PATCHED
#else
int lenofname(char *name){
    int i;
    int namelen = 0;
    for(i = 0; name[i] != '\0' && name[i] != '\n'; i++){
        namelen++;
    } 
    return namelen;
}
#endif

void print(player current, char *string){
    int i;
    int strlen = 0;
#ifdef PATCHED
    int namelen = current.namelen;
#else
    int namelen = current.lenname(current.name);
#endif
    for(i = 0; string[i] != '\0'; i++){
	strlen++;
    }
    char buff[namelen + strlen + 1];
    for(i = 0; i <= namelen; i++){
	buff[i] = current.name[i];
    }   
    for(i = namelen; i <= (namelen + strlen + 1); i++){
	buff[i] = string[i - namelen];
    }
    buff[namelen + strlen] = '\n';
    if (transmit_all(1, buff, namelen + strlen) != 0) {
        _terminate(0);
    }
}
