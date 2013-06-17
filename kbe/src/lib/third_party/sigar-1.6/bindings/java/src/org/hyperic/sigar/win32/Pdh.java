/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
 * Copyright (c) 2010 VMware, Inc.
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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.StringTokenizer;

import org.hyperic.sigar.SigarLoader;

public class Pdh extends Win32 {

    //PDH_CSTATUS_* from pdhmsg.h

    /**
     * The returned data is valid.
     */
    public static final int VALID_DATA = 0x00000000;

    /**
     * The specified instance is not present.
     */
    public static final int NO_INSTANCE = 0x800007D1;

    /**
     * The specified counter could not be found.
     */
    public static final int NO_COUNTER = 0xC0000BB9;

    /**
     * The specified object is not found on the system.
     */
    public static final int NO_OBJECT = 0xC0000BB8;

    /**
     * Unable to connect to specified machine or machine is off line.
     */
    public static final int NO_MACHINE = 0x800007D0;

    /**
     * Unable to parse the counter path.
     */
    public static final int BAD_COUNTERNAME = 0xC0000BC0;

    public static final String PERFLIB_KEY =
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";

    //see winperf.h
    public static final long PERF_TYPE_NUMBER  = 0x00000000; // a number (not a counter)

    public static final long PERF_TYPE_COUNTER = 0x00000400; // an increasing numeric value

    public static final long PERF_TYPE_TEXT    = 0x00000800; // a text field

    public static final long PERF_TYPE_ZERO    = 0x00000C00; // displays a zero

    private long   query = -1l; // Handle to the query
    private String hostname = null;
    private static Map counters = null;

    static {
        final String prop = "sigar.pdh.enableTranslation";
        if (SigarLoader.IS_WIN32 &&
            !"false".equals(System.getProperty(prop)))
        {
            try {
                enableTranslation();
            } catch (Exception e) {
                System.err.println(prop + ": " +
                                   e.getMessage());
            }
        }
    }

    public Pdh() throws Win32Exception {
        this.query = pdhOpenQuery();
    }
    
    public Pdh(String hostName) throws Win32Exception {
        this();
        this.hostname = hostName;
    }

    protected void finalize() throws Throwable {
        try {
            this.close();
        } finally {
            super.finalize();
        }
    }

    public synchronized void close() throws Win32Exception {
        if (this.query != -1l) {
            pdhCloseQuery(this.query);
            this.query = -1l;
        }
    }

    public static void enableTranslation() throws Win32Exception {
        if (counters != null) {
            return;
        }
        
        if (LocaleInfo.isEnglish()) {
            return;
        }

        counters = getEnglishPerflibCounterMap();
    }

    private static class PerflibCounterMap extends ArrayList {
        private Map map = new HashMap();
        private String index = null;

        //called by RegistryKey.getMultiStringValue
        //format description see: http://support.microsoft.com/kb/q287159/
        public boolean add(Object o) {
            if (index == null) {
                index = (String)o;
                return true;
            }
            String name = ((String)o).trim().toLowerCase();
            int[] ix = (int[])this.map.get(name);
            if (ix == null) {
                ix = new int[1];
            }
            else {
                int[] cur = ix;
                ix = new int[cur.length + 1];
                System.arraycopy(cur, 0, ix, 1, cur.length);
            }
            ix[0] = Integer.parseInt(index);
            //name -> index
            this.map.put(name, ix);
            index = null; //reset
            return true;
        }
    }

    public static Map getEnglishPerflibCounterMap()
        throws Win32Exception {

        LocaleInfo locale =
            new LocaleInfo(LocaleInfo.LANG_ENGLISH);

        return getPerflibCounterMap(locale);
    }

    public static Map getPerflibCounterMap(LocaleInfo locale)
        throws Win32Exception {

        String path =
            PERFLIB_KEY + "\\" + locale.getPerflibLangId();

        RegistryKey key =
            RegistryKey.LocalMachine.openSubKey(path);

        PerflibCounterMap counters = new PerflibCounterMap();
        try {
            key.getMultiStringValue("Counter", counters);
        } finally {
            key.close();
        }

        return counters.map;
    }

    public static String getCounterName(int index)
        throws Win32Exception {

        String name = pdhLookupPerfName(index).trim();

        return name;
    }

    /**
     * @deprecated
     * @see #getRawValue(String path)
     */
    public double getSingleValue(String path) throws Win32Exception {
        return getRawValue(path);
    }

    public double getRawValue(String path) throws Win32Exception {
        return getValue(path, false);
    }

    public double getFormattedValue(String path) throws Win32Exception {
        return getValue(path, true);
    }

    private static final String DELIM = "\\";

    private static int[] getCounterIndex(String englishName) {
        if (counters == null) {
            return null;
        }
        return (int[])counters.get(englishName.toLowerCase());
    }

    private static String getCounterName(String englishName)
        throws Win32Exception {

        int[] ix = getCounterIndex(englishName);
        if (ix == null) {
            return englishName;
        }

        String name = getCounterName(ix[0]);
        return name;
    }

    public static String translate(String path)
        throws Win32Exception {

        if (counters == null) {
            return path;
        }

        StringBuffer trans = new StringBuffer();
        StringTokenizer tok =
            new StringTokenizer(path, DELIM);

        int num = tok.countTokens();

        if (num == 3) {
            String hostname = tok.nextToken();
            trans.append(DELIM).append(DELIM).append(hostname);
        }

        String object = tok.nextToken();
        String instance = null;
        int ix = object.indexOf('(');
        if (ix != -1) {
            instance = object.substring(ix);
            object = object.substring(0, ix);
        }

        trans.append(DELIM).append(getCounterName(object));
        if (instance != null) {
            trans.append(instance);
        }

        String counter = tok.nextToken();
        trans.append(DELIM);

        int[] cix = getCounterIndex(counter);
        if (cix != null) {
            if (cix.length == 1) {
                counter = getCounterName(cix[0]);
            }
            else {
                //handle duplicate counter names
                for (int i=0; i<cix.length; i++) {
                    String name =
                        getCounterName(cix[i]);
                    if (validate(trans + name) == VALID_DATA) {
                        counter = name;
                        break;
                    }
                }
            }
        }

        trans.append(counter);

        return trans.toString();
    }

    private double getValue(String path, boolean format)
        throws Win32Exception {

        if (this.hostname != null) {
            pdhConnectMachine(this.hostname);
        }

        long counter =
            pdhAddCounter(this.query, translate(path));
        try {
            return pdhGetValue(this.query, counter, format);
        } finally {
            pdhRemoveCounter(counter);
        }
    }

    /* PdhCounterInfo.ExplainText */
    public String getDescription(String path) throws Win32Exception {
        long counter =
            pdhAddCounter(this.query, translate(path));
        try {
            return pdhGetDescription(counter);
        } finally {
            pdhRemoveCounter(counter);
        }
    }

    public long getCounterType(String path) throws Win32Exception {
        long counter =
            pdhAddCounter(this.query, translate(path));
        try {
            return pdhGetCounterType(counter);
        } finally {
            pdhRemoveCounter(counter);
        }
    }

    private static final class InstanceIndex {
        long index = 0;
    }

    public static String[] getInstances(String path) throws Win32Exception {
        String[] instances = pdhGetInstances(getCounterName(path));

        /* PdhEnumObjectItems() does not include the instance index */
        HashMap names = new HashMap(instances.length);
        for (int i=0; i<instances.length; i++) {
            InstanceIndex ix = (InstanceIndex)names.get(instances[i]);
            if (ix == null) {
                ix = new InstanceIndex();
                names.put(instances[i], ix);
            }
            else {
                ix.index++;
                instances[i] = instances[i] + "#" + ix.index;
            }
        }

        return instances;
    }

    public static String[] getKeys(String path) throws Win32Exception {
        return pdhGetKeys(getCounterName(path));
    }

    public static String[] getObjects() throws Win32Exception {
        return pdhGetObjects();
    }

    public static final native int validate(String path);

    private static final native void pdhConnectMachine(String host)
        throws Win32Exception;
    private static final native long pdhOpenQuery() throws Win32Exception;
    private static final native void pdhCloseQuery(long query)
        throws Win32Exception;
    private static final native long pdhAddCounter(long query, String path)
        throws Win32Exception;
    private static final native void pdhRemoveCounter(long counter)
        throws Win32Exception;
    private static final native double pdhGetValue(long query, 
                                                   long counter,
                                                   boolean fmt)
        throws Win32Exception;
    private static final native String pdhGetDescription(long counter)
        throws Win32Exception;
    private static final native long pdhGetCounterType(long counter)
        throws Win32Exception;
    private static final native String[] pdhGetInstances(String path)
        throws Win32Exception;
    private static final native String[] pdhGetKeys(String path)
        throws Win32Exception;
    private static final native String[] pdhGetObjects()
        throws Win32Exception;
    private static final native String pdhLookupPerfName(int index)
        throws Win32Exception;
    private static final native int pdhLookupPerfIndex(String name)
        throws Win32Exception;

    /**
     * Main method for dumping the entire PDH
     *
     * Usage: Pdh [OPTION]
     * Show information from the Windows PDH
     *
     * -v, --values         include key values [default=no]
     *     --object=NAME    only print info on this object
     *     --contains=NAME  only print info on objects that contain 
                            this substring
     * -i, --instance       show instances [default=no]
     * -k, --keys           show keys [default=no]
     * -h, --help           display help and exit
     */
    public static void main(String args[]) {

        Pdh      pdh           = null;
        String   objectName    = null;
        String   partialName   = null;
        boolean  showValues    = false;
        boolean  showInstances = false;
        boolean  showKeys      = false;

        // Parse command line arguments
        if (args.length > 0) {
            for (int i = 0; i < args.length; i++) {
                if (args[i].equals("-h") ||
                    args[i].equals("-help") ||
                    args[i].equals("--help")) {
                    System.out.println("Usage: Pdh [OPTION]");
                    System.out.println("Show information from the Windows " +
                                       "PDH");
                    System.out.println("");
                    System.out.println("    --object=NAME    " +
                                       "only print info on this object");
                    System.out.println("    --contains=NAME  " +
                                       "only print info on objects that");
                    System.out.println("                     " +
                                       "contain this substring");
                    System.out.println("-i, --instance       " +
                                       "show instances [default=no]");
                    System.out.println("-k, --keys           " +
                                       "show keys [default=no]");
                    System.out.println("-v, --values         " +
                                       "include key values [default=no]");
                    System.out.println("-h, --help           " +
                                       "display help and exit");
                    return;
                } else if (args[i].equals("-v") ||
                           args[i].equals("--values")) {
                    showKeys   = true;  // Assume -k when -v is used.
                    showValues = true;
                } else if (args[i].equals("-i") ||
                           args[i].equals("--instances")) {
                    showInstances = true;
                } else if (args[i].equals("-k") ||
                           args[i].equals("--keys")) {
                    showKeys = true;
                } else if (args[i].startsWith("--contains=")) {
                    int idx = args[i].indexOf("=");
                    partialName = args[i].substring(idx + 1);
                } else if (args[i].startsWith("--object=")) {
                    int idx = args[i].indexOf("=");
                    objectName = args[i].substring(idx + 1);
                } else {
                    System.out.println("Unknown option: " + args[i]);
                    System.out.println("Use --help for usage information");
                    return;
                }
            }
        }
        
        try {
            pdh = new Pdh();

            String[] objects;  // The list of objects to inspect.

            if (partialName != null) {
                // Check list of object names for a user defined
                // substring. (e.g. --contains=PLUMTREE for example)
                List matching = new ArrayList();
                String[] allObjects = Pdh.getObjects();

                for (int i = 0; i < allObjects.length; i++) {
                    if (allObjects[i].toUpperCase().
                        indexOf(partialName.toUpperCase()) != -1) {
                        matching.add(allObjects[i]);
                    }
                }
                objects = (String[])matching.toArray(new String[0]);

            } else if (objectName != null) {
                // Query for a single object
                objects = new String[] { objectName };
            } else {
                objects = Pdh.getObjects();
            }

            for (int o = 0; o < objects.length; o++) {
                System.out.println(objects[o]);

                // Get the query keys for this object
                String[] keys;
                try {
                    keys = Pdh.getKeys(objects[o]);
                } catch (Win32Exception e) {
                    System.err.println("Unable to get keys for object=" +
                                       objects[o] + " Reason: " +
                                       e.getMessage());
                    continue;
                }

                int pad = getLongestKey(keys);

                // Get the instances for this object
                String[] instances = Pdh.getInstances(objects[o]);
                if (instances.length == 0) {
                    // No instances, dump the keys and values for the 
                    // top level object

                    if (showKeys == false)
                        continue;

                    for (int k = 0; k < keys.length; k++) {
                        if (showValues) {
                            String query =
                                "\\" + objects[o] + "\\" + keys[k];
                            double val;

                            try {
                                val = pdh.getRawValue(query);
                            } catch (Win32Exception e) {
                                System.err.println("Unable to get value for " +
                                                   " key=" + query +
                                                   " Reason: " + e.getMessage());
                                continue;
                            }

                            String out = pad(keys[k], pad, ' ');
                            System.out.println("  " + out + " = " + val);
                        } else {
                            System.out.println("  " + keys[k]);
                        }
                    }
                } else {
                    // Only show instance level info if asked.
                    if (showInstances == false)
                        continue;

                    // For each instance, print it along with the keys
                    for (int i = 0; i < instances.length; i++) {
                        System.out.println("  " + instances[i]);
                        // Dump the keys for this instance
                        
                        if (showKeys == false)
                            continue;

                        for (int k = 0; k < keys.length; k++) {
                            if (showValues) {
                                String query = 
                                    "\\" + objects[o] + 
                                    "(" + instances[i] + ")" +
                                    "\\" + keys[k];

                                double val;

                                try {
                                    val = pdh.getRawValue(query);
                                } catch (Win32Exception e) {
                                    System.err.println("Unable to get value " +
                                                       "for key=" + query +
                                                       " Reason: " + 
                                                       e.getMessage());

                                    continue;
                                }
                                
                                String out = pad(keys[k], pad, ' ');
                                System.out.println("    " + out + " = " +
                                                   val);
                            } else {
                                System.out.println("    " + keys[k]);
                            }
                        }
                    }
                }
            }

            pdh.close();

        } catch (Win32Exception e) {
            // Should never happen
            System.err.println("Unable to dump PDH data: " +
                               e.getMessage());
            return;
        }
    }
    
    /**
     * String padder yanked from java-util's StringUtil.java
     */
    private static String pad(String value, int length, char ch) {
        StringBuffer padder = new StringBuffer(value);
        if (value.length() < length) {
            for (int i=0; i < (length - value.length()); i++) {
                padder.append(ch);
            }
        }
        return padder.toString();
    }

    /**
     * Returns the length of the longest string in an array
     */
    private static int getLongestKey(String[] keys) {
        int longest = 0;

        for (int i = 0; i < keys.length; i++) {
            int len = keys[i].length();
            if (len > longest)
                longest = len;
        }

        return longest;
    }
}
