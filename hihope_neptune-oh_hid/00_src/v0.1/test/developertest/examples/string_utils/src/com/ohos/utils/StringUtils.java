/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.ohos.utils;

import java.util.Locale;


/**
 * handle string method
 */
public class StringUtils {
    private static String EMPTY_STRING = "";

    /**
     * change string to lower case
     *
     * @param str input string
     * @return lower case string
     */
    public static String toLowerCase(String str) {
        if (str == null) {
            return EMPTY_STRING;
        }

        return str.toLowerCase(Locale.getDefault());
    }

    /**
     * change string to upper case
     *
     * @param str input string
     * @return upper case string
     */
    public static String toUpperCase(String str) {
        if (str == null) {
            return EMPTY_STRING;
        }

        return str.toUpperCase(Locale.getDefault());
    }

    /**
     * change string to reverse
     *
     * @param str input string
     * @return reverse string
     */
    public static String reverse(String str) {
        if (str == null) {
            return EMPTY_STRING;
        }

        return new StringBuffer(str).reverse().toString();
    }
}
