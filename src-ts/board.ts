import { data, Pieces, Board, SquareToEdge } from './data';

export class GameBoard {

    constructor() {
        this.clear();
    }

    init(fen: string): Board[] {
        
        const pieces: {p: number, b: number, n: number, r: number, q: number, k: number} = {
            'p': Pieces.Pawn, 'b': Pieces.Bishop, 'n': Pieces.Knight,
            'r': Pieces.Rook, 'q': Pieces.Queen, 'k': Pieces.King
        }

        if (fen === 'default') fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

        if (fen === 'shuffled') {
            const cache = this.shuffle();
            fen = `${cache[0]}/pppppppp/8/8/8/8/PPPPPPPP/${cache[1]}`;
        }

        const fenArr: string[] = fen.split('');
        let row: number = 7, col: number = 0;
    
        for (let i: number = 0; i < fenArr.length; i++) {
    
            if (fenArr[i] === '/') {
                row--; col = 0;

            } else {

                if (/^[0-9]$/.test(fenArr[i])) {
                    col += parseInt(fenArr[i], 10);

                } else {

                    const type: number = pieces[fenArr[i].toLowerCase()];
                    const color: number = (fenArr[i].toUpperCase() === fenArr[i]) ? Pieces.White : Pieces.Black;
                    const indx: number = row * 8 + col;
    
                    data.board[indx] = {
                        ...data.board[indx],
                        type: type,
                        color: color,
                    }
    
                    col++;
                }
            }
        }
    
        return data.board;
    }

    dirInit(): SquareToEdge[] {

        for (let row: number = 0; row < 8; row++) {

            for (let col: number = 0; col < 8; col++) {
    
                const north: number = 7 - col;
                const south: number = col;
                const west: number = row;
                const east: number = 7 - row;
    
                const index: number = col * 8 + row;
    
                data.sqToEdge[index] = {
                    ...data.sqToEdge[index],
    
                    NDir: north,
                    SDir: south,
                    WDir: west,
                    EDir: east,
                    NWDir: Math.min(north, west),
                    SEDir: Math.min(south, east),
                    NEDir: Math.min(north, east),
                    SWDir: Math.min(south, west),
                }
            }
        }
    
        return data.sqToEdge;
    }

    private shuffle(): string[] {

        const st: string[] = 'rnbqkbnr'.split('');
        let res: string[] = [];

        for (let i = st.length - 1; i > 0; i--) {
            const rn = Math.floor(Math.random() * (i + 1));

            const tmp = st[i];
            st[i] = st[rn];
            st[rn] = tmp;

        }

        res.push(st.join(''));
        res.push(st.join('').toUpperCase());

        return res;
    }

    clear(): Board[] {
        data.sqToEdge = [];
        data.board = [];

        data.board = new Array(64).fill(undefined).map((placeholder, i: number) => {
            const row = Math.floor(i / 8);
            const col = i % 8;
    
            return {
                type: 0,
                color: 0,
                mov: 0,
        
                indx: i,
                access: `${row}${col}`,

                ESD_posbPos: undefined
            }
        })

        data.sqToEdge = new Array(64).fill(undefined).map((placeholder, i: number) => {
            const row = Math.floor(i / 8);
            const col = i % 8;
        
            return {
                NDir: 0,
                SDir: 0,
                WDir: 0,
                EDir: 0,
                NWDir: 0,
                NEDir: 0,
                SWDir: 0,
                SEDir: 0,
        
                indx: i,
                access: `${row}${col}`
            }
        })
        
        return data.board;
    }
}