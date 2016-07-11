/*
 *    Python Doom Tools
 *    Copyright (C) 2016 Alexey Lysiuk
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

grammar UDMF;

udmf
    :   globalExpression*
    ;

globalExpression
    :   blockExpression
    |   assignmentExpression
    ;

blockExpression
    :   Identifier '{' assignmentExpression* '}'
    ;

assignmentExpression
    :   Identifier '=' value ';'
    ;

value
    :   Boolean
    |   Integer
    |   Float
    |   QuotedString
    |   Keyword
    ;

Boolean
    :   [Tt][Rr][Uu][Ee]
    |   [Ff][Aa][Ll][Ss][Ee]
    ;

Identifier
    :   [A-Za-z_]+[A-Za-z0-9_]*
    ;

Integer
    :   [+-]?[1-9]+[0-9]*
    |   '0'[0-9]+
    |   '0x'[0-9A-Fa-f]+
    ;

Float
    :   [+-]?[0-9]+'.'[0-9]*([eE][+-]?[0-9]+)?
    ;

QuotedString
    :   '"'~["\\]*('\\'.~["\\]*)*'"'
    ;

Keyword
    :   ~[{}();"'/=\r\n\t ]+
    ;

WhiteSpace
    :   [ \t\r\n]+
        -> skip
    ;

BlockComment
    :   '/*' .*? '*/'
        -> skip
    ;

LineComment
    :   '//' ~[\r\n]*
        -> skip
    ;
