/* GENERATED SOURCE. DO NOT MODIFY. */
// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
package ohos.global.icu.impl.number.parse;

import ohos.global.icu.impl.StringSegment;
import ohos.global.icu.text.DecimalFormatSymbols;
import ohos.global.icu.text.UnicodeSet;

/**
 * @author sffc
 * @hide exposed on OHOS
 *
 */
public class NanMatcher extends SymbolMatcher {

    private static final NanMatcher DEFAULT = new NanMatcher("NaN");

    public static NanMatcher getInstance(DecimalFormatSymbols symbols, int parseFlags) {
        String symbolString = symbols.getNaN();
        if (DEFAULT.string.equals(symbolString)) {
            return DEFAULT;
        } else {
            return new NanMatcher(symbolString);
        }
    }

    private NanMatcher(String symbolString) {
        super(symbolString, UnicodeSet.EMPTY);
    }

    @Override
    protected boolean isDisabled(ParsedNumber result) {
        return result.seenNumber();
    }

    @Override
    protected void accept(StringSegment segment, ParsedNumber result) {
        result.flags |= ParsedNumber.FLAG_NAN;
        result.setCharsConsumed(segment);
    }

    @Override
    public String toString() {
        return "<NanMatcher>";
    }

}
