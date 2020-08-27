# PCRE parser

An ANTLR 4 grammar and parser for PCRE (Perl Compatible Regular Expressions).

To generate the parser classes, do a `mvn clean install` and then do something
like this:

```java
import nl.bigo.pcreparser.*;

// ...

String regex = "((.)\\1+ (?<YEAR>(?:19|20)\\d{2})) [^]-x]";

PCRELexer lexer = new PCRELexer(new ANTLRInputStream(regex));
PCREParser parser = new PCREParser(new CommonTokenStream(lexer));
ParseTree tree = parser.parse();

// ...
```

To print an ASCII tree of the generated parse tree, see the [`Main` class](https://github.com/bkiers/pcre-grammar/blob/master/src/main/java/nl/bigo/pcreparser/Main.java).


