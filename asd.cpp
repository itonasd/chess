#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <vector>
#include <array>
#include <algorithm>

#include "hashmap_OPTIMISE_THIS_SHIT.h"
#include "utils.h"

using namespace std;

class Data {
public:
    int log_md(int first = 0, int second = 0, int third = 0, int forth = 0, int fifth = 0) {
        if (first == 0) return md.board[second][third][forth][fifth];
        if (first == 1) return md.metadata[second];
        return 0;
    }

protected:
    struct MainData {
        int ****board;
        hashmap_t ***board_data;
        vector<array<int, 2>> ***move_predictions;
        int *metadata;
    };
    
public:
    static MainData md;
};

Data::MainData Data::md = {
    nullptr, nullptr
};

class Board : public Data {
private:
    void allocate(MainData *md, int row, int col) {
        md->board = (int ****) malloc(row * sizeof(int ***));
        md->metadata = (int *) malloc(3 * sizeof(int));

        md->metadata[0] = row;
        md->metadata[1] = col;

        for (int i = 0; i < row; i++) {
            md->board[i] = (int ***) malloc(col * sizeof(int **));

            for (int j = 0; j < col; j++) {
                md->board[i][j] = (int **) malloc(3 * sizeof(int *));
                md->board[i][j][0] = (int *) malloc(3 * sizeof(int));
                md->board[i][j][2] = (int *) malloc(4 * sizeof(int));
            }
        }

        md->board_data = (hashmap_t***) malloc(row * sizeof(hashmap_t**));

        for (int i = 0; i < row; i++) {
            md->board_data[i] = (hashmap_t**) malloc(col * sizeof(hashmap_t*));
            for (int j = 0; j < col; j++) {
                md->board_data[i][j] = (hashmap_t*) malloc(8 * sizeof(hashmap_t));
            }
        }

        md->move_predictions = new vector<array<int, 2>>**[row];

        for (int i = 0 ; i < row; i++) {
            md->move_predictions[i] = new vector<array<int, 2>>*[col];
            for (int j = 0; j < col; j++) {
                md->move_predictions[i][j] = new vector<array<int, 2>>;
            }
        }
    }

    void initalize(MainData *md) {
        int rowa = md->metadata[0];
        int cola = md->metadata[1];
        int row = rowa - 1;

        for (int i = 0; i < rowa; i++) {
            for (int j = 0; j < cola; j++) {
                md->board[row][j][0][0] = (cola * i) + j;
                md->board[row][j][0][1] = 65 + j;
                md->board[i][j][0][2] = row + 1;
            }
            row--;
        }
    }

    void edgecount(MainData *md) {
        for (int i = md->metadata[0] - 1; i >= 0; i--) {
            for (int j = 0; j < md->metadata[1]; j++) {
                int east = (md->metadata[1] - 1) - j;
                int west = j;
                int south = (md->metadata[0] - 1) - i;
                int north = i;

                int cache[8] = {north, south, west, east, mathmin(north, west), mathmin(south, east), mathmin(north, east), mathmin(south, west)};

                for (int k = 0; k < 8; k++) {
                    md->board_data[i][j][k].key = k;
                    md->board_data[i][j][k].val = cache[k];
                }
            }
        }
    }

    void insertpieces(MainData *md, const char *fen) {
        auto chartoint = [](char data) {
            switch (data) {
                case 'p':
                    return 1;
                case 'b':
                    return 2;
                case 'n':
                    return 3;
                case 'r':
                    return 4;
                case 'q':
                    return 5;
                case 'k':
                    return 6;
                default:
                    return 0;
            };
        };

        int row = md->metadata[0] - 1, col = 0;

        for (int i = 0; i < md->metadata[0]; i++) {
            for (int j = 0; j < md->metadata[1]; j++) {
                md->board[i][j][2][0] = 0;
                md->board[i][j][2][1] = 0;
                md->board[i][j][2][2] = 0;
            }
        }
        for (int i = 0; i < strlen(fen); i++) {
            if (fen[i] == '/') {
                row--;
                col = 0;
            } else {
                if (isdigit(fen[i])) {
                    col += fen[i] - '0';
                } else {
                    md->board[row][col][2][0] = fen[i];
                    md->board[row][col][2][1] = chartoint(tolower(fen[i]));
                    md->board[row][col][2][2] = (tolower(fen[i]) == fen[i]) ? 8 : 16;

                    col++;
                }
            }
        }
    }

    void deleteboard(MainData *md) {
        int row = md->metadata[0];
        int col = md->metadata[1];

        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                free(md->board[i][j][0]);
                free(md->board[i][j][2]);
                free(md->board[i][j]);

                free(md->board_data[i][j]);
                delete md->move_predictions[i][j]; 
            }
            free(md->board[i]);
            
            free(md->board_data[i]);
            delete[] md->move_predictions[i]; 
        }
        free(md->board);
        free(md->metadata);

        free(md->board_data);
        delete[] md->move_predictions;
    }

    int boardState = -1;

public:

    int interface(const char* type, int row = 8, int col = 8) {
        if (strcmp(type, "delete") == 0 && boardState == -1) {
            printf("cannot delete void\n");
            return -1;
        }

        if (strcmp(type, "delete") == 0 && boardState == 0) {
            deleteboard(&md);
            boardState = -1;
            printf("board deleted\n");
            return 0;
        }

        if (boardState == 0) {
            printf("cannot override board\n");
            return -1;
        }

        if (row < 1 || col < 1) {
            printf("cannot initialize zero or negative board\n");
            return -1;
        }
        if (strcmp(type, "default") == 0) type = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

        allocate(&md, row, col);
        initalize(&md);
        edgecount(&md);
        insertpieces(&md, type);
        boardState = 0;
        printf("board initalized\n");

        return 0;
    }

    ~Board() {
        deleteboard(&md);
        printf("board collapsed\n");
    }
};

class Validate : public Data { // ESSENTIAL PART, OPTIMISATION REQUIRED ESPECIALLY ON HASHMAP.H LIB
public:
    Validate() {
        printf("engine constructed\n");
    }

protected:
    hashmap_t directions[8] = {
        {0, md.metadata[0]}, {1, -(md.metadata[0])}, {2, -1}, {3, 1}, {4, (md.metadata[0] - 1)}, 
        {5, -(md.metadata[0] - 1)}, {6, (md.metadata[0] + 1)}, {7, -(md.metadata[0] + 1)}
    };

    vector<array<int, 2>> qrb(int row, int col) {
        int **curPiece = md.board[row][col];
        int stDirIndx = (curPiece[2][1] == 2) ? 4 : 0; // bishop 4, 0
        int enDirIndx = (curPiece[2][1] == 4) ? 4 : 8; // rook 4, 8
        int enemColor = (curPiece[2][2] == 8) ? 16 : 8;
        int allyColor = (curPiece[2][2] == 8) ? 8 : 16;
        vector<array<int, 2>> cache;
        
        for (int dirIndx = stDirIndx; dirIndx < enDirIndx; dirIndx++) {
            
            for (int i = 0; i < hashmap_pairs(directions, md.board_data[row][col], 8, dirIndx); i++) {
                int targetrow = row_revert((curPiece[0][0] + directions[dirIndx].val * (i + 1)) / md.metadata[0], md.metadata[0]);
                
                int targetcol = (curPiece[0][0] + directions[dirIndx].val * (i + 1)) % md.metadata[1];

                if (md.board[targetrow][targetcol][2][2] == allyColor) break;
                
                array<int, 2> vcache = { targetrow, targetcol };
                cache.push_back(vcache);

                if (md.board[targetrow][targetcol][2][2] == enemColor) break;
            }
        }

        return cache;
    }

    vector<array<int, 2>> king(int row, int col) {
        int **curPiece = md.board[row][col];
        int allyColor = (curPiece[2][2] == 8) ? 8 : 16;
        vector<array<int, 2>> cache;
        
        for (int dirIndx = 0; dirIndx < 8; dirIndx++) {
            if (hashmap_pairs(directions, md.board_data[row][col], 8, dirIndx) > 0) {
                int targetrow = row_revert((curPiece[0][0] + directions[dirIndx].val) / md.metadata[0], md.metadata[0]);
                int targetcol = (curPiece[0][0] + directions[dirIndx].val) % md.metadata[1];
                if (md.board[targetrow][targetcol][2][2] == allyColor) continue;
                
                array<int, 2> vcache = { targetrow, targetcol };
                cache.push_back(vcache);
            }
        }

        // castling

        return cache;
    }

    vector<array<int, 2>> knight(int row, int col) {
        int **curPiece = md.board[row][col];
        int allyColor = (curPiece[2][2] == 8) ? 8 : 16;
        vector<array<int, 2>> cache;
        array<array<int, 2>, 8> knightDirections = {{
            { -2, -1 }, { -1, -2 }, { 1, -2 }, {2, -1}, {2, 1}, {1, 2}, {-1, 2}, {-2, 1}
        }};

        for (int i = 0; i < 8; i++) {
            int rowOffsets = knightDirections[i][0];
            int colOffsets = knightDirections[i][1];

            int targetrow = row + rowOffsets;
            int targetcol = col + colOffsets;

            if (targetrow < 0 || targetrow >= md.metadata[0] || targetcol < 0 || targetcol >= md.metadata[1]) continue;
            if (md.board[targetrow][targetcol][2][2] == allyColor) continue;

            array<int, 2> vcache = { targetrow, targetcol };
            cache.push_back(vcache);
        }

        return cache;
    }

    vector<array<int, 2>> pawn(int row, int col) {
        int **curPiece = md.board[row][col];
        int enemColor = (curPiece[2][2] == 8) ? 16 : 8;
        int allyColor = (curPiece[2][2] == 8) ? 8 : 16;
        int diagnMove = (allyColor == 8) ? -1 : 1;
        int pawnBehav = ((row == 6 && allyColor == 8) || (row == 1 && allyColor == 16)) ? 2 : 1;
        array<array<int, 2>, 2> dirOffsets = (allyColor == 8) ? 
        array<array<int, 2>, 2>{{ {-1, 1}, {-1, -1} }} : array<array<int, 2>, 2>{{ {1, 1}, {1, -1} }};
        vector<array<int, 2>> cache;

        for (int i = 0; i < pawnBehav; i++) {
            int targetrow = row + ((i + 1) * diagnMove);
            
            
            if (targetrow < 0 || targetrow >= md.metadata[0]) break;
            if (md.board[targetrow][col][2][2] == enemColor || md.board[targetrow][col][2][2] == allyColor) break;
            
            array<int, 2> vcache = { targetrow, col };
            cache.push_back(vcache);
        }

        for (int i = 0; i < 2; i++) {
            int targetrow = row + dirOffsets[i][0];
            int targetcol = col + dirOffsets[i][1];
            
            if (targetrow < 0 || targetrow >= md.metadata[0] || targetcol < 0 || targetcol >= md.metadata[1]) continue;
            if (md.board[targetrow][targetcol][2][2] != enemColor) continue;

            array<int, 2> vcache = { targetrow, targetcol };
            cache.push_back(vcache);
        }

        return cache;
    }

};

class Move : private Validate {
public:
    Move() {
        printf("move constructed\n");
    }

    void generate() {
        for (int i = 0; i < md.metadata[0]; i++) {
            for (int j = 0; j < md.metadata[1]; j++) {
                if (!md.move_predictions[i][j][0].empty()) md.move_predictions[i][j][0].clear();
                switch (md.board[i][j][2][1]) {
                    case 1:
                        md.move_predictions[i][j][0] = pawn(i, j);
                        break;
                    case 2:
                        md.move_predictions[i][j][0] = qrb(i, j);
                        break;
                    case 3:
                        md.move_predictions[i][j][0] = knight(i, j);
                        break;
                    case 4:
                        md.move_predictions[i][j][0] = qrb(i, j);
                        break;
                    case 5:
                        md.move_predictions[i][j][0] = qrb(i, j);
                        break;
                    case 6:
                        md.move_predictions[i][j][0] = king(i, j);
                        break;
                }
            }
        }
    }

    void moveData(int row, int col, int targetrow, int targetcol) {
        printf("data shall be moved\n"); // section 2
        md.board[targetrow][targetcol][2][0] = md.board[row][col][2][0];
        md.board[targetrow][targetcol][2][1] = md.board[row][col][2][1];
        md.board[targetrow][targetcol][2][2] = md.board[row][col][2][2];

        md.board[row][col][2][0] = 0;
        md.board[row][col][2][1] = 0;
        md.board[row][col][2][2] = 0;
    }

    void move(int curIndx, int nxtIndx) {
        int row = row_revert(curIndx / md.metadata[0], md.metadata[0]);
        int col = curIndx % md.metadata[1];

        int targetrow = row_revert(nxtIndx / md.metadata[0], md.metadata[0]);
        int targetcol = nxtIndx % md.metadata[1];

        generate();
        if (row < 0 || row >= md.metadata[0] || col < 0 || col >= md.metadata[1] || curIndx == nxtIndx) return;
        if (md.move_predictions[row][col][0].empty()) return;

        array<int, 2> target = { targetrow, targetcol };
        auto iterator = find(md.move_predictions[row][col][0].begin(), md.move_predictions[row][col][0].end(), target);

        if (iterator != md.move_predictions[row][col][0].end()) moveData(row, col, targetrow, targetcol);
    }


};


int main() {
    Board board;
    
    board.interface("rnbqkbnr/pppppppp/3P4/8/8/8/PPPPPPPP/RNBQKBNR", 8, 8);
    printf("size: %d(row) x %d(col)\n\n", board.log_md(1, 0), board.log_md(1, 1));

    for (int i = 0; i < board.log_md(1, 0); i++) {
        for (int j = 0; j < board.log_md(1, 1); j++) {
            printf("[%2d:%c%d | %c:%2d:%2d] ", board.log_md(0, i, j, 0, 0), board.log_md(0, i, j, 0, 1), board.log_md(0, i, j, 0, 2), board.log_md(0, i, j, 2, 0), board.log_md(0, i, j, 2, 1), board.log_md(0, i, j, 2, 2));
        }
        printf("\n\n");
    }


    Move eng;

    eng.generate();

    for (int i = 0; i < board.md.metadata[0]; i++) {
        for (int j = board.md.metadata[1] - 1; j >= 0; j--) {
            printf("%2d -> ", board.md.board[i][j][0][0]);
            if (board.md.move_predictions[i][j][0].empty()) printf("-");
            for (const auto &arr : board.md.move_predictions[i][j][0]) {
                printf("%2d ",board.md.board[arr[0]][arr[1]][0][0]);
            }
            printf("\n");
        }
    }

    eng.move(1, 18);

    for (int i = 0; i < board.log_md(1, 0); i++) {
        for (int j = 0; j < board.log_md(1, 1); j++) {
            printf("[%2d:%c%d | %c:%2d:%2d] ", board.log_md(0, i, j, 0, 0), board.log_md(0, i, j, 0, 1), board.log_md(0, i, j, 0, 2), board.log_md(0, i, j, 2, 0), board.log_md(0, i, j, 2, 1), board.log_md(0, i, j, 2, 2));
        }
        printf("\n\n");
    }

    eng.generate();

    for (int i = 0; i < board.md.metadata[0]; i++) {
        for (int j = board.md.metadata[1] - 1; j >= 0; j--) {
            printf("%2d -> ", board.md.board[i][j][0][0]);
            if (board.md.move_predictions[i][j][0].empty()) printf("-");
            for (const auto &arr : board.md.move_predictions[i][j][0]) {
                printf("%2d ",board.md.board[arr[0]][arr[1]][0][0]);
            }
            printf("\n");
        }
    }
    
    return 0;
}
