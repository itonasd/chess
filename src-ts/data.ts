export type Board = {
    type: number,
    color: number,

    mov: number,
    indx: number,
    access: string

    ESD_posbPos: number[] | undefined
}

export type SquareToEdge = {
    NDir: number,
    SDir: number,
    WDir: number,
    EDir: number,
    NWDir: number,
    NEDir: number,
    SWDir: number,
    SEDir: number, 

    indx: number,
    access: string
}

export type ReadOnlyData = {
    dirIndx: ReadonlyArray<string>, 
    dirIndxPawn: ReadonlyArray<string>,
    dirOffsets: ReadonlyArray<number>, 
    dirOffsetsKnight: ReadonlyArray<[number, number]>,
    dirOffsetsPawnWhite: ReadonlyArray<number>,
    dirOffsetsPawnBlack: ReadonlyArray<number>
}

interface ClientData {
    gameStatus: { isGameStarted: boolean; currentTurn?: number; gameTime?: number }

    gameConfig?: { 
        boardType: string; isUndoAllowed: boolean; timeControl?: boolean;
        time?: { limit: number; increment?: number }
    }

    profiles?: {
        white: { score?: number; move?: number; totalTime?: number; timeRemaining?: number };
        black: { score?: number; move?: number; totalTime?: number; timeRemaining?: number };
    }
}

/////////////////////////////////////////////////////////////

export const clientData: ClientData = {
    gameStatus: {
        isGameStarted: false
    }
}

export class Pieces {
    static Pawn = 1;
    static Bishop = 2;
    static Knight = 3;
    static Rook = 4;
    static Queen = 5;
    static King = 6;

    static White = 8;
    static Black = 16;
}

export const data: { board: Board[], sqToEdge: SquareToEdge[], prevPos: Board[][] } = {
    board: [],
    sqToEdge: [],
    prevPos: [],
}

export const readOnlyData: ReadOnlyData = {
    dirOffsetsKnight: [
        [-2, -1], [-1, -2], [1, -2], [2, -1], 
        [2, 1], [1, 2], [-1, 2], [-2, 1]
    ],

    dirOffsets: [
        8, -8, -1, 1, 7, -7, 9, -9
    ],

    dirOffsetsPawnWhite: [
        7, 9
    ],

    dirOffsetsPawnBlack: [
        -7, -9
    ],
    
    dirIndx: [
        'NDir',
        'SDir',
        'WDir',
        'EDir',
        'NWDir',
        'SEDir',
        'NEDir',
        'SWDir',
    ],

    dirIndxPawn: [
        'NWDir',
        'NEDir'
    ]
}
