#include "../include/expression_tree.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static Token get_next_token(FILE* stream, TreeStatus* status) {
    int c;
    while ((c = fgetc(stream)) != EOF && isspace(c) && c != '\n') {}
    bool ungot = false;
    if (c == EOF || c == '\n') {
        return (Token){.type = TOK_END};
    }

    Token tok = {0};

    if (isdigit(c) || c == '.') {
        char buffer[64];
        int i = 0;
        buffer[i++] = (char)c;

        while (i < 63 && (c = fgetc(stream)) != EOF &&
               (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-')) {
            if ((c == '+' || c == '-') && (toupper(buffer[i-1]) != 'E')) {

                ungetc(c, stream);
                ungot = true;
                break;
            }
            buffer[i++] = (char)c;
        }
        if (c != EOF && !isdigit(c) && c != '.' && toupper(c) != 'E') {
            if (!ungot) {
                ungetc(c, stream);
                ungot = true;
            }
        }
        buffer[i] = '\0';

        char* endptr;
        tok.data.value = strtod(buffer, &endptr);

        if (endptr == buffer) {
            *status = TREE_ERROR_SYNTAX_NUM_TOO_LONG;
        } else {
            tok.type = TOK_CONST;
            if (*endptr != '\0') {
                *status = TREE_ERROR_SYNTAX_NUM_TOO_LONG;
            }
        }
        return tok;
    }

    if (isalpha(c)) {
        int i = 0;
        tok.data.var[i++] = (char)c;
        while ((c = fgetc(stream)) != EOF && isalnum(c)) {
            if (i < 7) tok.data.var[i++] = (char)c;
            else {
                *status = TREE_ERROR_SYNTAX_VAR_TOO_LONG;
                while ((c = fgetc(stream)) != EOF && isalnum(c)) {}
                if (c != EOF) ungetc(c, stream);

                return tok;
            }
        }
        if (c != EOF && isalnum(c)) {
            while ((c = fgetc(stream)) != EOF && isalnum(c));
        }
        if (c != EOF) ungetc(c, stream);


        tok.data.var[i] = '\0';
        tok.type = TOK_VAR;
        return tok;
    }

    switch (c) {
        case '(': tok.type = TOK_LPAREN; break;
        case ')': tok.type = TOK_RPAREN; break;
        case '+': case '-': case '*': case '/': case '^':
            tok.type = TOK_OP;
            tok.data.op = (char)c;
            break;
        default:
            *status = TREE_ERROR_SYNTAX_OP_ORDER;
            break;
    }

    return tok;
}


Vec(Token) tokenize_stream(FILE* stream, TreeStatus* status) {
    Vec(Token) tokens = NULL;
    TokenType last_type = TOK_END;
    *status = TREE_OK;

    while (true) {
        Token tok = get_next_token(stream, status);
        if (*status != TREE_OK) goto fail;
        if (tok.type == TOK_END) break;

        if (tok.type == TOK_OP && (tok.data.op == '-' || tok.data.op == '+')) {
            if (last_type == TOK_END || last_type == TOK_OP || last_type == TOK_LPAREN) {
                tok.is_unary = true;
            }
        }

        if (tok.type == TOK_CONST || tok.type == TOK_VAR || tok.type == TOK_LPAREN) {
            if (last_type == TOK_CONST || last_type == TOK_VAR || last_type == TOK_RPAREN) {
                *status = TREE_ERROR_SYNTAX_OP_ORDER;
                goto fail;
            }
        }
        else if (tok.type == TOK_OP) {
            if (!tok.is_unary) {
                if (last_type == TOK_OP || last_type == TOK_LPAREN || last_type == TOK_END) {
                    *status = TREE_ERROR_SYNTAX_OP_ORDER;
                    goto fail;
                }
            }
            else {
                if (last_type == TOK_CONST || last_type == TOK_VAR || last_type == TOK_RPAREN) {
                    *status = TREE_ERROR_SYNTAX_OP_ORDER;
                    goto fail;
                }
            }
        }
        else if (tok.type == TOK_RPAREN) {
            if (last_type == TOK_OP || last_type == TOK_LPAREN || last_type == TOK_END) {
                *status = TREE_ERROR_SYNTAX_OP_ORDER;
                goto fail;
            }
        }

        if (v_push(tokens, tok) != CVEC_SUCCESS) {
            *status = TREE_ERROR_MEM;
            goto fail;
        }

        last_type = tok.type;
    }

    if (last_type == TOK_OP) {
        *status = TREE_ERROR_SYNTAX_OP_ORDER;
        goto fail;
    }

    return tokens;

fail:
    v_free(tokens);
    return NULL;
}