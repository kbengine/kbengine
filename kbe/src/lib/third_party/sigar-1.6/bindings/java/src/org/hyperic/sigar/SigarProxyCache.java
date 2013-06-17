/*
 * Copyright (c) 2006-2009 Hyperic, Inc.
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

import java.util.Map;

import java.lang.reflect.Proxy;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

import org.hyperic.sigar.util.ReferenceMap;

/**
 * This class implements a caching mechanism to avoid system calls
 * with heavy Sigar usage in a short period of time.  It is intended
 * for use in applications such as Top.
 */
public class SigarProxyCache
    implements InvocationHandler {

    private Sigar sigar;
    private Map cache = ReferenceMap.newInstance();
    public static final int EXPIRE_DEFAULT = 30 * 1000; //30 seconds
    private int expire;
    private static final boolean debugEnabled =
        "debug".equals(System.getProperty("sigar.log"));

    public SigarProxyCache(Sigar sigar, int expire) {
        this.sigar = sigar;
        this.expire = expire;
    }

    public static SigarProxy newInstance() {
        return newInstance(new Sigar());
    }

    public static SigarProxy newInstance(Sigar sigar) {
        return newInstance(sigar, EXPIRE_DEFAULT);
    }

    public static SigarProxy newInstance(Sigar sigar, int expire) {

        SigarProxyCache handler = new SigarProxyCache(sigar, expire);
        SigarProxy proxy;

        proxy = (SigarProxy)
            Proxy.newProxyInstance(SigarProxy.class.getClassLoader(),
                                   new Class[] { SigarProxy.class },
                                   handler);

        return proxy;
    }

    //poor mans logging
    private void debug(String msg) {
        SigarLog.getLogger("SigarProxyCache").debug(msg);
    }

    /**
     * @deprecated
     */
    public static void setExpire(SigarProxy proxy,
                                 String type,
                                 int expire)
        throws SigarException {
    }

    private static SigarProxyCache getHandler(Object proxy) {
        return (SigarProxyCache)Proxy.getInvocationHandler(proxy);
    }

    public static void clear(Object proxy) {
        getHandler(proxy).cache.clear();
    }

    public static Sigar getSigar(Object proxy) {
        if (proxy.getClass() == Sigar.class) {
            return (Sigar)proxy;
        }
        else {
            return getHandler(proxy).sigar;
        }
    }

    private String getDebugArgs(Object[] args, Object argKey) {

        if (args.length == 0) {
            return null;
        }

        StringBuffer dargs =
            new StringBuffer(args[0].toString());
        
        for (int i=1; i<args.length; i++) {
            dargs.append(',').append(args[i].toString());
        }

        if (!dargs.toString().equals(argKey.toString())) {
            dargs.append('/').append(argKey);
        }

        return dargs.toString();
    }

    /**
     * The java.lang.reflect.InvocationHandler used by the Proxy.
     * This method handles caching of all Sigar type objects.
     */
    public synchronized Object invoke(Object proxy, Method method, Object[] args)
        throws SigarException, SigarNotImplementedException {

        SigarCacheObject cacheVal = null;
        Object retval;
        Object argKey = null;
        Map argMap = null;
        long timeNow = System.currentTimeMillis();

        if (args != null) {
            if (args.length == 1) {
                argKey = args[0];
            }
            else {
                int hashCode = 0;
                for (int i=0; i<args.length; i++) {
                    hashCode ^= args[i].hashCode();
                }
                argKey = new Integer(hashCode);
            }
            argMap = (Map)this.cache.get(method);
            if (argMap == null) {
                argMap = ReferenceMap.newInstance();
            }
            else {
                //XXX what todo when pids are stale?
                cacheVal = (SigarCacheObject)argMap.get(argKey);
            }
        }
        else {
            cacheVal = (SigarCacheObject)this.cache.get(method);
        }

        if (cacheVal == null) {
            cacheVal = new SigarCacheObject();
        }

        String argDebug = "";
        if (debugEnabled) {
            if ((args != null) && (args.length != 0)) {
                argDebug = " with args=" +
                    getDebugArgs(args, argKey);
            }
        }

        if (cacheVal.value != null) {
            if (debugEnabled) {
                debug("found " + method.getName() +
                      " in cache" + argDebug);
            }

            if ((timeNow - cacheVal.timestamp) > this.expire) {
                if (debugEnabled) {
                    debug("expiring " + method.getName() +
                          " from cache" + argDebug);
                }

                cacheVal.value = null;
            }
        }
        else {
            if (debugEnabled) {
                debug(method.getName() +
                      " NOT in cache" + argDebug);
            }
        }

        if (cacheVal.value == null) {
            try {
                retval = method.invoke(this.sigar, args);
            } catch (InvocationTargetException e) {
                Throwable t =
                    ((InvocationTargetException)e).
                    getTargetException();

                String msg;

                if (t instanceof SigarException) {
                    msg = "";
                }
                else {
                    msg = t.getClass().getName() + ": ";
                }

                msg += t.getMessage();

                if (argKey != null) {
                    msg += ": " + getDebugArgs(args, argKey);
                }

                if (t instanceof SigarNotImplementedException) {
                    throw new SigarNotImplementedException(msg);
                }
                else if (t instanceof SigarPermissionDeniedException) {
                    throw new SigarPermissionDeniedException(msg);
                }
                throw new SigarException(msg);
            } catch (Exception e) {
                String msg =
                    e.getClass().getName() + ": " +
                    e.getMessage();

                if (argKey != null) {
                    msg += ": " + getDebugArgs(args, argKey);
                }

                throw new SigarException(msg);
            }

            cacheVal.value = retval;
            cacheVal.timestamp = timeNow;

            if (args == null) {
                this.cache.put(method, cacheVal);
            }
            else {
                argMap.put(argKey, cacheVal);
                this.cache.put(method, argMap);
            }
        }
        else {
            retval = cacheVal.value;
        }

        return retval;
    }
}
