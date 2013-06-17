/*
 * Copyright (c) 2006, 2008 Hyperic, Inc.
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

package org.hyperic.sigar;

import java.util.Arrays;
import java.util.HashMap;

import java.lang.reflect.Proxy;
import java.lang.reflect.Method;

/**
 * This class provides a string-ish interface to sigar.
 * It is intended for use via JMX and Hyperic HQ measurement
 * plugins.  Method lookups are cached and the like for better
 * performance.
 */
public class SigarInvoker {

    private static HashMap attrCache = new HashMap();
    private static HashMap compatTypes = new HashMap();
    private static HashMap compatAttrs = new HashMap();

    static {
        //XXX backwards compat for HQ because metric template
        //updating does not work.
        compatTypes.put("NetIfconfig", "NetInterfaceConfig");
        compatTypes.put("NetIfstat", "NetInterfaceStat");
        compatTypes.put("DirStats", "DirStat");
        compatAttrs.put("Utime", "User");
        compatAttrs.put("Stime", "Sys");
    }

    //avoid object creation as much as possible
    private static final Class[] VOID_SIGNATURE = new Class[0];
    private Class[] ARG_SIGNATURE = new Class[] { String.class };
    private Class[] ARG2_SIGNATURE = new Class[] {
        String.class, String.class
    };
    private static final Object[] VOID_ARGS = new Object[0];
    private Object[] ARG_ARGS = new Object[1];

    private String type = null;

    private boolean typeIsArray = false;
    private int arrayIdx = -1;
    private boolean hasArrayIdx = false;
    private int typeArrayType;

    private static final int ARRAY_TYPE_OBJECT = 1;
    private static final int ARRAY_TYPE_DOUBLE = 2;
    private static final int ARRAY_TYPE_LONG   = 3;

    private Method typeMethod;

    private SigarProxy sigarProxy;
    private SigarProxyCache handler;

    protected SigarInvoker() {
    }

    /**
     * @param proxy SigarProxy implementation such as SigarProxyCache
     * @param type The sigar type.  Valid name is any of the SigarProxy
     * interface methods (minus the 'get' prefix).
     */
    public SigarInvoker(SigarProxy proxy, String type) {
        setProxy(proxy);
        setType(type);
    }

    protected void setProxy(SigarProxy proxy) {
        try {
            this.handler =
                (SigarProxyCache)Proxy.getInvocationHandler(proxy);
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
            //XXX we can still deal w/o this.handler
        }

        this.sigarProxy = proxy;
    }

    protected void setType(String val) {
        String alias = (String)compatTypes.get(val);
        if (alias != null) {
            val = alias;
        }
        this.type = val;
    }

    /**
     * The type of this instance, as passed to the constructor.
     */
    public String getType() {
        return this.type;
    }

    private int getAttributeIndex(String attr) {
        try {
            return Integer.valueOf(attr).intValue();
        } catch (NumberFormatException e) {
            return -1;
        }
    }

    private Method getTypeMethod(Object[] args)
        throws SigarException {

        if (this.typeMethod == null) {
            Class[] sig = VOID_SIGNATURE;
            boolean argIsArrayIdx = false;
            int argLength = 0;
            String getter = "get" + getType();

            if (args != null) {
                argLength = args.length;
                switch (argLength) {
                  case 1:
                    sig = ARG_SIGNATURE;
                    break;
                  case 2:
                    sig = ARG2_SIGNATURE;
                    break;
                  default:
                    throw new IllegalArgumentException();
                }
            }

            try {
                this.typeMethod = Sigar.class.getMethod(getter, sig);
            } catch (Exception e) {
                try {
                    this.typeMethod =
                        Sigar.class.getMethod(getter, VOID_SIGNATURE);
                    if (argLength == 1) {
                        argIsArrayIdx = true;
                    }
                } catch (Exception e2) {
                    String msg = "Unable to determine getter for " + type;
                    throw new SigarException(msg);
                }
            }

            Class typeClass = this.typeMethod.getReturnType();

            if (typeClass.isArray()) {
                this.typeIsArray = true;
                if (argIsArrayIdx) {
                    try {
                        this.arrayIdx = Integer.parseInt((String)args[0]);
                    } catch (NumberFormatException ne) {
                        String msg = getType() + ": '" +
                            args[0] + "' is not a number";
                        throw new SigarException(msg);
                    }
                    this.hasArrayIdx = true;
                }
                Class componentClass = typeClass.getComponentType();

                if (componentClass.isPrimitive()) {
                    //e.g. getLoadAverage
                    if (componentClass == Double.TYPE) {
                        this.typeArrayType = ARRAY_TYPE_DOUBLE;
                    }
                    //e.g. getProcList
                    else if (componentClass == Long.TYPE) {
                        this.typeArrayType = ARRAY_TYPE_LONG;
                    }
                    else {
                        //won't happen.
                        throw new SigarException("unsupported array type: " +
                                                 componentClass.getName());
                    }
                }
                else {
                    this.typeArrayType = ARRAY_TYPE_OBJECT;
                }
            }
            else {
                this.typeIsArray = false;
            }
        }

        return this.typeMethod;
    }

    public Object invoke(Object arg, String attr)
        throws SigarException,
               SigarNotImplementedException,
               SigarPermissionDeniedException {

        Object[] args = null;

        if (arg != null) {
            args = ARG_ARGS;
            args[0] = arg;
        }

        return invoke(args, attr);
    }

    private String aobMsg(int idx, int length) {
        return "Array index " + idx + " out of bounds " + length;
    }

    public Object invoke(Object[] args, String attr)
        throws SigarException,
               SigarNotImplementedException,
               SigarPermissionDeniedException {

        Method typeGetter, attrGetter;
        Object typeObject;

        typeGetter = getTypeMethod(args);
        if (this.hasArrayIdx) {
            args = null;
        }
        try {
            typeObject = this.handler.invoke(this.sigarProxy,
                                             typeGetter,
                                             args);
        } catch (Throwable t) {
            String parms =
                (args == null) ? "" : Arrays.asList(args).toString();
            String msg = "Failed to invoke " + 
                typeGetter.getName() + parms +
                ": " + t.getMessage();
            if (t instanceof SigarNotImplementedException) {
                throw (SigarNotImplementedException)t;
            }
            else if (t instanceof SigarPermissionDeniedException) {
                throw (SigarPermissionDeniedException)t;
            }
            throw new SigarException(msg);
        }

        if (attr == null) {
            return typeObject;
        }

        /*
         * if the return type is an array and we've been given
         * an attr, the attr is an index into the array, e.g.
         * for getLoadAverage which returns a double[].
         * working with primitive arrays here kinda sucks.
         */
        if (this.typeIsArray) {
            if (this.hasArrayIdx) {
                Object[] array = (Object[])typeObject;
                if (this.arrayIdx >= array.length) {
                    throw new SigarException(aobMsg(this.arrayIdx,
                                                    array.length));
                }
                typeObject = array[this.arrayIdx];
            }
            else {
                int idx = getAttributeIndex(attr);
                if (idx < 0) {
                    throw new SigarException("Invalid array index: " + attr);
                }
                switch (this.typeArrayType) {
                  case ARRAY_TYPE_DOUBLE:
                    double[] d_array = (double[])typeObject;
                    if (idx >= d_array.length) {
                        throw new SigarException(aobMsg(idx,
                                                        d_array.length));
                    }
                    return new Double(d_array[idx]);
                  case ARRAY_TYPE_LONG:
                    long[] l_array = (long[])typeObject;
                    if (idx >= l_array.length) {
                        throw new SigarException(aobMsg(idx,
                                                        l_array.length));
                    }
                    return new Long(l_array[idx]);
                  case ARRAY_TYPE_OBJECT:
                    Object[] o_array = (Object[])typeObject;
                    if (idx >= o_array.length) {
                        throw new SigarException(aobMsg(idx,
                                                        o_array.length));
                    }
                    return o_array[idx];
                }
            }
        }

        attrGetter = getAttributeMethod(attr);

        try {
            return attrGetter.invoke(typeObject, VOID_ARGS);
        } catch (Throwable t) {
            throw new SigarException(t.getMessage());
        }
    }

    private Method getAttributeMethod(String attr)
        throws SigarException {

        String alias = (String)compatAttrs.get(attr);
        if (alias != null) {
            attr = alias;
        }
        Method attrMethod;
        Class type = getTypeMethod(null).getReturnType();

        HashMap attrs;

        if (this.hasArrayIdx) {
            type = type.getComponentType();
        }

        //Class.getMethod can be expensive so cache the lookups
        synchronized (attrCache) {
            attrs = (HashMap)attrCache.get(type);

            if (attrs == null) {
                attrs = new HashMap();
                attrCache.put(type, attrs);
            }
            else {
                if ((attrMethod = (Method)attrs.get(attr)) != null) {
                    return attrMethod;
                }
            }
        }

        try {
            attrMethod = type.getMethod("get" + attr,
                                        VOID_SIGNATURE);
        } catch (Exception e) {
            String msg = "Failed to invoke get" + attr + ": " +
                e.getMessage();
            throw new SigarException(msg);
        }

        synchronized (attrs) {
            attrs.put(attr, attrMethod);
        }

        return attrMethod;
    }
}
