package nl.bigo.pcreparser;

/**
 * A small demo class that demonstrates how to use the
 * generated parser classes.
 */
public class Main {

    public static void main(String[] args) throws Exception {

        String regex = "((.)\\1+ (?<YEAR>(?:19|20)\\d{2})) [^]-x]";

        System.out.println(new Builder.Tree(regex).toStringASCII());
    }
}
