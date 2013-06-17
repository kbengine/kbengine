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
import javax.management.AttributeNotFoundException;
import javax.management.MBeanAttributeInfo;
import javax.management.MBeanConstructorInfo;
import javax.management.MBeanInfo;
import javax.management.MBeanParameterInfo;
import javax.management.ReflectionException;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;
import org.hyperic.sigar.SigarNotImplementedException;

/**
 * Sigar JMX MBean implementation for the <code>LoadAverage</code> information
 * package. Provides an OpenMBean conform implementation.
 * 
 * @author Bjoern Martin
 * @since 1.5
 */
public class SigarLoadAverage extends AbstractMBean {

    private static final String MBEAN_TYPE = "LoadAverage";

    /**
     * Returned if {@link Sigar#getLoadAverage()}} is detected to be not
     * implemented on the platform.
     * 
     * @see #notImplemented
     */
    private static final double NOT_IMPLEMENTED_LOAD_VALUE = -1.0d;

    private static final MBeanInfo MBEAN_INFO;

    private static final MBeanAttributeInfo MBEAN_ATTR_LAST1MIN;

    private static final MBeanAttributeInfo MBEAN_ATTR_LAST5MIN;

    private static final MBeanAttributeInfo MBEAN_ATTR_LAST15MIN;

    private static final MBeanConstructorInfo MBEAN_CONSTR_SIGAR;

    private static MBeanParameterInfo MBEAN_PARAM_SIGAR;

    static {
        MBEAN_ATTR_LAST1MIN = new MBeanAttributeInfo(
                "LastMinute",
                "double",
                "The load average in the last minute, as a fraction of 1, or "
                        + "-1.0 if the load cannot be determined on this platform",
                true, false, false);
        MBEAN_ATTR_LAST5MIN = new MBeanAttributeInfo(
                "LastFiveMinutes",
                "double",
                "The load average over the last five minutes, as a fraction "
                        + "of 1, or -1.0 if the load cannot be determined on this platform",
                true, false, false);
        MBEAN_ATTR_LAST15MIN = new MBeanAttributeInfo(
                "Last15Minutes",
                "double",
                "The load average over the last 15 minutes, as a fraction of "
                        + "1, or -1.0 if the load cannot be determined on this platform",
                true, false, false);

        MBEAN_PARAM_SIGAR = new MBeanParameterInfo("sigar", Sigar.class
                .getName(), "The Sigar instance to use to fetch data from");

        MBEAN_CONSTR_SIGAR = new MBeanConstructorInfo(
                SigarLoadAverage.class.getName(),
                "Creates a new instance, using the Sigar instance specified "
                        + "to fetch the data. Fails if the CPU index is out of range.",
                new MBeanParameterInfo[] { MBEAN_PARAM_SIGAR });
        MBEAN_INFO = new MBeanInfo(
                SigarLoadAverage.class.getName(),
                "Sigar load average MBean. Provides load averages of the "
                        + "system over the last one, five and 15 minutes. Due to the "
                        + "long term character of that information, the fetch is done "
                        + "using a Sigar proxy cache with a timeout of 30 seconds.",
                new MBeanAttributeInfo[] { MBEAN_ATTR_LAST1MIN,
                        MBEAN_ATTR_LAST5MIN, MBEAN_ATTR_LAST15MIN },
                new MBeanConstructorInfo[] { MBEAN_CONSTR_SIGAR }, null, null);

    }

    /**
     * Object name this instance will give itself when being registered to an
     * MBeanServer.
     */
    private final String objectName;

    /**
     * <p>Set <code>true</code> when the load average fetch failed with a 
     * <code>SigarException</code> that indicates the method is not implemented.
     * Any subsequent call to this instance will then be answered with 
     * {@link #NOT_IMPLEMENTED_LOAD_VALUE}.
     * </p>
     * 
     * <p><b>FIXME</b> : This is a workaround and should be replaced by something 
     * more stable, as the code setting this member <code>true</code> relies on 
     * a substring being present within the exception. A proposal was made at
     * <a href="http://jira.hyperic.com/browse/SIGAR-52">issue SIGAR-52</a>.
     * </p>
     */
    private boolean notImplemented;

    /**
     * Creates a new instance, using a new Sigar instance to fetch the data.
     * 
     * @throws IllegalArgumentException
     *      If an unexpected Sigar error occurs.
     */
    public SigarLoadAverage() throws IllegalArgumentException {
        this(new Sigar());
    }

    /**
     * Creates a new instance, using the Sigar instance specified to fetch the 
     * data.
     * 
     * @param sigar
     *            The Sigar instance to use to fetch data from
     * 
     * @throws IllegalArgumentException
     *             If an unexpected Sigar error occurs
     */
    public SigarLoadAverage(Sigar sigar) throws IllegalArgumentException {
        super(sigar, CACHED_30SEC);

        // all fine
        this.objectName = SigarInvokerJMX.DOMAIN_NAME + ":" + MBEAN_ATTR_TYPE
                + "=" + MBEAN_TYPE;
    }

    /**
     * Object name this instance will give itself when being registered to an
     * MBeanServer.
     */
    public String getObjectName() {
        return this.objectName;
    }

    /**
     * @return The load average in the last minute, as a fraction of 1, or 
     *      <code>-1.0d</code> if the load cannot be determined on this platform
     */
    public double getLastMinute() {
        try {
            return sigarImpl.getLoadAverage()[0];
            
        } catch (SigarNotImplementedException e) {
            return NOT_IMPLEMENTED_LOAD_VALUE;
            
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The load average over the last five minutes, as a fraction of 1, 
     *      or <code>-1.0d</code> if the load cannot be determined on this 
     *      platform
     */
    public double getLastFiveMinutes() {
        try {
            return sigarImpl.getLoadAverage()[1];
            
        } catch (SigarNotImplementedException e) {
            return NOT_IMPLEMENTED_LOAD_VALUE;
            
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The load average over the last 15 minutes, as a fraction of 1, or 
     *      <code>-1.0d</code> if the load cannot be determined on this platform
     */
    public double getLast15Minutes() {
        try {
            return sigarImpl.getLoadAverage()[2];
            
        } catch (SigarNotImplementedException e) {
            return NOT_IMPLEMENTED_LOAD_VALUE;
            
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    // -------
    // Implementation of the DynamicMBean interface
    // -------

    /*
     * (non-Javadoc)
     * 
     * @see DynamicMBean#getAttribute(String)
     */
    public Object getAttribute(String attr) throws AttributeNotFoundException {

        if (MBEAN_ATTR_LAST1MIN.getName().equals(attr)) {
            return new Double(getLastMinute());

        } else if (MBEAN_ATTR_LAST5MIN.getName().equals(attr)) {
            return new Double(getLastFiveMinutes());

        } else if (MBEAN_ATTR_LAST15MIN.getName().equals(attr)) {
            return new Double(getLast15Minutes());

        } else {
            throw new AttributeNotFoundException(attr);
        }
    }

    /*
     * (non-Javadoc)
     * 
     * @see DynamicMBean#setAttribute(Attribute)
     */
    public void setAttribute(Attribute attr) throws AttributeNotFoundException {
        throw new AttributeNotFoundException(attr.getName());
    }

    /*
     * (non-Javadoc)
     * 
     * @see DynamicMBean#invoke(String, Object[], String[])
     */
    public Object invoke(String actionName, Object[] params, String[] signature)
            throws ReflectionException {
        throw new ReflectionException(new NoSuchMethodException(actionName),
                actionName);
    }

    /*
     * (non-Javadoc)
     * 
     * @see DynamicMBean#getMBeanInfo()
     */
    public MBeanInfo getMBeanInfo() {
        return MBEAN_INFO;
    }
}
