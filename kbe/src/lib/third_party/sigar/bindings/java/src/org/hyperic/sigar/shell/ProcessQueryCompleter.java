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

package org.hyperic.sigar.shell;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.hyperic.sigar.SigarProxy;
import org.hyperic.sigar.util.GetlineCompleter;

public class ProcessQueryCompleter implements GetlineCompleter {

    private static final String SIGAR_PACKAGE =
        "org.hyperic.sigar.";

    private static final Map METHODS = new HashMap();

    private static final Collection NOPS =
        Arrays.asList(new String[] {
            "eq", "ne", "gt", "ge", "lt", "le"
        });

    private static final Collection SOPS =
        Arrays.asList(new String[] {
            "eq", "ne", "re", "ct", "ew", "sw"
        });

    private static final Class[] NOPARAM = new Class[0];
    private static final String PROC_PREFIX = "getProc";

    private ShellBase shell;
    private GetlineCompleter m_completer;
    private Map methods;

    static {
        Method[] methods = SigarProxy.class.getMethods();

        for (int i=0; i<methods.length; i++) {
            String name = methods[i].getName();

            if (!name.startsWith(PROC_PREFIX)) {
                continue;
            }

            Class[] params = methods[i].getParameterTypes();

            //getProcFoo(long pid)
            if (!((params.length == 1) &&
                  (params[0] == Long.TYPE)))
            {
                continue;
            }

            METHODS.put(name.substring(PROC_PREFIX.length()), methods[i]);
        }
    }

    public ProcessQueryCompleter(ShellBase shell) {
        this.shell = shell;
        this.methods = getMethods();
        this.m_completer =
            new CollectionCompleter(shell, methods.keySet());
    }

    public static Map getMethods() {
        return METHODS;
    }

    public static Collection getMethodOpNames(Method method) {
        if (method == null) {
            return SOPS;
        }

        Class rtype = method.getReturnType();

        if ((rtype == Character.TYPE) ||
            (rtype == Double.TYPE) ||
            (rtype == Integer.TYPE) ||
            (rtype == Long.TYPE))
        {
            return NOPS;
        }

        return SOPS;
    }

    public static boolean isSigarClass(Class type) {
        return type.getName().startsWith(SIGAR_PACKAGE);
    }

    public String complete(String line) {
        int ix = line.indexOf(".");

        if (ix == -1) {
            line = this.m_completer.complete(line);
            if (!line.endsWith(".")) {
                if (this.methods.get(line) != null) {
                    return line + ".";
                }
            }
            return line;
        }

        String attrClass = line.substring(0, ix);
        String attr = line.substring(ix+1, line.length());

        Method method = (Method)this.methods.get(attrClass);
        if (method == null) {
            return line;
        }

        Class subtype = method.getReturnType();

        boolean isSigarClass = isSigarClass(subtype);

        int ix2 = attr.indexOf(".");
        if (ix2 != -1) {
            method = null;
            String op = attr.substring(ix2+1, attr.length());
            attr = attr.substring(0, ix2);

            if (isSigarClass) {
                try {
                    method =
                        subtype.getMethod("get" + attr, NOPARAM);
                } catch (NoSuchMethodException e) { }
            }

            final Method m = method;

            GetlineCompleter completer =
                new CollectionCompleter(this.shell,
                                        getMethodOpNames(m));

            String partial = completer.complete(op);
            String result = attrClass + "." + attr + "." + partial;
            if (partial.length() == 2) {
                result += "=";
            }
            return result;
        }

        if (isSigarClass) {
            final ArrayList possible = new ArrayList();
            Method[] submethods = subtype.getDeclaredMethods();
            for (int i=0; i<submethods.length; i++) {
                Method m = submethods[i];
                if (m.getName().startsWith("get")) {
                    possible.add(m.getName().substring(3));
                }
            }

            GetlineCompleter completer =
                new CollectionCompleter(this.shell, possible);

            String partial = completer.complete(attr);
            String result = attrClass + "." + partial;
            if (possible.contains(partial)) {
                result += ".";
            }
            return result;
        }
        
        return line;
    }
}
