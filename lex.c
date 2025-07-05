#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "lex.h"




void pushchar (lex_instance_t *instance, int c) {
    if (instance->last_char == EOF) {
        instance->last_char = c;
    } else {
        instance->error("lex: double-push");
    }
    return;
}

int popchar (lex_instance_t *instance) {
    int c;
    if (instance->last_char != EOF) {
        c = instance->last_char;
        instance->last_char = EOF;
    } else {
        if (!instance->read_byte(&c)) {
            c = EOF;
        }
    }
    return c;
}

/**
*/
int newline (lex_instance_t *instance, char c) {
    switch (c) {
      case '\n' : case '\r' :
        return (!(instance->flags & LEX_NEWLINE_AS_TOKEN));
    }
    return 0;
}

int wspace (lex_instance_t *instance, char c) {
    switch (c) {
      case ' ' : case '\t' :
        return 1;
    }
    return (newline(instance, c));
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
int lex (lex_instance_t *instance, char c) {

    if (c == '\\') {
        switch (instance->token) {
          case T_CHAR_START :
            instance->token = T_CHAR_SPECIAL;
            return 1;
          case T_STRING_START :
          case T_STRING_CONTENT :
            instance->token = T_STRING_SPECIAL;
            return 1;
        }
        instance->error("stray '\\' in program");
        instance->token = T_ERROR;
        return 0;
    }

    if (c == '\'') {
        switch (instance->token) {
          case T_NONE :
            instance->token = T_CHAR_START;
            return 1;   /* ok, continue */
          case T_CHAR_CONTENT :
            instance->token = T_CHAR;
            return 1;   /* seems like finished */
        }
    }
    if (c == '\"') {
        switch (instance->token) {
          case T_NONE :
            instance->token = T_STRING_START;
            return 1;   /* ok, continue */
          case T_STRING_CONTENT :
            instance->token = T_STRING;
            return 1;   /* seems like finished */
        }
    }
    /* newline */
    if (newline(instance, c)) {
        switch (instance->token) {
          case T_CHAR_START :
          case T_CHAR_CONTENT :
            instance->error("missing terminating ' character");
            instance->token = T_ERROR;
            return 0;
          case T_STRING_START :
          case T_STRING_CONTENT :
            instance->error("missing terminating \" character");
            instance->token = T_ERROR;
            return 0;
        }
    }

    /* numerical */
    if (dec(c)) {
        switch (instance->token) {
          case T_INTEGER :
          case T_IDENTIFIER :
            return 1; /* ok, continue */
          case T_LEAD_ZERO :
          case T_OCTAL :
            if (oct(c)) {
                instance->token = T_OCTAL;
                return 1; /* ok, continue */
            } else {
                instance->error("invalid octal digit");
                instance->token = T_ERROR;
                return 0;
            }
          case T_BINARY_S :
          case T_BINARY :
            if (bin(c)) {
                instance->token = T_BINARY;
                return 1; /* ok, continue */
            } else {
                instance->error("invalid binary digit");
                instance->token = T_ERROR;
                return 0;
            }
          case T_HEXA_S :
          case T_HEXA :
            if (hex(c)) {
                instance->token = T_HEXA;
                return 1; /* ok, continue */
            } else {
                instance->error("invalid hexadecimal digit");
                instance->token = T_ERROR;
                return 0;
            }
          case T_NONE :
            if (c == '0') {
                instance->token = T_LEAD_ZERO;
            } else {
                instance->token = T_INTEGER;
            }
            return 1;
        }
    }
    /* alphabetical */
    if (alpha(c)) {
        switch (instance->token) {
          case T_IDENTIFIER:
            return 1; /* ok, continue */
          case T_NONE :
            instance->token = T_IDENTIFIER;
            return 1;
          case T_INTEGER:
            instance->error("invalid decimal digit");
            instance->token = T_ERROR;
            return 0;
          case T_HEXA_S :
          case T_HEXA :
            if (hex(c)) {
                instance->token = T_HEXA;
                return 1; /* ok, continue */
            } else {
                instance->error("invalid hexadecimal digit");
                instance->token = T_ERROR;
                return 0;
            }
          case T_LEAD_ZERO :
            if (c == 'x' || c == 'X') {
                instance->token = T_HEXA_S;
                return 1;
            } else if (c == 'b' || c == 'B') {
                instance->token = T_BINARY_S;
                return 1;
            } else {
                instance->error("invalid character followed by 0");
                instance->token = T_ERROR;
                return 0;
            }
        }
    }
    /* two-char operators */
    switch (c) {
      case '=' :
        switch (instance->token) {
          case  T_ASSIGN :
            instance->token = T_EQ;          /* == */
            return 1;
          case T_NEG :
            instance->token = T_NEQ;         /* != */
            return 1;
          case T_LESS :
            instance->token = T_LEQ;         /* <= */
            return 1;
          case T_GREATER :
            instance->token = T_GREQ;        /* >= */
            return 1;
          case T_PLUS :
            instance->token = T_RECURADD;    /* += */
            return 1;
          case T_MINUS :
            instance->token = T_RECURSUB;    /* -= */
            return 1;
          case T_MUL :
            instance->token = T_RECURMUL;    /* *= */
            return 1;
          case T_DIV :
            instance->token = T_RECURDIV;    /* /= */
            return 1;
          case T_BWAND :
            instance->token = T_RECURBWAND;  /* &= */
            return 1;
          case T_BWOR :
            instance->token = T_RECURBWOR;   /* |= */
            return 1;
          case T_BWXOR :
            instance->token = T_RECURBWXOR;  /* ^= */
            return 1;
        }
        break;
      case '&' :
        switch (instance->token) {
          case T_BWAND :
            instance->token = T_LAND;        /* && */
            return 1;
        }
        break;
      case '|' :
        switch (instance->token) {
          case T_BWOR :
            instance->token = T_LOR;         /* || */
            return 1;
        }
        break;
      case '<' :
        switch (instance->token) {
          case T_LESS :                 /* << */
            instance->token = T_SLEFT;
            return 1;
        }
        break;
      case '>' :
        switch (instance->token) {
          case T_GREATER :              /* >> */
            instance->token = T_SRIGHT;
            return 1;
		  case T_MINUS :
            instance->token = T_ARROW;            /* -> */
            return 1;
        }
        break;
    }

    switch (instance->token) {
      case T_CHAR_START :
      case T_CHAR_SPECIAL :
        instance->token = T_CHAR_CONTENT;
        return 1;   /* ok, continue */
      case T_STRING_START :
      case T_STRING_SPECIAL :
      case T_STRING_CONTENT :
        instance->token = T_STRING_CONTENT;
        return 1;   /* ok, continue */
      case T_LEAD_ZERO:
        /* convert LEAD ZERO to integer in case it was
         * not followed by anything useful
         */
        instance->token = T_INTEGER;
        break;
      case T_HEXA_S :
      case T_BINARY_S :
        instance->error("expected digit after prefix");
        instance->token = T_ERROR;
        return 0;
    }

    if ((instance->token != T_NONE) || wspace(instance, c)) {
        return 0; /* Other token begins, return */
    }

    switch (c) {
      case ';' : instance->token = T_SEMICOLON; return 1;
      case ':' : instance->token = T_COLON; return 1;
      case '?' : instance->token = T_QUESTIONMARK; return 1;
      case '(' : instance->token = T_LEFT_PARENTH; return 1;
      case ')' : instance->token = T_RIGHT_PARENTH; return 1;
      case '[' : instance->token = T_LEFT_SQUARE_BRACKET; return 1;
      case ']' : instance->token = T_RIGHT_SQUARE_BRACKET; return 1;
      case '{' : instance->token = T_LEFT_BRACE; return 1;
      case '}' : instance->token = T_RIGHT_BRACE; return 1;
      case ',' : instance->token = T_COMMA; return 1;
      case '.' : instance->token = T_FULLSTOP; return 1;
      case '+' : instance->token = T_PLUS; return 1;
      case '-' : instance->token = T_MINUS; return 1;
      case '*' : instance->token = T_MUL; return 1;
      case '/' : instance->token = T_DIV; return 1;
      case '%' : instance->token = T_MOD; return 1;
      case '=' : instance->token = T_ASSIGN; return 1;
      case '<' : instance->token = T_LESS; return 1;
      case '>' : instance->token = T_GREATER; return 1;
      case '!' : instance->token = T_NEG; return 1;
      case '@' : instance->token = T_AT; return 1;
      case '&' : instance->token = T_BWAND; return 1;
      case '^' : instance->token = T_BWXOR; return 1;
      case '~' : instance->token = T_BWNEG; return 1;
      case '|' : instance->token = T_BWOR; return 1;
      case '\n' : instance->token = T_NEWLINE; return 1;
      case (char)EOF : instance->token = T_EOF; return 1;
    }

    instance->error("Illegal character");
    instance->token = T_ERROR;
    return (0);
}

/**
*/
void next_token (lex_instance_t* instance) {
    int c;

    instance->token = T_NONE;
    instance->pointer = instance->lexeme;
    while (1) {
        c = popchar(instance);
        if (!lex(instance, (char)c)) {
            if (instance->token != T_NONE) {
                pushchar(instance, c);
                *(instance->pointer) = '\0';
                return;
            }
            continue; /* It was a trailing whitespace */
        }
        *(instance->pointer++) = (char)c;
    }
}


void lex_reset (lex_instance_t *instance) {
    instance->last_char = EOF;
    next_token(instance);
}


lex_instance_t* lex_init (int lexeme_size,
                          int (*read_byte)(int*),
                          void (*error) (const char*),
                          int init_flags) {

    lex_instance_t* instance;

    instance = (lex_instance_t*)malloc(sizeof(lex_instance_t));
    if (!instance) {
        return (instance);
    }
    instance->lexeme = (char*)malloc(lexeme_size);
    if (!instance->lexeme) {
        return NULL;
    }
    instance->flags = init_flags;
    instance->read_byte = read_byte;
    instance->error = error;
    instance->lexeme_size = lexeme_size;
    lex_reset(instance);

    return instance;
}

int lex_get (lex_instance_t *instance, int token_type, const char* str) {
    if (token_type != instance->token) {
        return 0;
    }
    if (str) {
        if (strcmp(str, instance->lexeme)) {
            return 0;
        }
    }
    next_token(instance);
    return 1;
}


void str_process (lex_instance_t *instance) {
    int tp;
    int bp;
    char *buf = (char*)malloc(instance->lexeme_size);
    if (instance->token != T_STRING) {
        return;
    }
    if (!buf) {
        instance->error("malloc error");
        instance->token = T_ERROR;
        return;
    }
    tp = 0;
    bp = 0;
    while ((instance->lexeme[tp]) && (tp < instance->lexeme_size)) {
        switch (instance->lexeme[tp]) {
          case '\"' :
            break;
          case '\\' :
            tp++;
            switch (instance->lexeme[tp]) {
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
                instance->error("illegal esc seq");
                instance->token = T_ERROR;
                break;
            }
            bp++;
            break;
          default :
            buf[bp] = instance->lexeme[tp];
            bp++;
            break;
        }
        tp++;
    }
    buf[bp] = '\0';
    strcpy(instance->lexeme, buf);
    free(buf);
}



int num_process (lex_instance_t *instance) {
    int i;
    int value = 0;

    switch (instance->token) {
      case T_CHAR:                  /* 'a' */
        value = instance->lexeme[1];
        break;
      case T_INTEGER:               /* 1234 */
        for (i = 0; instance->lexeme[i]; i++) {
            value *= 10;
            value += instance->lexeme[i] - '0';
        }
        break;
      case T_OCTAL:                 /* 0123 */
        for (i = 1; instance->lexeme[i]; i++) {
            value *= 8;
            value += instance->lexeme[i] - '0';
        }
        break;
      case T_BINARY:                /* 0b101 */
        for (i = 2; instance->lexeme[i]; i++) {
            value *= 2;
            value += instance->lexeme[i] - '0';
        }
        break;
      case T_HEXA:                  /* 0x1fa */
        for (i = 2; instance->lexeme[i]; i++) {
            value *= 16;
            if (instance->lexeme[i] >= '0' && instance->lexeme[i] <= '9') {
                value += instance->lexeme[i] - '0';
            } else if (instance->lexeme[i] >= 'a' && instance->lexeme[i] <= 'f') {
                value += instance->lexeme[i] + 0xa - 'a';
            } else if (instance->lexeme[i] >= 'A' && instance->lexeme[i] <= 'F') {
                value += instance->lexeme[i] + 0xA - 'A';
            }
        }
        break;
      default:
        instance->error("unknown numeric format");
        instance->token = T_ERROR;
    }
    return value;
}


