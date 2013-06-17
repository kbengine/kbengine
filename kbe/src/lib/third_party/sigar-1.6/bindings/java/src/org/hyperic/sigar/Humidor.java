/*
 * Copyright (c) 2009 SpringSource, Inc.
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

package org.hyperic.sigar;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

/**
 * The Humidor provides access to a single Sigar instance.  Sigar has a
 * high-cost setup/teardown and has a high amount of caching, used to determine
 * things like the % of CPU used in-between calls.
 *
 * The Humidor also synchronizes access to all Sigar methods, so all
 * calls will be serialized.
 */
public class Humidor {
    private static final Humidor INSTANCE = new Humidor();
    private Object LOCK = new Object();
    private InvocationHandler _handler;
    private SigarProxy _sigar;
    private Sigar _impl, _inst;

    private Humidor() {}

    public Humidor(Sigar sigar) {
        _impl = sigar;
    }

    private static class MyHandler implements InvocationHandler {
        private SigarProxy _sigar;
        private Object _lock = new Object();

        public MyHandler(SigarProxy sigar) {
            _sigar = sigar;
        }

        public Object invoke(Object proxy, Method meth, Object[] args)
            throws Throwable
        {
            try {
                synchronized (_lock) {
                    return meth.invoke(_sigar, args);
                }
            } catch(InvocationTargetException e) {
                throw e.getTargetException();
            }
        }
    }

    public SigarProxy getSigar() {
        synchronized(LOCK) {
            if (_sigar == null) {
                if (_impl == null) {
                    _inst = _impl = new Sigar();
                }
                _handler = new MyHandler(_impl);
                _sigar = (SigarProxy)
                    Proxy.newProxyInstance(Humidor.class.getClassLoader(),
                                           new Class[] { SigarProxy.class },
                                           _handler);
            }
            return _sigar;
        }
    }

    public static Humidor getInstance() {
        return INSTANCE;
    }

    //close the Sigar instance if getSigar() created it
    public void close() {
        if (_inst != null) {
            _inst.close();
            _impl = _inst = null;
        }
        _sigar = null;
    }
}
