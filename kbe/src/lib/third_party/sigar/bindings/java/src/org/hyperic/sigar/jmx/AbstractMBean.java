/*
 * Copyright (c) 2007, 2009 Hyperic, Inc.
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

import javax.management.Attribute;
import javax.management.AttributeList;
import javax.management.AttributeNotFoundException;
import javax.management.DynamicMBean;
import javax.management.InvalidAttributeValueException;
import javax.management.MBeanException;
import javax.management.MBeanRegistration;
import javax.management.MBeanServer;
import javax.management.ObjectName;
import javax.management.ReflectionException;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarProxy;
import org.hyperic.sigar.SigarProxyCache;

/**
 * Base class for all Sigar JMX MBeans. Provides a skeleton which handles
 * creation of the Sigar proxy instance and provides some convenience methods.
 * It also enforces usage of the {@link DynamicMBean} inferface while
 * implementing part of it, and it adds empty implementations for all methods of
 * the {@link MBeanRegistration} interface, allowing subclasses to only
 * implement subset of them.
 * 
 * @author Bjoern Martin
 * @since 1.5
 */
public abstract class AbstractMBean implements DynamicMBean, MBeanRegistration {

    protected static final String MBEAN_ATTR_TYPE = "type";

    protected static final short CACHED_30SEC = 0;

    protected static final short CACHED_5SEC = 1;

    protected static final short CACHED_500MS = 2;

    protected static final short CACHELESS = 3;

    protected static final short DEFAULT = CACHED_30SEC;

    /**
     * The Sigar implementation to be used to fetch information from the system.
     */
    protected final Sigar sigarImpl;

    /**
     * The Sigar proxy cache to be used in case the data does not have to be 
     * fetched during each call. The cache timeout is decided during 
     * construction. See {@link AbstractMBean#AbstractMBean(Sigar, short)} for 
     * details.
     * 
     * @see AbstractMBean#AbstractMBean(Sigar, short)
     */
    protected final SigarProxy sigar;

    /**
     * The MBean server this MBean is registered to. Set during the MBean's 
     * registration to the MBean server and unset to <code>null</code> when
     * the deregistration finished.
     * 
     * @see #preRegister(MBeanServer, ObjectName)
     * @see #postDeregister()
     */
    protected MBeanServer mbeanServer;

    /**
     * <p>Creates a new instance of this class. The Sigar instance is stored (and 
     * accessible) via the {@link #sigarImpl} member. A second instance is 
     * stored within the {@link #sigar} member which is either {@link #sigarImpl}
     * or an instance of {@link SigarProxyCache} with the expiration time set to 
     * whatever the <code>cacheMode</code> parameter specifies.</p>
     * 
     * <p>The following cache modes exist:</p>
     * 
     * <table border = "1">
     * <tr><td><b>Constant</b></td><td><b>Description</b></td></tr>
     * <tr><td>{@link #CACHELESS}</td><td>No cached instance, {@link #sigar} 
     *         <code>==</code> {@link #sigarImpl}.</td></tr>
     * <tr><td>{@link #CACHED_500MS}</td><td>500 millisecond cache, for high 
     *         frequency queries on raw data such as reading out CPU timers each 
     *         second. Avoids reading out multiple data sets when all attributes of 
     *         an MBean are queried in short sequence.</td></tr>
     * <tr><td>{@link #CACHED_5SEC}</td><td>5 second cache, for high frequency 
     *         queries on calculated data such as CPU percentages.</td></tr>
     * <tr><td>{@link #CACHED_30SEC}</td><td>30 second cache, for normal queries 
     *         or data readouts such as CPU model / vendor. This is the default if 
     *         nothing (<code>0</code>) is specified.</td></tr>
     * <tr><td>{@link #DEFAULT}</td><td>Same as {@link #CACHED_30SEC}.</td></tr>
     * </table>
     * 
     * <p><b>Note:</b> Only make use of the cacheless or half second mode if you 
     * know what you are doing. They may have impact on system performance if 
     * used excessively.</p>
     * 
     * @param sigar The Sigar impl to use. Must not be <code>null</code>
     * @param cacheMode The cache mode to use for {@link #sigar} or {@link #CACHELESS}
     *         if no separate, cached instance is to be maintained.
     */
    protected AbstractMBean(Sigar sigar, short cacheMode) {
        // store Sigar
        this.sigarImpl = sigar;

        // create a cached instance as well
        if (cacheMode == CACHELESS) {
            // no cached version
            this.sigar = this.sigarImpl;

        } else if (cacheMode == CACHED_500MS) {
            // 500ms cached version (for 1/sec queries)
            this.sigar = SigarProxyCache.newInstance(this.sigarImpl, 500);

        } else if (cacheMode == CACHED_5SEC) {
            // 5sec cached version (for avg'd queries)
            this.sigar = SigarProxyCache.newInstance(this.sigarImpl, 5000);

        } else /* if (cacheMode == CACHED_30SEC) */{
            // 30sec (default) cached version (for info and long term queries)
            this.sigar = SigarProxyCache.newInstance(this.sigarImpl, 30000);
        }
    }

    /**
     * Returns the object name the MBean is registered with within the
     * MBeanServer. May be <code>null</code> in case the instance is not
     * registered to an MBeanServer, but used standalone.
     * 
     * @return The object name or <code>null</code> if not registered to an
     *         MBeanServer
     */
    public abstract String getObjectName();

    /**
     * Returns a runtime exception for the type and SigarException specified.
     * 
     * @param type
     *            The type that was called
     * @param e
     *            The exception that was raised
     * @return A runtime exception encapsulating the information specified
     */
    protected RuntimeException unexpectedError(String type, SigarException e) {
        String msg = "Unexected error in Sigar.get" + type + ": "
                + e.getMessage();
        return new IllegalArgumentException(msg);
    }

    /**
     * Loops over all attributes and calls
     * {@link DynamicMBean#getAttribute(java.lang.String)} method for each
     * attribute sequentially. Any exception thrown by those methods are ignored
     * and simply cause the attribute not being added to the result.
     */
    public AttributeList getAttributes(String[] attrs) {
        final AttributeList result = new AttributeList();
        for (int i = 0; i < attrs.length; i++) {
            try {
                result.add(new Attribute(attrs[i], getAttribute(attrs[i])));
            } catch (AttributeNotFoundException e) {
                // ignore, as we cannot throw this exception
            } catch (MBeanException e) {
                // ignore, as we cannot throw this exception
            } catch (ReflectionException e) {
                // ignore, as we cannot throw this exception
            }
        }
        return result;
    }

    /**
     * Loops over all attributes and calls
     * {@link DynamicMBean#setAttribute(Attribute)} for each attribute
     * sequentially. Any exception thrown by those methods are ignored and
     * simply cause the attribute not being added to the result.
     */
    public AttributeList setAttributes(AttributeList attrs) {
        final AttributeList result = new AttributeList();
        for (int i = 0; i < attrs.size(); i++) {
            try {
                final Attribute next = (Attribute) attrs.get(i);
                setAttribute(next);
                result.add(next);
            } catch (AttributeNotFoundException e) {
                // ignore, as we cannot throw this exception
            } catch (InvalidAttributeValueException e) {
                // ignore, as we cannot throw this exception
            } catch (MBeanException e) {
                // ignore, as we cannot throw this exception
            } catch (ReflectionException e) {
                // ignore, as we cannot throw this exception
            }
        }
        return result;
    }

    // -------
    // Implementation of the MBeanRegistration interface
    // -------

    /**
     * <p>Returns <code>new ObjectName(this.getObjectName())</code> to guarantee 
     * a reliable and reproducable object name.</p>
     * 
     * <p><b>Note:</b> Make sure any subclass does a super call to this method, 
     * otherwise the implementation might be broken.</p>
     * 
     * @see MBeanRegistration#preRegister(MBeanServer, ObjectName)
     */
    public ObjectName preRegister(MBeanServer server, ObjectName name)
            throws Exception {
        this.mbeanServer = server;
        return new ObjectName(getObjectName());
    }

    /**
     * Empty implementation, allowing aubclasses to ignore the interface.
     * 
     * <p><b>Note:</b> Make sure any subclass does a super call to this method, 
     * otherwise the implementation might be broken.</p>
     * 
     * @see MBeanRegistration#postRegister(Boolean)
     */
    public void postRegister(Boolean success) {
    }

    /**
     * Empty implementation, allowing aubclasses to ignore the interface.
     * 
     * <p><b>Note:</b> Make sure any subclass does a super call to this method, 
     * otherwise the implementation might be broken.</p>
     * 
     * @see MBeanRegistration#preDeregister()
     */
    public void preDeregister() throws Exception {
    }

    /**
     * Empty implementation, allowing aubclasses to ignore the interface.
     * 
     * <p><b>Note:</b> Make sure any subclass does a super call to this method, 
     * otherwise the implementation might be broken.</p>
     * 
     * @see MBeanRegistration#postDeregister()
     */
    public void postDeregister() {
        this.mbeanServer = null;
    }
}
