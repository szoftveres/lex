#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "lex.h"


static int     last_char;
static int     flags;
static int     fd;
static int     mode;

static char*   buf;
static int     bufidx;

enum {
    LEX_MODE_FILE,
    LEX_MODE_BUF,
};

static char*  pointer;

char     lexeme[MAX_TOKEN_SIZE];
int      token;

void syntax_error (const char* msg) {
    fprintf(stderr, "syntax error: %s\n", msg);
    exit(1);
}

void pushchar (int c) {
    if (last_char == EOF) {
        last_char = c;
    } else {
        fprintf(stderr, "[%s:%d]: error\n", __FUNCTION__, __LINE__);
        exit (1);
    }
    return;
}

int popchar (void) {
    int c;
    if (last_char != EOF) {
        c = last_char;
        last_char = EOF;
    } else {
        switch (mode) {
          case LEX_MODE_FILE:
            if (!read(fd, &c, 1)) {
                c = EOF;
            }
            break;
          case LEX_MODE_BUF:
            c = buf[bufidx];
            bufidx++;
            break;
          default:
            fprintf(stderr, "[%s:%d]: error\n", __FUNCTION__, __LINE__);
            exit (1);
        }
    }
    return c;
}

/**
*/
int newline (char c) {
    switch (c) {
      case '\n' : case '\r' :
        return (!(flags & LEX_NEWLINE_AS_TOKEN));
    }
    return 0;
}

int wspace (char c) {
    switch (c) {
      case ' ' : case '\t' :
        return 1;
    }
    return (newline(c));
}

int bin (char c) {
    switch (c) {
      case '0' : case '1' :
        return 1;
    }
    return 0;
}

int oct (char c) {
    switch (c) {
      case '2' : case '3' : case '4' : case '5' :
      case '6' : case '7' :
        return 1;
    }
    return bin(c);
}

int dec (char c) {
    switch (c) {
      case '8' : case '9' :
        return 1;
    }
    return oct(c);
}

int hex (char c) {
    switch (c) {
      case 'A' : case 'a' :
      case 'B' : case 'b' :
      case 'C' : case 'c' :
      case 'D' : case 'd' :
      case 'E' : case 'e' :
      case 'F' : case 'f' :
        return 1;
    }
    return dec(c);
}

int alpha (char c) {
    switch (c) {
      case 'a' : case 'b' : case 'c' : case 'd' : case 'e' :
      case 'f' : case 'g' : case 'h' : case 'i' : case 'j' :
      case 'k' : case 'l' : case 'm' : case 'n' : case 'o' :
      case 'p' : case 'q' : case 'r' : case 's' : case 't' :
      case 'u' : case 'v' : case 'w' : case 'x' : case 'y' :
      case 'z' :
      case 'A' : case 'B' : case 'C' : case 'D' : case 'E' :
      case 'F' : case 'G' : case 'H' : case 'I' : case 'J' :
      case 'K' : case 'L' : case 'M' : case 'N' : case 'O' :
      case 'P' : case 'Q' : case 'R' : case 'S' : case 'T' :
      case 'U' : case 'V' : case 'W' : case 'X' : case 'Y' :
      case 'Z' :
      case '_' :
        return 1;
    }
    return 0;
}

/**
*/
int lex (char c) {

    if (c == '\\') {
        switch (token) {
          case T_CHAR_START :
            token = T_CHAR_SPECIAL;
            return 1;
          case T_STRING_START :
          case T_STRING_CONTENT :
            token = T_STRING_SPECIAL;
            return 1;
        }
        syntax_error("stray '\\' in program");
    }

    if (c == '\'') {
        switch (token) {
          case T_NONE :
            token = T_CHAR_START;
            return 1;   /* ok, continue */
          case T_CHAR_CONTENT :
            token = T_CHAR;
            return 1;   /* seems like finished */
        }
    }
    if (c == '\"') {
        switch (token) {
          case T_NONE :
            token = T_STRING_START;
            return 1;   /* ok, continue */
          case T_STRING_CONTENT :
            token = T_STRING;
            return 1;   /* seems like finished */
        }
    }
    /* newline */
    if (newline(c)) {
        switch (token) {
          case T_CHAR_START :
          case T_CHAR_CONTENT :
            syntax_error("missing terminating ' character");
          case T_STRING_START :
          case T_STRING_CONTENT :
            syntax_error("missing terminating \" character");
        }
    }

    /* numerical */
    if (dec(c)) {
        switch (token) {
          case T_INTEGER :
          case T_IDENTIFIER :
            return 1; /* ok, continue */
          case T_LEAD_ZERO :
          case T_OCTAL :
            if (oct(c)) {
                token = T_OCTAL;
                return 1; /* ok, continue */
            } else {
                syntax_error("invalid octal digit");
            }
          case T_BINARY_S :
          case T_BINARY :
            if (bin(c)) {
                token = T_BINARY;
                return 1; /* ok, continue */
            } else {
                syntax_error("invalid binary digit");
            }
          case T_HEXA_S :
          case T_HEXA :
            if (hex(c)) {
                token = T_HEXA;
                return 1; /* ok, continue */
            } else {
                syntax_error("invalid hexadecimal digit");
            }
          case T_NONE :
            if (c == '0') {
                token = T_LEAD_ZERO;
            } else {
                token = T_INTEGER;
            }
            return 1;
        }
    }
    /* alphabetical */
    if (alpha(c)) {
        switch (token) {
          case T_IDENTIFIER:
            return 1; /* ok, continue */
          case T_NONE :
            token = T_IDENTIFIER;
            return 1;
          case T_INTEGER:
            syntax_error("invalid decimal digit");
            break;
          case T_HEXA_S :
          case T_HEXA :
            if (hex(c)) {
                token = T_HEXA;
                return 1; /* ok, continue */
            } else {
                syntax_error("invalid hexadecimal digit");
            }
          case T_LEAD_ZERO :
            if (c == 'x' || c == 'X') {
                token = T_HEXA_S;
                return 1;
            } else if (c == 'b' || c == 'B') {
                token = T_BINARY_S;
                return 1;
            } else {
                syntax_error("invalid character followed by 0");
            }
        }
    }
    /* two-char operators */
    switch (c) {
      case '=' :
        switch (token) {
          case  T_ASSIGN :
            token = T_EQ;          /* == */
            return 1;
          case T_NEG :
            token = T_NEQ;         /* != */
            return 1;
          case T_LESS :
            token = T_LEQ;         /* <= */
            return 1;
          case T_GREATER :
            token = T_GREQ;        /* >= */
            return 1;
          case T_PLUS :
            token = T_RECURADD;    /* += */
            return 1;
          case T_MINUS :
            token = T_RECURSUB;    /* -= */
            return 1;
          case T_MUL :
            token = T_RECURMUL;    /* *= */
            return 1;
          case T_DIV :
            token = T_RECURDIV;    /* /= */
            return 1;
          case T_BWAND :
            token = T_RECURBWAND;  /* &= */
            return 1;
          case T_BWOR :
            token = T_RECURBWOR;   /* |= */
            return 1;
          case T_BWXOR :
            token = T_RECURBWXOR;  /* ^= */
            return 1;
        }
        break;
      case '&' :
        switch (token) {
          case T_BWAND :
            token = T_LAND;        /* && */
            return 1;
        }
        break;
      case '|' :
        switch (token) {
          case T_BWOR :
            token = T_LOR;         /* || */
            return 1;
        }
        break;
      case '<' :
        switch (token) {
          case T_LESS :                 /* << */
            token = T_SLEFT;
            return 1;
        }
        break;
      case '>' :
        switch (token) {
          case T_GREATER :              /* >> */
            token = T_SRIGHT;
            return 1;
		  case T_MINUS :
            token = T_ARROW;            /* -> */
            return 1;
        }
        break;
    }

    switch (token) {
      case T_CHAR_START :
      case T_CHAR_SPECIAL :
        token = T_CHAR_CONTENT;
        return 1;   /* ok, continue */
      case T_STRING_START :
      case T_STRING_SPECIAL :
      case T_STRING_CONTENT :
        token = T_STRING_CONTENT;
        return 1;   /* ok, continue */
      case T_LEAD_ZERO:
        /* convert LEAD ZERO to integer in case it was
         * not followed by anything useful
         */
        token = T_INTEGER;
        break;
      case T_HEXA_S :
      case T_BINARY_S :
        syntax_error("expected digit after non-decimal prefix");
        break;
    }

    if ((token != T_NONE) || wspace(c)) {
        return 0; /* Other token begins, return */
    }

    switch (c) {
      case ';' : token = T_SEMICOLON; return 1;
      case ':' : token = T_COLON; return 1;
      case '?' : token = T_QUESTIONMARK; return 1;
      case '(' : token = T_LEFT_PARENTH; return 1;
      case ')' : token = T_RIGHT_PARENTH; return 1;
      case '[' : token = T_LEFT_SQUARE_BRACKET; return 1;
      case ']' : token = T_RIGHT_SQUARE_BRACKET; return 1;
      case '{' : token = T_LEFT_BRACE; return 1;
      case '}' : token = T_RIGHT_BRACE; return 1;
      case ',' : token = T_COMMA; return 1;
      case '.' : token = T_FULLSTOP; return 1;
      case '+' : token = T_PLUS; return 1;
      case '-' : token = T_MINUS; return 1;
      case '*' : token = T_MUL; return 1;
      case '/' : token = T_DIV; return 1;
      case '%' : token = T_MOD; return 1;
      case '=' : token = T_ASSIGN; return 1;
      case '<' : token = T_LESS; return 1;
      case '>' : token = T_GREATER; return 1;
      case '!' : token = T_NEG; return 1;
      case '@' : token = T_AT; return 1;
      case '&' : token = T_BWAND; return 1;
      case '^' : token = T_BWXOR; return 1;
      case '~' : token = T_BWNEG; return 1;
      case '|' : token = T_BWOR; return 1;
      case '\n' : token = T_NEWLINE; return 1;
      case (char)EOF : token = T_EOF; return 1;
    }
    fprintf(stderr, "syntax error: llegal character '%c' 0x%02x\n", c, c);

    /* not reached */
    return (0);
}

/**
*/
void next_token (void) {
    int c;

    token = T_NONE;
    pointer = lexeme;
    while (1) {
        c = popchar();
        if (!lex((char)c)) {
            if (token != T_NONE) {
                pushchar(c);
                *pointer = '\0';
//                fprintf(stdout, "//      %s   %d \n", lexeme, token);
                return;
            }
            continue; /* It was a trailing whitespace */
        }
        *pointer++ = (char)c;
    }
}


void lex_init_fd (int ifd, int init_flags) {
    mode = LEX_MODE_FILE;
    last_char = EOF;
    flags = init_flags;
    fd = ifd;
    next_token();
}

void lex_init_buf (char* ibuf, int init_flags) {
    mode = LEX_MODE_BUF;
    last_char = EOF;
    flags = init_flags;
    buf = ibuf;
    bufidx = 0;
    next_token();
}



int lex_get (int token_type, const char* str) {
    if (token_type != token) {
        return 0;
    }
    if (str) {
        if (strcmp(str, lexeme)) {
            return 0;
        }
    }
    next_token();
    return 1;
}


void str_process (void) {
    int tp;
    int bp;
    char buf[MAX_TOKEN_SIZE];
    if (token != T_STRING) {
        return;
    }
    tp = 0;
    bp = 0;
    while ((lexeme[tp]) && (tp < MAX_TOKEN_SIZE)) {
        switch (lexeme[tp]) {
          case '\"' :
            break;
          case '\\' :
            tp++;
            switch (lexeme[tp]) {
              case 'n' :
                buf[bp] = '\n';
                break;
              case 'r' :
                buf[bp] = '\r';
                break;
              case 't' :
                buf[bp] = '\t';
                break;
              case '\\' :
                buf[bp] = '\\';
                break;
              case '\"' :
                buf[bp] = '\"';
                break;
              case '\'' :
                buf[bp] = '\'';
                break;
              default :
                syntax_error("illegal esc seq");
                break;
            }
            bp++;
            break;
          default :
            buf[bp] = lexeme[tp];
            bp++;
            break;
        }
        tp++;
    }
    buf[bp] = '\0';
    strcpy(lexeme, buf);
}



int num_process (void) {
    int i;
    int value = 0;

    switch (token) {
      case T_CHAR:                  /* 'a' */
        value = lexeme[1];
        break;
      case T_INTEGER:               /* 1234 */
        for (i = 0; lexeme[i]; i++) {
            value *= 10;
            value += lexeme[i] - '0';
        }
        break;
      case T_OCTAL:                 /* 0123 */
        for (i = 1; lexeme[i]; i++) {
            value *= 8;
            value += lexeme[i] - '0';
        }
        break;
      case T_BINARY:                /* 0b101 */
        for (i = 2; lexeme[i]; i++) {
            value *= 2;
            value += lexeme[i] - '0';
        }
        break;
      case T_HEXA:                  /* 0x1fa */
        for (i = 2; lexeme[i]; i++) {
            value *= 16;
            if (lexeme[i] >= '0' && lexeme[i] <= '9') {
                value += lexeme[i] - '0';
            } else if (lexeme[i] >= 'a' && lexeme[i] <= 'f') {
                value += lexeme[i] + 0xa - 'a';
            } else if (lexeme[i] >= 'A' && lexeme[i] <= 'F') {
                value += lexeme[i] + 0xA - 'A';
            }
        }
        break;
      default:
        return 0;
    }
    return value;
}


