# Lexical Analyzer
Universal, platform-independent lexical analyzer

```
    T_NONE,
    T_EOF,
    T_ERROR,
    T_NEWLINE,
    T_BACKSLASH,                /* \ */
    T_IDENTIFIER,               /* [a-zA-Z_][a-zA-Z0-9_] */
    T_INTEGER,                  /* [0-9] */
    T_LEAD_ZERO,                /* 0 */
    T_BINARY_S,                 /* 0b, 0B */
    T_BINARY,                   /* 0b[01]*, 0B[01]* */
    T_HEXA_S,                   /* 0x  0X */
    T_HEXA,                     /* 0x[0-fF], 0X[0-fF] */
    T_OCTAL,                    /* 00 */

    T_CHAR_START,               /* ' */
    T_CHAR_CONTENT,             /* '. */
    T_CHAR_SPECIAL,             /* \n \t */
    T_CHAR,                     /* '.' */

    T_STRING_START,             /* " */
    T_STRING_CONTENT,           /* ".* */
    T_STRING_SPECIAL,           /* \n \t */
    T_STRING,                   /* ".*" */

    T_AT,                       /* @ */
    T_SEMICOLON,                /* ; */
    T_COLON,                    /* : */
    T_QUESTIONMARK,             /* ? */
    T_LEFT_PARENTH,             /* ( */
    T_RIGHT_PARENTH,            /* ) */
    T_LEFT_SQUARE_BRACKET,      /* [ */
    T_RIGHT_SQUARE_BRACKET,     /* ] */
    T_LEFT_BRACE,               /* { */
    T_RIGHT_BRACE,              /* } */
    T_COMMA,                    /* , */
    T_FULLSTOP,                 /* . */
    T_ASSIGN,                   /* = */
    T_NEG,                      /* ! */
    T_LESS,                     /* < */
    T_GREATER,                  /* > */
    T_PLUS,                     /* + */
    T_MINUS,                    /* - */
    T_MUL,                      /* * */
    T_DIV,                      /* / */
    T_MOD,                      /* % */
    T_EQ,                       /* == */
    T_NEQ,                      /* != */
    T_LEQ,                      /* <= */
    T_GREQ,                     /* >= */

    T_SLEFT,                    /* << */
    T_SRIGHT,                   /* >> */

    T_ARROW,                    /* -> */

    T_RECURADD,                 /* += */
    T_RECURSUB,                 /* -= */
    T_RECURMUL,                 /* *= */
    T_RECURDIV,                 /* /= */
    T_RECURBWOR,                /* |= */
    T_RECURBWAND,               /* &= */
    T_RECURBWXOR,               /* ^= */

    T_BWXOR,                    /* ^ */
    T_BWNEG,                    /* ~ */
    T_BWAND,                    /* & */
    T_LAND,                     /* && */
    T_BWOR,                     /* | */
    T_LOR,                      /* || */
```
