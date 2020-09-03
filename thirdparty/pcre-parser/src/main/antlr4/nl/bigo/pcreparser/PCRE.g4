/*
 * Copyright (c) 2014 by Bart Kiers
 *
 * The MIT license.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Project      : PCRE Parser, an ANTLR 4 grammar for PCRE
 * Developed by : Bart Kiers, bart@big-o.nl
 */
grammar PCRE;

// Most single line comments above the lexer- and  parser rules 
// are copied from the official PCRE man pages (last updated: 
// 10 January 2012): http://www.pcre.org/pcre.txt
parse
 : alternation EOF
 ;

// ALTERNATION
//
//         expr|expr|expr...
alternation
 : expr (Pipe expr)*
 ;

expr
 : (atom quantifier?)*
 ;

// QUANTIFIERS
//
//         ?           0 or 1, greedy
//         ?+          0 or 1, possessive
//         ??          0 or 1, lazy
//         *           0 or more, greedy
//         *+          0 or more, possessive
//         *?          0 or more, lazy
//         +           1 or more, greedy
//         ++          1 or more, possessive
//         +?          1 or more, lazy
//         {n}         exactly n
//         {n,m}       at least n, no more than m, greedy
//         {n,m}+      at least n, no more than m, possessive
//         {n,m}?      at least n, no more than m, lazy
//         {n,}        n or more, greedy
//         {n,}+       n or more, possessive
//         {n,}?       n or more, lazy
quantifier
 : QuestionMark quantifier_type
 | Plus quantifier_type
 | Star quantifier_type
 | OpenBrace number CloseBrace quantifier_type
 | OpenBrace number Comma CloseBrace quantifier_type
 | OpenBrace number Comma number CloseBrace quantifier_type
 ;

quantifier_type
 : Plus
 | QuestionMark
 | /* nothing */
 ;

// CHARACTER CLASSES
//
//         [...]       positive character class
//         [^...]      negative character class
//         [x-y]       range (can be used for hex characters)
//         [[:xxx:]]   positive POSIX named set
//         [[:^xxx:]]  negative POSIX named set
//
//         alnum       alphanumeric
//         alpha       alphabetic
//         ascii       0-127
//         blank       space or tab
//         cntrl       control character
//         digit       decimal digit
//         graph       printing, excluding space
//         lower       lower case letter
//         print       printing, including space
//         punct       printing, excluding alphanumeric
//         space       white space
//         upper       upper case letter
//         word        same as \w
//         xdigit      hexadecimal digit
//
//       In PCRE, POSIX character set names recognize only ASCII  characters  by
//       default,  but  some  of them use Unicode properties if PCRE_UCP is set.
//       You can use \Q...\E inside a character class.
character_class
 : CharacterClassStart Caret CharacterClassEnd Hyphen cc_atom+ CharacterClassEnd
 | CharacterClassStart Caret CharacterClassEnd cc_atom* CharacterClassEnd
 | CharacterClassStart Caret cc_atom+ CharacterClassEnd
 | CharacterClassStart CharacterClassEnd Hyphen cc_atom+ CharacterClassEnd
 | CharacterClassStart CharacterClassEnd cc_atom* CharacterClassEnd
 | CharacterClassStart cc_atom+ CharacterClassEnd
 ;

// BACKREFERENCES
//
//         \n              reference by number (can be ambiguous)
//         \gn             reference by number
//         \g{n}           reference by number
//         \g{-n}          relative reference by number
//         \k<name>        reference by name (Perl)
//         \k'name'        reference by name (Perl)
//         \g{name}        reference by name (Perl)
//         \k{name}        reference by name (.NET)
//         (?P=name)       reference by name (Python)
backreference
 : backreference_or_octal
 | SubroutineOrNamedReferenceStart number
 | SubroutineOrNamedReferenceStart OpenBrace number CloseBrace
 | SubroutineOrNamedReferenceStart OpenBrace Hyphen number CloseBrace
 | NamedReferenceStartK LessThan name GreaterThan
 | NamedReferenceStartK SingleQuote name SingleQuote
 | NamedReferenceStartK OpenBrace name CloseBrace
 | NamedReferenceStartK OpenBrace name CloseBrace
 | OpenBackreference name CloseParen
 ;

backreference_or_octal
 : octal_char
 | Backslash digit
 ;

// CAPTURING
//
//         (...)           capturing group
//         (?<name>...)    named capturing group (Perl)
//         (?'name'...)    named capturing group (Perl)
//         (?P<name>...)   named capturing group (Python)
//         (?:...)         non-capturing group
//         (?|...)         non-capturing group; reset group numbers for
//                          capturing groups in each alternative
//
// ATOMIC GROUPS
//
//         (?>...)         atomic, non-capturing group
capture
 : OpenParen QuestionMark LessThan name GreaterThan alternation CloseParen
 | OpenParen QuestionMark SingleQuote name SingleQuote alternation CloseParen
 | OpenCapture name GreaterThan alternation CloseParen
 | OpenParen alternation CloseParen
 ;

non_capture
 : OpenParen QuestionMark Colon alternation CloseParen
 | OpenParen QuestionMark Pipe alternation CloseParen
 | OpenParen QuestionMark GreaterThan alternation CloseParen
 ;

// COMMENT
//
//         (?#....)        comment (not nestable)
comment
 : OpenParen QuestionMark Hash non_close_parens CloseParen
 ;

// OPTION SETTING
//
//         (?i)            caseless
//         (?J)            allow duplicate names
//         (?m)            multiline
//         (?s)            single line (dotall)
//         (?U)            default ungreedy (lazy)
//         (?x)            extended (ignore white space)
//         (?-...)         unset option(s)
//
//       The following are recognized only at the start of a  pattern  or  after
//       one of the newline-setting options with similar syntax:
//
//         (*NO_START_OPT) no start-match optimization (PCRE_NO_START_OPTIMIZE)
//         (*UTF8)         set UTF-8 mode: 8-bit library (PCRE_UTF8)
//         (*UTF16)        set UTF-16 mode: 16-bit library (PCRE_UTF16)
//         (*UCP)          set PCRE_UCP (use Unicode properties for \d etc)
option
 : Option_1
 | Option_2
 | Option_3
 | Option_4
 | Option_5
 | Option_6
 | Option_7
 ;

// LOOKAHEAD AND LOOKBEHIND ASSERTIONS
//
//         (?=...)         positive look ahead
//         (?!...)         negative look ahead
//         (?<=...)        positive look behind
//         (?<!...)        negative look behind
//
//       Each top-level branch of a look behind must be of a fixed length.
look_around
 : OpenParen QuestionMark Equals alternation CloseParen
 | OpenParen QuestionMark Exclamation alternation CloseParen
 | OpenParen QuestionMark LessThan Equals alternation CloseParen
 | OpenParen QuestionMark LessThan Exclamation alternation CloseParen
 ;

// SUBROUTINE REFERENCES (POSSIBLY RECURSIVE)
//
//         (?R)            recurse whole pattern
//         (?n)            call subpattern by absolute number
//         (?+n)           call subpattern by relative number
//         (?-n)           call subpattern by relative number
//         (?&name)        call subpattern by name (Perl)
//         (?P>name)       call subpattern by name (Python)
//         \g<name>        call subpattern by name (Oniguruma)
//         \g'name'        call subpattern by name (Oniguruma)
//         \g<n>           call subpattern by absolute number (Oniguruma)
//         \gnLC           call subpattern by absolute number (Oniguruma)
//         \g<+n>          call subpattern by relative number (PCRE extension)
//         \g'+n'          call subpattern by relative number (PCRE extension)
//         \g<-n>          call subpattern by relative number (PCRE extension)
//         \g'-n'          call subpattern by relative number (PCRE extension)
subroutine_reference
 : SubRoutine_1
 | OpenParen QuestionMark number CloseParen
 | OpenParen QuestionMark Plus number CloseParen
 | OpenParen QuestionMark Hyphen number CloseParen
 | OpenParen QuestionMark Ampersand name CloseParen
 | OpenSubRoutine_2 name CloseParen
 | SubroutineOrNamedReferenceStart LessThan name GreaterThan
 | SubroutineOrNamedReferenceStart SingleQuote name SingleQuote
 | SubroutineOrNamedReferenceStart LessThan number GreaterThan
 | SubroutineOrNamedReferenceStart SingleQuote number SingleQuote
 | SubroutineOrNamedReferenceStart LessThan Plus number GreaterThan
 | SubroutineOrNamedReferenceStart SingleQuote Plus number SingleQuote
 | SubroutineOrNamedReferenceStart LessThan Hyphen number GreaterThan
 | SubroutineOrNamedReferenceStart SingleQuote Hyphen number SingleQuote
 ;

// CONDITIONAL PATTERNS
//
//         (?(condition)yes-pattern)
//         (?(condition)yes-pattern|no-pattern)
//
//         (?(n)...        absolute reference condition
//         (?(+n)...       relative reference condition
//         (?(-n)...       relative reference condition
//         (?(<name>)...   named reference condition (Perl)
//         (?('name')...   named reference condition (Perl)
//         (?(name)...     named reference condition (PCRE)
//         (?(R)...        overall recursion condition
//         (?(Rn)...       specific group recursion condition
//         (?(R&name)...   specific recursion condition
//         (?(DEFINE)...   define subpattern for reference
//         (?(assert)...   assertion condition
conditional
 : OpenParen QuestionMark OpenParen number CloseParen alternation (Pipe alternation)? CloseParen
 | OpenParen QuestionMark OpenParen Plus number CloseParen alternation (Pipe alternation)? CloseParen
 | OpenParen QuestionMark OpenParen Hyphen number CloseParen alternation (Pipe alternation)? CloseParen
 | OpenParen QuestionMark OpenParen LessThan name GreaterThan CloseParen alternation (Pipe alternation)? CloseParen
 | OpenParen QuestionMark OpenParen SingleQuote name SingleQuote CloseParen alternation (Pipe alternation)? CloseParen
 | OpenConditional_1 number CloseParen alternation (Pipe alternation)? CloseParen
 | OpenConditional_1 CloseParen alternation (Pipe alternation)? CloseParen
 | OpenConditional_1 Ampersand name CloseParen alternation (Pipe alternation)? CloseParen
 | OpenParen OpenConditional_2 alternation (Pipe alternation)? CloseParen
 | OpenParen OpenConditional_3 alternation (Pipe alternation)? CloseParen
 | OpenParen QuestionMark OpenParen name CloseParen alternation (Pipe alternation)? CloseParen
 ;

// BACKTRACKING CONTROL
//
//       The following act immediately they are reached:
//
//         (*ACCEPT)       force successful match
//         (*FAIL)         force backtrack; synonym (*F)
//         (*MARK:NAME)    set name to be passed back; synonym (*:NAME)
//
//       The  following  act only when a subsequent match failure causes a back-
//       track to reach them. They all force a match failure, but they differ in
//       what happens afterwards. Those that advance the start-of-match point do
//       so only if the pattern is not anchored.
//
//         (*COMMIT)       overall failure, no advance of starting point
//         (*PRUNE)        advance to next starting character
//         (*PRUNE:NAME)   equivalent to (*MARK:NAME)(*PRUNE)
//         (*SKIP)         advance to current matching position
//         (*SKIP:NAME)    advance to position corresponding to an earlier
//                         (*MARK:NAME); if not found, the (*SKIP) is ignored
//         (*THEN)         local failure, backtrack to next alternation
//         (*THEN:NAME)    equivalent to (*MARK:NAME)(*THEN)
backtrack_control
 : BackTrack_1
 | BackTrack_2
 | BackTrack_3
 | BackTrack_4
 | BackTrack_5
 | BackTrack_6
 | BackTrack_7
 | BackTrack_8
 | BackTrack_9
 | BackTrack_10
 | BackTrack_11
 | BackTrack_12
 ;

// NEWLINE CONVENTIONS
//capture
//       These are recognized only at the very start of the pattern or  after  a
//       (*BSR_...), (*UTF8), (*UTF16) or (*UCP) option.
//
//         (*CR)           carriage return only
//         (*LF)           linefeed only
//         (*CRLF)         carriage return followed by linefeed
//         (*ANYCRLF)      all three of the above
//         (*ANY)          any Unicode newline sequence
//
// WHAT \R MATCHES
//
//       These  are  recognized only at the very start of the pattern or after a
//       (*...) option that sets the newline convention or a UTF or UCP mode.
//
//         (*BSR_ANYCRLF)  CR, LF, or CRLF
//         (*BSR_UNICODE)  any Unicode newline sequence
newline_convention
 : NewLine_1
 | NewLine_2
 | NewLine_3
 | NewLine_4
 | NewLine_5
 | NewLine_6
 | NewLine_7
 ;

// CALLOUTS
//
//         (?C)      callout
//         (?Cn)     callout with data n
callout
 : OpenCallout CloseParen
 | OpenCallout number CloseParen
 ;

atom
 : subroutine_reference
 | shared_atom
 | literal
 | character_class
 | capture
 | non_capture
 | comment
 | option
 | look_around
 | backreference
 | conditional
 | backtrack_control
 | newline_convention
 | callout
 | ( Dot
 | Caret
 | StartOfSubject
 | WordBoundary
 | NonWordBoundary
 | EndOfSubjectOrLine
 | EndOfSubjectOrLineEndOfSubject
 | EndOfSubject
 | PreviousMatchInSubject
 | ResetStartMatch
 | OneDataUnit
 | ExtendedUnicodeChar )
 ;

cc_atom
 : cc_literal Hyphen cc_literal
 | shared_atom
 | cc_literal
 | backreference_or_octal // only octal is valid in a cc
 ;

shared_atom
 : POSIXNamedSet
 | POSIXNegatedNamedSet
 | ControlChar
 | DecimalDigit
 | NotDecimalDigit
 | HorizontalWhiteSpace
 | NotHorizontalWhiteSpace
 | NotNewLine
 | CharWithProperty
 | CharWithoutProperty
 | NewLineSequence
 | WhiteSpace
 | NotWhiteSpace
 | VerticalWhiteSpace
 | NotVerticalWhiteSpace
 | WordChar
 | NotWordChar 
 ;

literal
 : octal_char
 | digit
 | ( Letter
 |  BellChar
 | EscapeChar
 | FormFeed
 | NewLine
 | CarriageReturn
 | Tab
 | HexChar
 | Quoted
 | BlockQuoted
 | OpenBrace
 | CloseBrace
 | Comma
 | Hyphen
 | LessThan
 | GreaterThan
 | SingleQuote
 | Underscore
 | Colon
 | Hash
 | Equals
 | Exclamation
 | Ampersand
 | OtherChar
 | CharacterClassEnd )
 ;

cc_literal
 : octal_char
 | Letter
 | digit
 | ( BellChar
 | EscapeChar
 | FormFeed
 | NewLine
 | CarriageReturn
 | Tab
 | HexChar
 | Quoted
 | BlockQuoted
 | OpenBrace
 | CloseBrace
 | Comma
 | Hyphen
 | LessThan
 | GreaterThan
 | SingleQuote
 | Underscore
 | Colon
 | Hash
 | Equals
 | Exclamation
 | Ampersand
 | OtherChar
 | Dot
 | CharacterClassStart
 | Caret
 | QuestionMark
 | Plus
 | Star
 | WordBoundary
 | EndOfSubjectOrLine
 | Pipe
 | OpenParen
 | CloseParen )
 ;

number
 : digits
 ;

octal_char
 : ( Backslash (D0 | D1 | D2 | D3) octal_digit octal_digit
   | Backslash octal_digit octal_digit                     
   )

 ;

octal_digit
 : D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7
 ;
 
digits
 : digit+
 ;

digit
 : D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7 | D8 | D9
 ;

name
 : ( Letter | Underscore) ( ( Letter | Underscore ) | digit)*
 ;

Letter : Alphabetical ;

non_close_parens
 : non_close_paren+
 ;

non_close_paren
 : ~CloseParen
 ;

// QUOTING
//
//         \x         where x is non-alphanumeric is a literal x
//         \Q...\E    treat enclosed characters as literal
Quoted      : '\\' NonAlphaNumeric;
BlockQuoted : '\\Q' .*? '\\E';

// CHARACTERS
//
//         \a         alarm, that is, the BEL character (hex 07)
//         \cx        "control-x", where x is any ASCII character
//         \e         escape (hex 1B)
//         \f         form feed (hex 0C)
//         \n         newline (hex 0A)
//         \r         carriage return (hex 0D)
//         \t         tab (hex 09)
//         \ddd       character with octal code ddd, or backreference
//         \xhh       character with hex code hh
//         \x{hhh..}  character with hex code hhh..
BellChar       : '\\a';
ControlChar    : '\\c' ASCII;
EscapeChar     : '\\e';
FormFeed       : '\\f';
NewLine        : '\\n';
CarriageReturn : '\\r';
Tab            : '\\t';
Backslash      : '\\';
HexChar        : '\\x' ( HexDigit HexDigit
                       | OpenBrace HexDigit HexDigit HexDigit+ CloseBrace
                       )
               ;

// CHARACTER TYPES
//
//         .          any character except newline;
//                      in dotall mode, any character whatsoever
//         \C         one data unit, even in UTF mode (best avoided)
//         \d         a decimal digit
//         \D         a character that is not a decimal digit
//         \h         a horizontal white space character
//         \H         a character that is not a horizontal white space character
//         \N         a character that is not a newline
//         \p{xx}     a character with the xx property
//         \P{xx}     a character without the xx property
//         \R         a newline sequence
//         \s         a white space character
//         \S         a character that is not a white space character
//         \v         a vertical white space character
//         \V         a character that is not a vertical white space character
//         \w         a "word" character
//         \W         a "non-word" character
//         \X         an extended Unicode sequence
//
//       In  PCRE,  by  default, \d, \D, \s, \S, \w, and \W recognize only ASCII
//       characters, even in a UTF mode. However, this can be changed by setting
//       the PCRE_UCP option.
Dot                     : '.';
OneDataUnit             : '\\C';
DecimalDigit            : '\\d';
NotDecimalDigit         : '\\D';
HorizontalWhiteSpace    : '\\h';
NotHorizontalWhiteSpace : '\\H';
NotNewLine              : '\\N';
CharWithProperty        : '\\p{' UnderscoreAlphaNumerics CloseBrace;
CharWithoutProperty     : '\\P{' UnderscoreAlphaNumerics CloseBrace;
NewLineSequence         : '\\R';
WhiteSpace              : '\\s';
NotWhiteSpace           : '\\S';
VerticalWhiteSpace      : '\\v';
NotVerticalWhiteSpace   : '\\V';
WordChar                : '\\w';
NotWordChar             : '\\W';
ExtendedUnicodeChar     : '\\X';

// CHARACTER CLASSES
//
//         [...]       positive character class
//         [^...]      negative character class
//         [x-y]       range (can be used for hex characters)
//         [[:xxx:]]   positive POSIX named set
//         [[:^xxx:]]  negative POSIX named set
//
//         alnum       alphanumeric
//         alpha       alphabetic
//         ascii       0-127
//         blank       space or tab
//         cntrl       control character
//         digit       decimal digit
//         graph       printing, excluding space
//         lower       lower case letter
//         print       printing, including space
//         punct       printing, excluding alphanumeric
//         space       white space
//         upper       upper case letter
//         word        same as \w
//         xdigit      hexadecimal digit
//
//       In PCRE, POSIX character set names recognize only ASCII  characters  by
//       default,  but  some  of them use Unicode properties if PCRE_UCP is set.
//       You can use \Q...\E inside a character class.
CharacterClassStart  : '[';
CharacterClassEnd    : ']';
Caret                : '^';
Hyphen               : '-';
POSIXNamedSet        : '[[:' AlphaNumerics ':]]';
POSIXNegatedNamedSet : '[[:^' AlphaNumerics ':]]';

QuestionMark : '?';
Plus         : '+';
Star         : '*';
OpenBrace    : '{';
CloseBrace   : '}';
Comma        : ',';

// ANCHORS AND SIMPLE ASSERTIONS
//
//         \b          word boundary
//         \B          not a word boundary
//         ^           start of subject
//                      also after internal newline in multiline mode
//         \A          start of subject
//         $           end of subject
//                      also before newline at end of subject
//                      also before internal newline in multiline mode
//         \Z          end of subject
//                      also before newline at end of subject
//         \z          end of subject
//         \G          first matching position in subject
WordBoundary                   : '\\b';
NonWordBoundary                : '\\B';
StartOfSubject                 : '\\A'; 
EndOfSubjectOrLine             : '$';
EndOfSubjectOrLineEndOfSubject : '\\Z'; 
EndOfSubject                   : '\\z'; 
PreviousMatchInSubject         : '\\G';

// MATCH POINT RESET
//
//         \K          reset start of match
ResetStartMatch : '\\K';

SubroutineOrNamedReferenceStart : '\\g';
NamedReferenceStartK             : '\\k';

Pipe        : '|';
OpenParen   : '(';
CloseParen  : ')';
LessThan    : '<';
GreaterThan : '>';
SingleQuote : '\'';
Underscore  : '_';
Colon       : ':';
Hash        : '#';
Equals      : '=';
Exclamation : '!';
Ampersand   : '&';

D1 : '1';
D2 : '2';
D3 : '3';
D4 : '4';
D5 : '5';
D6 : '6';
D7 : '7';
D8 : '8';
D9 : '9';
D0 : '0';

OtherChar : . ;

// specific lexer rules to avoid ambiguity with [a-zA-Z]
OpenCapture : '(?P<' ;
OpenBackreference : '(?P=' ;
fragment Option_flags
 : Option_flag+
 ;
fragment Option_flag : 'i' | 'J' | 'm' | 's' | 'U' | 'x' ;
Option_1 : '(?' Option_flags '-' Option_flags ')' ;
Option_2 : '(?' Option_flags ')' ;
Option_3 : '(?_' Option_flags ')' ;
Option_4 : '(*NO_START_OPT)' ;
Option_5 : '(*UTF8)' ;
Option_6 : '(*UTF16)' ;
Option_7 : '(*UCP)' ;
SubRoutine_1 : '(?R)' ;
OpenSubRoutine_2 : '(?P>' ;
OpenConditional_1 : '(?(<R' ;
OpenConditional_2 : '?(DEFINE)' ;
OpenConditional_3 : '?(ASSERT)' ;
BackTrack_1 : '(*ACCEPT)' ;
BackTrack_2 : '(*F)' ;
BackTrack_3 : '(*FAIL)' ;
BackTrack_4 : '(*MARK:NAME)' ;
BackTrack_5 : '(*:NAME)' ;
BackTrack_6 : '(*COMMIT)' ;
BackTrack_7 : '(*PRUNE)' ;
BackTrack_8 : '(*:PRUNE:NAME)' ;
BackTrack_9 : '(*:SKIP)' ;
BackTrack_10 : '(*SKIP:NAME)' ;
BackTrack_11 : '(*THEN)' ;
BackTrack_12 : '(*THEN:NAME)' ;
NewLine_1 : '(*CR)' ;
NewLine_2 : '(*LF)' ;
NewLine_3 : '(*CRLF)' ;
NewLine_4 : '(*ANYCRLF)' ;
NewLine_5 : '(*ANY)' ;
NewLine_6 : '(*BSR_ANYCRLF)' ;
NewLine_7 : '(*BSR_UNICODE)' ;
OpenCallout : '(?C' ;

// fragments
fragment UnderscoreAlphaNumerics : ('_' | AlphaNumeric)+;
fragment AlphaNumerics           : AlphaNumeric+;
fragment AlphaNumeric            : [a-zA-Z0-9];
fragment NonAlphaNumeric         : ~[a-zA-Z0-9];
fragment HexDigit                : [0-9a-fA-F];
fragment ASCII                   : [\u0000-\u007F];
fragment Alphabetical            : [A-Za-z];