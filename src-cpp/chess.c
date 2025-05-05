#include "chess.h"

/*
fixed:     pawn's double step only works on specific row
*/

board* cboard;

int main() {
    benchmark
    client_MainHandler();
    endbench
    return 0;
}

board* board_allocate(u32 row, u32 col) {
    board* dest = (board*) malloc(sizeof(board));
    dest->square = (square**) malloc(sizeof(square*) * row);
    dest->onBoard = new(posptr, 0);
    dest->history = new(posptr, 0);

    for (u32 i = 0; i < row; i++) {
        dest->square[i] = (square*) malloc(sizeof(square) * col);
        for (u32 j = 0; j < col; j++)
            dest->square[i][j].predict = (void*)0;
    }

    dest->row = row;
    dest->col = col;
    return dest;
}

void board_setup(board* dest, const i8* fen) {
    for (u32 i = 0; i < dest->row; i++) {
        for (u32 j = 0; j < dest->col; j++) {
            dest->square[i][j].row = i;
            dest->square[i][j].col = j;
            dest->square[i][j].piece.id = 0;
            dest->square[i][j].piece.color = 0;
            dest->square[i][j].piece.behavior = 0;
        }
    }

    u32 row = dest->row - 1;
    u32 col = 0;

    for (u32 i = 0; i < strlen(fen); i++) {
        if (fen[i] == '/') {
            row--;
            col = 0;
        } else {
            if (isdigit(fen[i])) {
                col += fen[i] - '0';
           } else {
                dest->square[row][col].piece.color = (tolower(fen[i]) == fen[i]) ? 8 : 16;
                u8 chartoint = tolower(fen[i]);

                dest->square[row][col].piece.id = chartoint;
                col++;
            }
        }
    }

    for (u32 i = 0; i < dest->row; i++)
        for (u32 j = 0; j < dest->col; j++)
            if (dest->square[i][j].piece.id) {
                posptr position = (posptr) malloc(sizeof(pos));
                position->row = i;
                position->col = j;

                push(posptr, dest->onBoard, position);
            }
}

void board_cleanup(board* dest) {
    for (u32 i = 0; i < dest->row; i++) {
        for (u32 j = 0; j < dest->col; j++) {
            if (dest->square[i][j].predict) {
                for (u32 k = 0; k < dest->square[i][j].predict->length; k++)
                    free(derefptr_unsafe(posptr, dest->square[i][j].predict->buffer, k));
                arr_free(dest->square[i][j].predict);
            }
        }
        free(dest->square[i]);
    }


    for (u32 i = 0; i < dest->history->length; i++)
        free(derefptr_unsafe(posptr, dest->history->buffer, i));
    arr_free(dest->history);

    for (u32 i = 0; i < dest->onBoard->length; i++)
        free(derefptr_unsafe(posptr, dest->onBoard->buffer, i));
    arr_free(dest->onBoard);

    free(dest->square);
    free(dest);
}

void board_update(board* dest) {
    for (u32 i = 0; i < dest->row; i++) {
        for (u32 j = 0; j < dest->row; j++) {
            if (dest->square[i][j].predict != NULL) {
                for (u32 k = 0; k < dest->square[i][j].predict->length; k++) {
                    free(derefptr_unsafe(posptr, dest->square[i][j].predict->buffer, k));
                }
    
                arr_free(dest->square[i][j].predict);
                dest->square[i][j].predict = (void*)0;
            }
            
            switch (dest->square[i][j].piece.id) {
                case PAWN:   dest->square[i][j].predict = filter_pawn(dest, i, j);   break;
                case BISHOP: dest->square[i][j].predict = filter_qrb(dest, i, j);    break;
                case KNIGHT: dest->square[i][j].predict = filter_knight(dest, i, j); break;
                case ROOK:   dest->square[i][j].predict = filter_qrb(dest, i, j);    break;
                case QUEEN:  dest->square[i][j].predict = filter_qrb(dest, i, j);    break;
                case KING:   dest->square[i][j].predict = filter_king(dest, i, j);   break;
            }
        }
    }
}

void board_move(board* dest, pos cur, pos nxt) {
    if (!dest->square[cur.row][cur.col].piece.id)
        return;

    checkBoundaries(nxt.row, nxt.col, dest, return);
    for (u32 i = 0; i < dest->square[cur.row][cur.col].predict->length; i++) {// hashmap for o1
        if (
            derefptr_unsafe(posptr, dest->square[cur.row][cur.col].predict->buffer, i)->row == nxt.row
            && 
            derefptr_unsafe(posptr, dest->square[cur.row][cur.col].predict->buffer, i)->col == nxt.col
        ) {
            dest->square[nxt.row][nxt.col] = dest->square[cur.row][cur.col];
            dest->square[cur.row][cur.col].piece.id = 0;
            dest->square[cur.row][cur.col].piece.color = 0;
            dest->square[cur.row][cur.col].piece.behavior = 0;
            dest->square[nxt.row][nxt.col].predict = (void*)0;
            dest->square[nxt.row][nxt.col].piece.behavior++;
            return;
        }
    }
}

array filter_qrb(board* dest, u32 row, u32 col) {
    square currentSquare          = dest->square[row][col];
    const u8 startDirectionIndex = (currentSquare.piece.id == BISHOP)   ?     4 : 0;
    const u8 endDirectionIndex   = (currentSquare.piece.id == ROOK)     ?     4 : 8;
    const u8 enemyColor          = (currentSquare.piece.color == WHITE) ? BLACK : WHITE;
    array returnItems = new(posptr, 0);

    for (u32 direction = startDirectionIndex; direction < endDirectionIndex; direction++) {
        i32 rowOffsets = piece_directions[direction][0];
        i32 colOffsets = piece_directions[direction][1];

        for (u32 i = 0;; i++) {
            if (i) {
                if (direction == 0 || direction == 4 || direction == 6) colOffsets += 1;
                if (direction == 1 || direction == 5 || direction == 7) colOffsets -= 1;
                if (direction == 2 || direction == 4 || direction == 7) rowOffsets -= 1;
                if (direction == 3 || direction == 5 || direction == 6) rowOffsets += 1;
            }

            i32 rowTarget = row + rowOffsets;
            i32 colTarget = col + colOffsets;

            checkBoundaries(rowTarget, colTarget, dest, break);
            if (dest->square[rowTarget][colTarget].piece.color == currentSquare.piece.color)
                break;
            
            push(posptr, returnItems, utils_newPos(rowTarget, colTarget));

            if (dest->square[rowTarget][colTarget].piece.color == enemyColor)
                break;
        }
    }

    return returnItems;
}

array filter_king(board* dest, u32 row, u32 col) {
    square currentSquare = dest->square[row][col];
    array returnItems = new(posptr, 0);

    for (u32 direction = 0; direction < 8; direction++) {
        i32 rowOffsets = piece_directions[direction][0];
        i32 colOffsets = piece_directions[direction][1];

        i32 rowTarget = row + rowOffsets;
        i32 colTarget = col + colOffsets;

        checkBoundaries(rowTarget, colTarget, dest, continue);

        if (dest->square[rowTarget][colTarget].piece.color == currentSquare.piece.color)
            continue;

        push(posptr, returnItems, utils_newPos(rowTarget, colTarget));
    }

    return returnItems;
}

array filter_knight(board* dest, u32 row, u32 col) {
    square currentSquare = dest->square[row][col];
    array returnItems = new(posptr, 0);

    for (u32 direction = 0; direction < 8; direction++) {
        i32 rowOffsets = knight_directions[direction][0];
        i32 colOffsets = knight_directions[direction][1];

        i32 rowTarget = row + rowOffsets;
        i32 colTarget = col + colOffsets;

        checkBoundaries(rowTarget, colTarget, dest, continue);

        if (dest->square[rowTarget][colTarget].piece.color == currentSquare.piece.color)
            continue;

        push(posptr, returnItems, utils_newPos(rowTarget, colTarget));
    }

    return returnItems;
}

array filter_pawn(board* dest, u32 row, u32 col) {
    square currentSquare = dest->square[row][col];
    const u8 enemyColor = (currentSquare.piece.color == WHITE)    ? BLACK : WHITE;
    const i8 moveDirection = (currentSquare.piece.color == WHITE) ?    -1 : 1;
    u8 pawnBehavior = (currentSquare.piece.behavior)              ?     1 : 2;
    array returnItems = new(posptr, 0);

    const i8 blackPawn_ontake_directions[2][2] = {{1, 1}, {1, -1}};
    const i8 whitePawn_ontake_directions[2][2] = {{-1, 1}, {-1, -1}};

    for (u32 i = 0; i < pawnBehavior; i++) {
        i32 rowTarget = row + ((i + 1) * moveDirection);

        if (rowTarget < 0 || rowTarget >= dest->row) 
            break;

        if (dest->square[rowTarget][col].piece.color)
            break;

        push(posptr, returnItems, utils_newPos(rowTarget, col));
    }

    for (u32 i = 0; i < 2; i++) {\
        i32 rowTarget, colTarget;
        if (currentSquare.piece.color == WHITE) {
            rowTarget = row + whitePawn_ontake_directions[i][0];
            colTarget = col + whitePawn_ontake_directions[i][1];
        } else {
            rowTarget = row + blackPawn_ontake_directions[i][0];
            colTarget = col + blackPawn_ontake_directions[i][1];
        }

        checkBoundaries(rowTarget, colTarget, dest, continue);

        if (dest->square[rowTarget][colTarget].piece.color == currentSquare.piece.color)
            continue;

        if (dest->square[rowTarget][colTarget].piece.color == enemyColor)
            push(posptr, returnItems, utils_newPos(rowTarget, colTarget));
    }

    return returnItems;
}

client* client_ui_setup() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    timeout(500);

    client* _client = (client*) malloc(sizeof(client));
    _client->input = 0;
    _client->naviTrigger = 0;
    _client->quitTrigger = 0;
    _client->window = WINDOW_MAIN;
    _client->select = 0;
    _client->exit = 0;
    getmaxyx(stdscr, _client->screen.height, _client->screen.width);

    _client->inpSlot.slot = (u32*) calloc(4, sizeof(u32) * 4);
    _client->inpSlot.at = 0;

    keypad(stdscr, TRUE);

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);

    return _client;
}

void client_MainHandler() {
    client* cli = client_ui_setup();
    while (!cli->exit) {
        switch (cli->window) {
            case WINDOW_MAIN:         client_ui_main(cli);         break;
            case WINDOW_GAME:         client_ui_game(cli);         break;
            case WINDOW_GAME_DEFAULT: client_ui_game_default(cli); break;
            case WINDOW_CREDIT:       client_ui_credit(cli);       break;
            case WINDOW_EXIT:         client_ui_exit(cli);         break;
        }
        client_ui_bar(cli);

        cli->input = getch();

        client_key_bar(cli);
        switch (cli->window) {
            case WINDOW_MAIN:         client_key_main(cli);         break;
            case WINDOW_GAME:         client_key_game(cli);         break;
            case WINDOW_GAME_DEFAULT: client_key_game_default(cli); break;
            case WINDOW_CREDIT:       client_key_credit(cli);       break;
            case WINDOW_EXIT:         client_key_exit(cli);         break;
        }
        usleep(10000);
    }
    printf("[client_UIHandler] terminated!\n");
    endwin();
}

void client_ui_bar(client* cli) {
    move(cli->screen.height - 2, 0);
    clrtoeol();
    move(cli->screen.height - 1, 0);
    clrtoeol();

    mvprintw(cli->screen.height - 2, 0, "welcome");
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(cli->screen.height - 2, 8, "placeh!");
    attroff(COLOR_PAIR(1) | A_BOLD);

    if (cli->quitTrigger) {
        mvprintw(cli->screen.height - 2, cli->screen.width - 32, "press 'q' or 'esc' again to exit");
        mvprintw(cli->screen.height - 1, cli->screen.width - 23, "press any key to cancel");
    } else if  (cli->naviTrigger) {
        mvprintw(cli->screen.height - 2, cli->screen.width - 27, "h: <- | j: v | k: ^ | l: ->");
        mvprintw(cli->screen.height - 1, cli->screen.width - 23, "       <enter>: confirm");
    } else {
        mvprintw(cli->screen.height - 2, cli->screen.width - 27, " press 'q' or 'esc' to exit");
        if (cli->window == WINDOW_GAME_DEFAULT)
            mvprintw(cli->screen.height - 1, cli->screen.width - 21, "use arrow to navigate");
        else
            mvprintw(cli->screen.height - 1, cli->screen.width - 31, "use 'hjkl' or arrow to navigate");
    }

    if (cli->input == '\n')
        mvprintw(cli->screen.height - 1, 0, "<enter>");
    else
        mvprintw(cli->screen.height - 1, 0, "%d", cli->input);

    u16 warningSlot = 6;

    if (cli->screen.width < 100 || cli->screen.height < 30) {
        mvprintw(cli->screen.height - warningSlot, 0, "[");
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(cli->screen.height - warningSlot, 1, "WARNING!");
        attroff(COLOR_PAIR(4) | A_BOLD);
        mvprintw(cli->screen.height - warningSlot, 8, "]");
        mvprintw(cli->screen.height - warningSlot + 1, 0, "required atleast 100x30 screen");
        mvprintw(cli->screen.height - warningSlot + 2, 0, "current resolution: %dx%d", cli->screen.width, cli->screen.height);

        warningSlot += 4;
    }

    if (((float)cli->screen.width/cli->screen.height) >= 4.0f) {
        mvprintw(cli->screen.height - warningSlot, 0, "[");
        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(cli->screen.height - warningSlot, 1, "WARNING!");
        attroff(COLOR_PAIR(4) | A_BOLD);
        mvprintw(cli->screen.height - warningSlot, 8, "]");
        mvprintw(cli->screen.height - warningSlot + 1, 0, "required atmost 4:1 aspect ratio");
        mvprintw(cli->screen.height - warningSlot + 2, 0, "current aspect ratio: %.2f:2", (float)cli->screen.width/cli->screen.height);
    }
}

void client_ui_main(client* cli) {
    attron(A_BOLD);
    mvprintw((cli->screen.height / 2) - 2, (cli->screen.width / 2) - 13, "CHESS TERMINAL PROJECT 2024");
    attroff(A_BOLD);

    for (u8 i = 0; i < 3; i++) {
        if (i == cli->select) {
            attron(A_REVERSE);
            mvprintw (
                cli->screen.height / 2, (cli->screen.width / 2) - client_main_spacing[i], 
                "%s", client_main_options[i]
            );
            attroff(A_REVERSE);
        } else
            mvprintw (
                cli->screen.height / 2, (cli->screen.width / 2) - client_main_spacing[i],
                "%s", client_main_options[i]
            );
    }

    mvprintw(cli->screen.height / 2, (cli->screen.width  / 2) - 4, "|");
    mvprintw(cli->screen.height / 2, (cli->screen.width  / 2) + 5, "|");
}

void client_ui_game(client* cli) {
    attron(A_BOLD);
    mvprintw((cli->screen.height / 2) - 4, (cli->screen.width / 2) - 6, "GAME SETTINGS");
    attroff(A_BOLD);

    if (cli->select == 0) {
        mvprintw((cli->screen.height / 2) - 2, (cli->screen.width / 2) - 29, "enjoy the classic chess experiences with the standard rules");
        mvprintw((cli->screen.height / 2) - 1, (cli->screen.width / 2) - 38, "great for a quick and familiar game that offers balance in strategy and skill");
    }

    if (cli->select == 1) {
        mvprintw((cli->screen.height / 2) - 2, (cli->screen.width / 2) - 29, " customizable board and set up the pieces however you like ");
        mvprintw((cli->screen.height / 2) - 1, (cli->screen.width / 2) - 38, "   perfect for trying out new setups or creating your own chess variations    ");
    }

    if (cli->select == 2) {
        mvprintw((cli->screen.height / 2) - 2, (cli->screen.width / 2) - 29, "                    return to main menu                    ");
        mvprintw((cli->screen.height / 2) - 1, (cli->screen.width / 2) - 38, "                                                                             ");
    }

    for (int i = 0; i < 3; i++) {
        if (i == cli->select) {
            attron(A_REVERSE);
            mvprintw((cli->screen.height / 2) + 2, (cli->screen.width / 2) - client_gameSettings_spacing[i], "%s", client_gameSettings_options[i]);
            attroff(A_REVERSE);
        } else
            mvprintw((cli->screen.height / 2) + 2, (cli->screen.width / 2) - client_gameSettings_spacing[i], "%s", client_gameSettings_options[i]);
    }

    mvprintw((cli->screen.height / 2) + 2, (cli->screen.width / 2) - 5, "|");
    mvprintw((cli->screen.height / 2) + 2, (cli->screen.width / 2) + 6, "|");
}

void client_ui_game_default(client* cli) {
    attron(A_BOLD);
    mvprintw(1, (cli->screen.width / 2) - 11, "DEFAULT CHESSBOARD v3.1");
    attroff(A_BOLD);

    u16 cell_width = (cli->screen.width / 42) * 2;
    u16 cell_height = (cli->screen.width / 42);
    
    u16 start_y = 4;
    u16 start_x = (cli->screen.width / 2) - (7 * (cell_width / 2));
    
    for (u16 i = 0; i < 8; i++) {
        for (u16 j = 0; j < 8; j++) {
            if (cboard->square[i][j].piece.id) {
                if (cboard->square[i][j].piece.color == 8) {
                    attron(COLOR_PAIR(3));
                    mvprintw(
                        start_y + (i * cell_height),
                        start_x + (j * cell_width) - 1,
                        " %c ", 
                        cboard->square[i][j].piece.id
                    );
                    attroff(COLOR_PAIR(3));
                } else {
                    attron(COLOR_PAIR(4));
                    mvprintw(
                        start_y + (i * cell_height),
                        start_x + (j * cell_width) - 1,
                        " %c ", 
                        cboard->square[i][j].piece.id - 32
                    );
                    attroff(COLOR_PAIR(4));
                }
            } else {
                mvprintw(
                    start_y + (i * cell_height),
                    start_x + (j * cell_width) - 1,
                    " _ "
                );
            }
        }
    }

    if (cli->inpSlot.at >= 2) {
        u16 row = abs((cli->inpSlot.slot[1] - 48) - 8);
        u16 col = cli->inpSlot.slot[0] - 65;

        attron(A_BOLD | A_UNDERLINE);
        if (cboard->square[row][col].piece.color == WHITE) {
            attron(COLOR_PAIR(3));
            mvprintw(
                start_y + row * cell_height,
                start_x + col * cell_width - 1,
                " %c ", cboard->square[row][col].piece.id
            );
            attroff(COLOR_PAIR(3));
        } else if (cboard->square[row][col].piece.color == BLACK) {
            attron(COLOR_PAIR(4));
            mvprintw(
                start_y + row * cell_height,
                start_x + col * cell_width - 1,
                " %c ", cboard->square[row][col].piece.id - 32
            );
            attroff(COLOR_PAIR(4));
        }
        attroff(A_BOLD | A_UNDERLINE);

        if (cboard->square[row][col].predict) {
            for (u16 k = 0; k < cboard->square[row][col].predict->length; k++) {
                posptr current = derefptr_unsafe(posptr, cboard->square[row][col].predict->buffer, k);

                if (current->row == abs((cli->inpSlot.slot[3] - 48) - 8) && current->col == cli->inpSlot.slot[2] - 65 && cli->inpSlot.at >= 4)
                    attron(COLOR_PAIR(1));

                u8 target = cboard->square[current->row][current->col].piece.id;
                if (cboard->square[current->row][current->col].piece.color == BLACK)
                    target -= 32;
                if (!target) 
                    target = '_';

                mvprintw(
                    start_y + (current->row * cell_height),
                    start_x + (current->col * cell_width),
                    "%c%d",
                    current->col + 65, 
                    abs(current->row - 8)
                );

                if (target >= 97) attron(COLOR_PAIR(3));
                else if (target >= 65 && target != 95) attron(COLOR_PAIR(4));

                mvprintw(
                    start_y + (current->row * cell_height),
                    start_x + (current->col * cell_width - 1),
                    "%c", target
                );
                if (target >= 97) attroff(COLOR_PAIR(3));
                else if (target >= 65 && target != 95) attroff(COLOR_PAIR(4));

                if (current->row == abs((cli->inpSlot.slot[3] - 48) - 8) && current->col == cli->inpSlot.slot[2] - 65 && cli->inpSlot.at >= 4)
                    attroff(COLOR_PAIR(1));
            }
        }
    }

    attron(COLOR_PAIR(2));
    for (u16 i = 0; i < 8; i++) {
        mvprintw(
            start_y + (i * cell_height),
            start_x + (-1 * cell_width) + (cell_width / 2),
            "%d",
            8 - i
        );

        mvprintw(
            start_y + (8 * cell_height) - (cell_height / 2),
            start_x + (i * cell_width),
            "%c",
            'A' + i
        );
    }
    attroff(COLOR_PAIR(2));

    for (u8 i = 0; i < 3; i++) {
        if (i == cli->select) {
            attron(A_REVERSE);
            mvprintw (
                cli->screen.height - 4, (cli->screen.width / 2) - client_game_default_spacing[i], 
                "%s", client_game_default_options[i]
            );
            attroff(A_REVERSE);
        } else {
            mvprintw (
                cli->screen.height - 4, (cli->screen.width / 2) - client_game_default_spacing[i],
                "%s", client_game_default_options[i]
            );
        }
    }

    move(cli->screen.height - 6, 0); clrtoeol();

    for (u8 i = 0; i < 4; i++) {
        if (cli->inpSlot.at == i) {
            attron(A_REVERSE);
            mvprintw(cli->screen.height - 6, (cli->screen.width / 2) - 5 + client_game_default_input_index[i], " ");
            attroff(A_REVERSE);
        } else
            mvprintw(cli->screen.height - 6, (cli->screen.width / 2) - 5 + client_game_default_input_index[i], "%c", cli->inpSlot.slot[i]);
    }

    if (cli->inpSlot.at >= 2)
        mvprintw(cli->screen.height - 6, (cli->screen.width / 2) - 2, "-->");
}

void client_ui_credit(client* cli) {
    attron(A_BOLD);
    mvprintw((cli->screen.height / 2) - 7, (cli->screen.width / 2) - 3, "CREDITS");
    attroff(A_BOLD);

    mvprintw((cli->screen.height / 2) - 6, (cli->screen.width / 2) - 14, "github.com/itonasd/chess v3.1");
    mvprintw((cli->screen.height / 2) - 4, (cli->screen.width / 2) - 10, "written entirely in c");
    mvprintw((cli->screen.height / 2) - 3, (cli->screen.width / 2) - 15, "Deimos/ncurses | itonasd/ctools");
    mvprintw((cli->screen.height / 2) - 2, (cli->screen.width / 2) - 16, "2024 IS project | built by itonasd");

    attron(A_BOLD);
    mvprintw((cli->screen.height / 2), (cli->screen.width / 2) - 13, "Welcome PCER15 to GC Family");
    attroff(A_BOLD);

    mvprintw((cli->screen.height / 2) + 1, (cli->screen.width / 2) - 21, "play the most common first move in chess :)");

    attron(A_REVERSE);
    mvprintw((cli->screen.height / 2) + 5, (cli->screen.width / 2) - 9, "return to main menu");
    attroff(A_REVERSE);
}

void client_ui_exit(client* cli) {
    attron(A_BOLD);
    mvprintw((cli->screen.height / 2) - 2, (cli->screen.width / 2) - 15, "exiting.. press enter to cancel");
    attroff(A_BOLD);

    attron(A_REVERSE);
    mvprintw(cli->screen.height / 2, (cli->screen.width / 2) - 9, "return to main menu");
    attroff(A_REVERSE);

    cli->exit++;
}

void client_key_bar(client* cli) {
    if (cli->window == WINDOW_GAME_DEFAULT) {
        if (
            cli->input == ARROW_LEFT || cli->input == ARROW_DOWN || cli->input == ARROW_UP ||
            cli->input == ARROW_RIGHT
        )
            cli->naviTrigger = 1;
        else
            cli->naviTrigger = 0;
    } else {
        if (
            cli->input == 'h' || cli->input == 'j' || cli->input == 'k' || cli->input == 'l' ||
            cli->input == ARROW_LEFT || cli->input == ARROW_DOWN || cli->input == ARROW_UP ||
            cli->input == ARROW_RIGHT
        )
            cli->naviTrigger = 1;
        else
            cli->naviTrigger = 0;
    }

    if (cli->input == 'q' ||  cli->input == 27) {
        if (cli->quitTrigger)
            cli->exit = 1;
        else
            cli->quitTrigger = 1;
    } else
        cli->quitTrigger = 0;
}

void client_key_main(client* cli) {
    if (cli->input == 'h' || cli->input == ARROW_LEFT)
        cli->select = (cli->select - 1 + 3) % 3;
    else if (cli->input == 'l' || cli->input == ARROW_RIGHT)
        cli->select = (cli->select + 1) % 3;
    else if (cli->input == '\n')
        switch (cli->select) {
            case 0: cli->window = WINDOW_GAME;   client_clear_main(cli); cli->input = 0; break;
            case 1: cli->window = WINDOW_CREDIT; client_clear_main(cli); cli->input = 0; break;
            case 2: cli->window = WINDOW_EXIT;   client_clear_main(cli); cli->input = 0; break;
        }
}

void client_key_game(client* cli) {
    if (cli->input == 'h' || cli->input == ARROW_LEFT) {
        cli->select = (cli->select - 1 + 3) % 3;
    } else if (cli->input == 'l' || cli->input == ARROW_RIGHT) {
        cli->select = (cli->select + 1) % 3;
    } else if (cli->input == '\n') {
        switch (cli->select) {
            case 0: cli->window = WINDOW_GAME_DEFAULT;  
                client_clear_game(cli); 
                cboard = board_allocate(8, 8); 
                board_setup(cboard, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"); 
                board_update(cboard);
                cli->input = 0; 
                break;

            case 1: cli->window = WINDOW_GAME_ADVANCED; 
                client_clear_game(cli); 
                cli->input = 0; 
                break;

            case 2: cli->window = WINDOW_MAIN;          
                client_clear_game(cli); 
                cli->input = 0; 
                cli->select = 0; 
                break;
        }
    }
}

void client_key_game_default(client* cli) {
    if ( cli->input == ARROW_LEFT) {
        cli->select = (cli->select - 1 + 3) % 3;
    } else if (cli->input == ARROW_RIGHT) {
        cli->select = (cli->select + 1) % 3;
    } else if (cli->input == '\n') {
        switch (cli->select) {
            case 0:
                pos posCur = {
                    abs((cli->inpSlot.slot[1] - 48) - 8), cli->inpSlot.slot[0] - 65
                };

                pos posNxt = {
                    abs((cli->inpSlot.slot[3] - 48) - 8), cli->inpSlot.slot[2] - 65
                };

                board_move(cboard, posCur, posNxt);
                while (cli->inpSlot.at) {
                    cli->inpSlot.slot[cli->inpSlot.at--] = 0;
                }

                board_update(cboard);
                break;

            case 1: 
                board_cleanup(cboard);
                while(cli->inpSlot.at) {
                    cli->inpSlot.slot[cli->inpSlot.at--] = 0;
                } 
                for (u16 i = 0; i < cli->screen.height - 2; i++) {
                    move(i, 0); clrtoeol();
                } 
                cboard = board_allocate(8, 8); 
                board_setup(cboard, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"); 
                board_update(cboard);
                cli->input = 0; 
                break;

            case 2: cli->window = WINDOW_MAIN; 
                board_cleanup(cboard);
                while(cli->inpSlot.at) {
                    cli->inpSlot.slot[cli->inpSlot.at--] = 0;
                } 
                
                for (u16 i = 0; i < cli->screen.height - 2; i++) {
                    move(i, 0); clrtoeol();
                }
                cli->input = 0; 
                cli->select = 0; 
                break;
        }
    }

    if ((cli->input != -1 && (cli->input >= 49 && cli->input <= 56)) && 
    (cli->inpSlot.at == 1 || cli->inpSlot.at == 3))
        cli->inpSlot.slot[cli->inpSlot.at++] = cli->input;

    if ((cli->input != -1 && (toupper(cli->input) >= 'A' && toupper(cli->input) <= 'H')) && (cli->inpSlot.at == 0 || cli->inpSlot.at == 2))
        cli->inpSlot.slot[cli->inpSlot.at++] = toupper(cli->input);

    if (cli->input == 263 && cli->inpSlot.at) {
        cli->inpSlot.slot[cli->inpSlot.at--] = 0;
    }
}

void client_key_credit(client* cli) {
    if (cli->input == '\n') {
        cli->window = WINDOW_MAIN; client_clear_credit(cli);
    }
}

void client_key_exit(client* cli) {
    if (cli->input == '\n') {
        cli->window = WINDOW_MAIN;
        client_clear_main(cli);
        cli->exit = 0;
    }
}

void client_clear_main(client* cli) {
    move((cli->screen.height / 2) - 2, 0);
    clrtoeol();
    move((cli->screen.height / 2), 0);
    clrtoeol();
}

void client_clear_game(client* cli) {
    move(((cli->screen.height / 2) - 4), 0); clrtoeol();
    move(((cli->screen.height / 2) - 2), 0); clrtoeol();
    move(((cli->screen.height / 2) - 1), 0); clrtoeol();
    move(((cli->screen.height / 2) + 2), 0); clrtoeol();
}

void client_clear_credit(client* cli) {
    move(((cli->screen.height / 2) - 7), 0); clrtoeol();
    move(((cli->screen.height / 2) - 6), 0); clrtoeol();
    move(((cli->screen.height / 2) - 4), 0); clrtoeol();
    move(((cli->screen.height / 2) - 3), 0); clrtoeol();
    move(((cli->screen.height / 2) - 2), 0); clrtoeol();
    move(((cli->screen.height / 2)), 0);     clrtoeol();
    move(((cli->screen.height / 2) + 1), 0); clrtoeol();
    move(((cli->screen.height / 2) + 5), 0); clrtoeol();
}