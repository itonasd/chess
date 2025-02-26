import { channel } from 'diagnostics_channel';
import { GameBoard } from './board';
import { Board, data, readOnlyData } from './data';
import { cachedDataVersionTag } from 'v8';

export class Validate {
    queenRookBishop(curIndx: number): Board {
        const stDirIndx: number = (data.board[curIndx].type === 2) ? 4 : 0;
        const enDirIndx: number = (data.board[curIndx].type === 4) ? 4 : 8;
        const enemColor: number = (data.board[curIndx].color === 8) ? 16 : 8;
        let cache: number[] = [];
        
        for (let dirIndx: number = stDirIndx; dirIndx < enDirIndx; dirIndx++) {
            let enemCount: number = 0;
            for (let i: number = 0; i < data.sqToEdge[curIndx][readOnlyData.dirIndx[dirIndx]]; i++) {
                
                const targetSq: number = curIndx + readOnlyData.dirOffsets[dirIndx] * ( i + 1 ); // calculate possible target square
    
                // block by friendly piece
                if (data.board[targetSq].color === data.board[curIndx].color) break;

                // capture enemy piece
                if (data.board[targetSq].color === enemColor) {
                    if (enemCount === 1) break;
                    cache.push(targetSq);
                    enemCount++;
                    break;
                }

                cache.push(targetSq)
            }
        }

        data.board[curIndx] = {
            ...data.board[curIndx],
            ESD_posbPos: cache,
        }

        cache = [];
        return data.board[curIndx];
    }

    king(curIndx: number): Board {
        let cache: number[] = [];
        for (let dirIndx: number = 0; dirIndx < 8; dirIndx++) {

            for (let i: number = 0; i < Math.min(1, data.sqToEdge[curIndx][readOnlyData.dirIndx[dirIndx]]); i++) {
                const targetSq: number = curIndx + readOnlyData.dirOffsets[dirIndx];
                if (data.board[targetSq].color === data.board[curIndx].color) break;

                cache.push(targetSq);
            }
        }

        if (data.board[curIndx].mov === 0) {

            if (data.board[curIndx + 1].color === 0 && data.board[curIndx + 2].color === 0 && data.board[curIndx + 3].mov === 0) cache.push(curIndx + 3);
            if (data.board[curIndx - 1].color === 0 && data.board[curIndx - 2].color === 0 && data.board[curIndx - 3].color === 0 && data.board[curIndx - 4].mov === 0) cache.push(curIndx - 4);
            
        }

        data.board[curIndx] = {
            ...data.board[curIndx],
            ESD_posbPos: cache
        }

        cache = [];
        return data.board[curIndx];
    }

    pawn(curIndx: number): Board {
        const enemColor: number = (data.board[curIndx].color === 8) ? 16 : 8;
        const pawnBehav: number = (!data.board[curIndx].mov) ? 2 : 1;

        let cache: number[] = [];
        let color: number = 0;
        let dirOffsetsPawn: ReadonlyArray<number> | number = 0;;

        if (data.board[curIndx].color === 8) {
            dirOffsetsPawn = readOnlyData.dirOffsetsPawnWhite;
            color = 8;
        }
        if (data.board[curIndx].color === 16) {
            dirOffsetsPawn = readOnlyData.dirOffsetsPawnBlack;
            color = -8;
        }

        for (let i: number = 0; i < pawnBehav; i++) {
            const targetSq: number = curIndx + ( (i + 1) * color );
            if (data.board[targetSq].color === data.board[curIndx].color) break;
            if (data.board[targetSq].color === enemColor) break;

            cache.push(targetSq);
        }

        for (let i: number = 0; i < 2; i++) {

            for (let j: number = 0; j < Math.min(1, data.sqToEdge[curIndx][readOnlyData.dirIndxPawn[i]]); j++) {
                const targetSq: number = curIndx + dirOffsetsPawn[i];
                if (data.board[targetSq].color === data.board[curIndx].color || !data.board[targetSq].color) break;
                
                cache.push(targetSq);
            }
        }

        data.board[curIndx] = {
            ...data.board[curIndx],
            ESD_posbPos: cache
        }

        cache = [];
        return data.board[curIndx];
    }

    knight(curIndx: number): Board {
        const curRow = parseInt(data.board[curIndx].access[0]);
        const curCol = parseInt(data.board[curIndx].access[1]);
        let cache: number[] = [];
    
        for (let i: number = 0; i < 8; i++) {

            const rowOffset = readOnlyData.dirOffsetsKnight[i][0];
            const colOffset = readOnlyData.dirOffsetsKnight[i][1];

            const newRow = curRow + rowOffset;
            const newCol = curCol + colOffset;
    
            if (newRow < 0 || newRow >= 8 || newCol < 0 || newCol >= 8) continue; // corner check
    
            const targetSq = newRow * 8 + newCol;

            if (data.board[targetSq].color === data.board[curIndx].color) continue;

            cache.push(targetSq);
        }

        data.board[curIndx] = {
            ...data.board[curIndx],
            ESD_posbPos: cache
        }

        cache = [];
        return data.board[curIndx];
    }
}