/*
 * Copyright (c) 2006 Hyperic, Inc.
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

package org.hyperic.sigar.jmx;

import java.util.Map;
import java.util.StringTokenizer;

import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarInvoker;
import org.hyperic.sigar.SigarNotImplementedException;
import org.hyperic.sigar.SigarProxy;
import org.hyperic.sigar.util.ReferenceMap;

/**
 * Extension of SigarInvoker to provide JMX style ObjectName
 * interface to sigar.
 */
public class SigarInvokerJMX extends SigarInvoker {

    public static final String DOMAIN_NAME = "sigar";

    public static final String PROP_TYPE = "Type";
    public static final String PROP_ARG  = "Arg";

    private String arg = null;

    private static Map cache = 
        ReferenceMap.synchronizedMap();

    /**
     * Get an invoker instance for the given object name.
     * A cache is maintained so only 1 invoker object per
     * unique object name will be constructed.  The object name
     * is in the format of:
     * domainName:prop=val,prop2=val2
     * Only two properties are currently recognized:
     * Type == The SigarProxy name, required. (e.g. Cpu, Mem)
     * Arg  == Argument (if any) for the given type, optional. (e.g. Arg=eth0)
     * @param proxy The SigarProxy object (e.g. SigarProxyCache)
     * @param name The object Name (e.g. sigar:Type=NetIfStat,Arg=eth0)
     */
    public static SigarInvokerJMX getInstance(SigarProxy proxy,
                                              String name) {

        SigarInvokerJMX invoker;

        int ix = name.indexOf(":");
        if (ix > 0) {
            //skip domain name
            name = name.substring(ix + 1);
        }

        if ((invoker = (SigarInvokerJMX)cache.get(name)) != null) {
            invoker.setProxy(proxy);
            return invoker;
        }

        invoker = new SigarInvokerJMX();

        invoker.setProxy(proxy);

        StringTokenizer st = new StringTokenizer(name, ",");

        while (st.hasMoreTokens()) {
            String attr = st.nextToken(); 
            ix = attr.indexOf('=');

            String key = attr.substring(0, ix);
            String val = attr.substring(key.length()+1);

            if (key.equals(PROP_TYPE)) {
                invoker.setType(val);
            }
            else if (key.equals(PROP_ARG)) {
                //need to decode value, e.g. Arg=C%3D\ => Arg=C:\
                invoker.setArg(decode(val));
            }
        }

        cache.put(name, invoker);

        return invoker;
    }

    //like URLDecoder but we only look for escaped ObjectName
    //delimiters ':', '=' and ','
    public static String decode(String val) {
        StringBuffer buf = new StringBuffer(val.length());
        boolean changed = false;
        int len = val.length();
        int i = 0;

        while (i < len) {
            char c = val.charAt(i);

            if (c == '%') {
                char d;
                if (i+2 > len) {
                    break;
                }
                String s = val.substring(i+1, i+3);

                if (s.equals("3A")) {
                    d = ':';
                }
                else if (s.equals("3D")) {
                    d = '=';
                }
                else if (s.equals("2C")) {
                    d = ',';
                }
                else {
                    buf.append(c);
                    i++;
                    continue;
                }

                changed = true;
                buf.append(d);
                i += 3;
            }
            else {
                buf.append(c);
                i++;
            }
        }

        return changed ? buf.toString() : val;
    }

    private void setArg(String val) {
        this.arg = val;
    }

    /**
     * The value of the parsed Arg property.
     */
    public String getArg() {
        return this.arg;
    }

    /**
     * Returns a JMX style object name with given property values.
     */
    public static String getObjectName(String type, String arg) {
        String s = DOMAIN_NAME + ":";

        s += PROP_TYPE + "=" + type;

        if (arg != null) {
            s += "," + PROP_ARG + "=" + arg;
        }

        return s;
    }

    /**
     * Returns a JMX style object that represents this instance.
     */
    public String getObjectName() {
        return getObjectName(getType(), getArg());
    }

    public String toString() {
        return getObjectName();
    }

    /**
     * Invoke an attribute method for the given object.
     * Example:
     * SigarInvokerJMX invoker = new SigarInvokerJMX(proxy, "Type=Mem");
     * Object val = invoker.invoke("Free");
     *
     * @param attr The attribute name (e.g. Total)
     * @exception SigarException If invocation fails.
     */
    public Object invoke(String attr)
        throws SigarException, SigarNotImplementedException {

        return super.invoke(getArg(), attr);
    }
}
