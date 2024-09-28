namespace Game::Colors {
    enum Color : char { BLACK = false, WHITE = true };
    static const Color BothColors[]{BLACK, WHITE};

} // namespace Game::Colors

namespace Game::Pieces {
    enum Piece : char { PAWNS, KNIGHTS, BISHOPS, ROOKS, QUEENS, KINGS, NONE };
    static const Piece AllPieces[]{PAWNS, KNIGHTS, BISHOPS,
                                   ROOKS, QUEENS,  KINGS};

    Piece char_to_piece(char c);

    char piece_to_char(Piece piece, Colors::Color color);

} // namespace Game::Pieces
