/*
 * Copyright (c) 2007 Hyperic, Inc.
 *
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

package org.hyperic.sigar.win32;

public class LocaleInfo extends Win32 {

    /**
     * English name of language
     */
    public static final int LOCALE_SENGLANGUAGE = 0x00001001;

    /**
     * English name of country
     */
    public static final int LOCALE_SENGCOUNTRY = 0x00001002;

    /**
     * English primary language id
     */
    public static final int LANG_ENGLISH = 0x09;

    private int id;

    private static native int getSystemDefaultLCID();

    private static native String getAttribute(int id, int attr);

    public LocaleInfo() {
        this(getSystemDefaultLCID());
    }

    public static final int makeLangId(int primary, int sub) {
        return (sub << 10) | primary;
    }

    public LocaleInfo(Integer id) {
        this(id.intValue());
    }

    public LocaleInfo(int id) {
        this.id = id;
    }

    public LocaleInfo(int primary, int sub) {
        this(makeLangId(primary, sub));
    }

    public int getId() {
        return this.id;
    }

    public void setId(int id) {
        this.id = id;
    }

    private static int getPrimaryLangId(int id) {
        return id & 0x3ff;
    }

    public int getPrimaryLangId() {
        return getPrimaryLangId(this.id);
    }

    private static int getSubLangId(int id) {
        return id >> 10;
    }

    public int getSubLangId() {
        return getSubLangId(this.id);
    }

    public static boolean isEnglish() {
        int id = getSystemDefaultLCID();
        return getPrimaryLangId(id) == LANG_ENGLISH;
    }

    public String getPerflibLangId() {
        String id =
            Integer.toHexString(getPrimaryLangId()).toUpperCase();

        //length always == 3
        int pad = 3 - id.length();
        StringBuffer fid = new StringBuffer(3);
        while (pad-- > 0) {
            fid.append("0");
        }
        fid.append(id);

        return fid.toString();
    }

    public String getAttribute(int attr) {
        return getAttribute(this.id, attr);
    }

    public String getEnglishLanguageName() {
        return getAttribute(LOCALE_SENGLANGUAGE);
    }

    public String getEnglishCountryName() {
        return getAttribute(LOCALE_SENGCOUNTRY);
    }

    public String toString() {
        return
            getId() + ":" +
            getEnglishLanguageName() +
            " (" + getEnglishCountryName() + ")";
    }
}
