/* GENERATED SOURCE. DO NOT MODIFY. */
// © 2017 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html#License
package ohos.global.icu.impl.number.parse;

import static ohos.global.icu.impl.number.parse.ParsingUtils.safeContains;

import ohos.global.icu.impl.StaticUnicodeSets;
import ohos.global.icu.impl.StringSegment;
import ohos.global.icu.text.DecimalFormatSymbols;

/**
 * @author sffc
 * @hide exposed on OHOS
 *
 */
public class PlusSignMatcher extends SymbolMatcher {

    private static final PlusSignMatcher DEFAULT = new PlusSignMatcher(false);
    private static final PlusSignMatcher DEFAULT_ALLOW_TRAILING = new PlusSignMatcher(true);

    public static PlusSignMatcher getInstance(DecimalFormatSymbols symbols, boolean allowTrailing) {
        String symbolString = symbols.getPlusSignString();
        if (safeContains(DEFAULT.uniSet, symbolString)) {
            return allowTrailing ? DEFAULT_ALLOW_TRAILING : DEFAULT;
        } else {
            return new PlusSignMatcher(symbolString, allowTrailing);
        }
    }

    private final boolean allowTrailing;

    private PlusSignMatcher(String symbolString, boolean allowTrailing) {
        super(symbolString, DEFAULT.uniSet);
        this.allowTrailing = allowTrailing;
    }

    private PlusSignMatcher(boolean allowTrailing) {
        super(StaticUnicodeSets.Key.PLUS_SIGN);
        this.allowTrailing = allowTrailing;
    }

    @Override
    protected boolean isDisabled(ParsedNumber result) {
        return !allowTrailing && result.seenNumber();
    }

    @Override
    protected void accept(StringSegment segment, ParsedNumber result) {
        result.setCharsConsumed(segment);
    }

    @Override
    public String toString() {
        return "<PlusSignMatcher>";
    }

}
