#ifndef _CHESS_H_
#define _CHESS_H_

#include "arr.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <unistd.h>
#include <time.h>


#ifdef __unix__
#define _UNIX

#include <ncurses.h>
#elif defined(_WIN64)
#define _WINDOWS

#include <ncurses/ncurses.h>
#endif

#ifndef __x86_64__
#error "32bit device is not supported"
#endif

#define checkBoundaries(_row, _col, _destPtr, _eval) \
    if (_row < 0 || _row >= _destPtr->row || _col < 0 || _col >= _destPtr->col) _eval \

#define derefptr_unsafe(_t, _src, _i) ((_t)((u64ptr)_src)[_i])

#define PAWN   112
#define BISHOP 98
#define KNIGHT 110
#define ROOK   114
#define QUEEN  113
#define KING   107

#define WHITE  8
#define BLACK 16

#define WINDOW_EXIT 0
#define WINDOW_MAIN 100
#define WINDOW_CREDIT 101
#define WINDOW_GAME 200
#define WINDOW_GAME_DEFAULT 201
#define WINDOW_GAME_ADVANCED 202

#define ARROW_LEFT 260
#define ARROW_RIGHT 261
#define ARROW_UP 259
#define ARROW_DOWN 258

typedef struct _position {
    i32 row;
    i32 col;
} pos;

typedef struct _square {
    u32 row;
    u32 col;

    struct _piece {
        u8 id;
        u8 color;
        u16 behavior;
    } piece;

    array predict;
} square;

typedef struct _history {
    pos from;
    pos to;
} history;

typedef struct _profile {
    u16 moveCount;
    u16 score;
} profile;

typedef struct _board {
    square** square;
    u32 row;
    u32 col;

    array history;
    array onBoard;

    struct _statistic {
        profile white;
        profile black;
    } stats;

} board;

typedef struct _client {
    u32 input;
    u8 quitTrigger;
    u8 naviTrigger;
    u8 window;
    u8 select;
    u8 exit;

    struct _inputSlot {
        u32* slot;
        u8 at;
    } inpSlot;

    struct _screen {
        u16 width;
        u16 height;
    } screen;
} client;

typedef pos* posptr;

const i8 piece_directions[8][2] = {
    {0, 1}, {0, -1}, {-1, 0}, {1, 0},
    {-1, 1}, {1, -1}, {1, 1}, {-1, -1}
};

const i8 knight_directions[8][2] = {
    { -2, -1 }, { -1, -2 }, { 1, -2 }, {2, -1}, 
    {2, 1}, {1, 2}, {-1, 2}, {-2, 1}
};

const i8* client_main_options[3] = {
    "start", "credit", "exit"
};

const i8 client_main_spacing[3]= {
    10, 2, -7
};

const i8* client_gameSettings_options[3]= {
    "default", "advanced", "return"
};

const i8 client_gameSettings_spacing[3]= {
    13, 3, -8
};

const i8* client_game_default_options[3]= {
    "enter move", "reset board", "exit game"
};

const i8 client_game_default_spacing[3]= {
    19, 4, -12
};

const i8 client_game_default_input_index[4] = {
    0, 1, 7, 8
};

static inline pos* utils_newPos(i32 row, i32 col) {
    pos* result = (pos*) malloc(sizeof(pos));
    result->row = row;
    result->col = col;

    return result;
}

board* board_allocate(u32 row, u32 col);
void board_setup(board* dest, const i8* fen);
void board_cleanup(board* dest);
void board_update(board* dest);
void board_move(board* dest, pos cur, pos nxt);

array filter_qrb(board* dest, u32 row, u32 col);
array filter_king(board* dest, u32 row, u32 col);
array filter_knight(board* dest, u32 row, u32 col);
array filter_pawn(board* dest, u32 row, u32 col);

client* client_ui_setup();
void client_MainHandler();
void client_ui_bar(client* cli);
void client_ui_main(client* cli);
void client_ui_game(client* cli);
void client_ui_game_default(client* cli);
void client_ui_credit(client* cli);
void client_ui_exit(client* cli);
void client_key_bar(client* cli);
void client_key_main(client* cli);
void client_key_game(client* cli);
void client_key_game_default(client* cli);
void client_key_credit(client* cli);
void client_key_exit(client* cli);

void client_clear_main(client* cli);
void client_clear_game(client* cli);
void client_clear_credit(client* cli);

#endif 
