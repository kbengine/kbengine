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
import javax.management.MBeanException;
import javax.management.MBeanInfo;
import javax.management.MBeanParameterInfo;
import javax.management.ReflectionException;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarException;

/**
 * Sigar JMX MBean implementation for the <code>Mem</code> information
 * package. Provides an OpenMBean conform implementation.
 * 
 * @author Bjoern Martin
 * @since 1.5
 */
public class SigarMem extends AbstractMBean {

    private static final String MBEAN_TYPE = "Mem";

    private static final MBeanInfo MBEAN_INFO;

    private static final MBeanAttributeInfo MBEAN_ATTR_ACTUAL_FREE;

    private static final MBeanAttributeInfo MBEAN_ATTR_ACTUAL_USED;

    private static final MBeanAttributeInfo MBEAN_ATTR_FREE;

    private static final MBeanAttributeInfo MBEAN_ATTR_RAM;

    private static final MBeanAttributeInfo MBEAN_ATTR_TOTAL;

    private static final MBeanAttributeInfo MBEAN_ATTR_USED;

    private static final MBeanConstructorInfo MBEAN_CONSTR_SIGAR;

    private static MBeanParameterInfo MBEAN_PARAM_SIGAR;

    static {
        MBEAN_ATTR_ACTUAL_FREE = new MBeanAttributeInfo("ActualFree", "long",
                "TODO add proper description here", true, false, false);
        MBEAN_ATTR_ACTUAL_USED = new MBeanAttributeInfo("ActualUsed", "long",
                "TODO add proper description here", true, false, false);
        MBEAN_ATTR_FREE = new MBeanAttributeInfo("Free", "long",
                "TODO add proper description here", true, false, false);
        MBEAN_ATTR_RAM = new MBeanAttributeInfo("Ram", "long",
                "TODO add proper description here", true, false, false);
        MBEAN_ATTR_TOTAL = new MBeanAttributeInfo("Total", "long",
                "TODO add proper description here", true, false, false);
        MBEAN_ATTR_USED = new MBeanAttributeInfo("Used", "long",
                "TODO add proper description here", true, false, false);
        MBEAN_PARAM_SIGAR = new MBeanParameterInfo("sigar", Sigar.class
                .getName(), "The Sigar instance to use to fetch data from");
        MBEAN_CONSTR_SIGAR = new MBeanConstructorInfo(SigarMem.class.getName(),
                "Creates a new instance, using the Sigar instance "
                        + "specified to fetch the data.",
                new MBeanParameterInfo[] { MBEAN_PARAM_SIGAR });
        MBEAN_INFO = new MBeanInfo(
                SigarMem.class.getName(),
                "Sigar Memory MBean, provides raw data for the physical "
                        + "memory installed on the system. Uses an internal cache "
                        + "that invalidates within 500ms, allowing for bulk request "
                        + "being satisfied with a single dataset fetch.",
                new MBeanAttributeInfo[] { MBEAN_ATTR_ACTUAL_FREE,
                        MBEAN_ATTR_ACTUAL_USED, MBEAN_ATTR_FREE,
                        MBEAN_ATTR_RAM, MBEAN_ATTR_TOTAL, MBEAN_ATTR_USED },
                new MBeanConstructorInfo[] { MBEAN_CONSTR_SIGAR }, null, null);

    }

    /**
     * Object name this instance will give itself when being registered to an
     * MBeanServer.
     */
    private final String objectName;

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
    public SigarMem(Sigar sigar) throws IllegalArgumentException {
        super(sigar, CACHED_500MS);

        this.objectName = SigarInvokerJMX.DOMAIN_NAME + ":" + MBEAN_ATTR_TYPE
                + "=Memory";
    }

    /**
     * Object name this instance will give itself when being registered to an
     * MBeanServer.
     */
    public String getObjectName() {
        return this.objectName;
    }

    /**
     * @return The actual amount of free physical memory, in [bytes]
     */
    public long getActualFree() {
        try {
            return sigar.getMem().getActualFree();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The actual amount of physical memory used, in [bytes]
     */
    public long getActualUsed() {
        try {
            return sigar.getMem().getActualUsed();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The amount of free physical memory, in [bytes]
     */
    public long getFree() {
        try {
            return sigar.getMem().getFree();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The amount of physical memory, in [bytes]
     */
    public long getRam() {
        try {
            return sigar.getMem().getRam();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The total amount of physical memory, in [bytes]
     */
    public long getTotal() {
        try {
            return sigar.getMem().getTotal();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The amount of physical memory in use, in [bytes]
     */
    public long getUsed() {
        try {
            return sigar.getMem().getUsed();
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
     * @see javax.management.DynamicMBean#getAttribute(java.lang.String)
     */
    public Object getAttribute(String attr) throws AttributeNotFoundException,
            MBeanException, ReflectionException {
        if (MBEAN_ATTR_ACTUAL_FREE.getName().equals(attr)) {
            return new Long(getActualFree());

        } else if (MBEAN_ATTR_ACTUAL_USED.getName().equals(attr)) {
            return new Long(getActualUsed());

        } else if (MBEAN_ATTR_FREE.getName().equals(attr)) {
            return new Long(getFree());

        } else if (MBEAN_ATTR_RAM.getName().equals(attr)) {
            return new Long(getRam());

        } else if (MBEAN_ATTR_TOTAL.getName().equals(attr)) {
            return new Long(getTotal());

        } else if (MBEAN_ATTR_USED.getName().equals(attr)) {
            return new Long(getUsed());

        } else {
            throw new AttributeNotFoundException(attr);
        }
    }

    /*
     * (non-Javadoc)
     * @see javax.management.DynamicMBean#setAttribute(javax.management.Attribute)
     */
    public void setAttribute(Attribute attr) throws AttributeNotFoundException {
        throw new AttributeNotFoundException(attr.getName());
    }

    /*
     * (non-Javadoc)
     * @see javax.management.DynamicMBean#invoke(java.lang.String,
     *      java.lang.Object[], java.lang.String[])
     */
    public Object invoke(String actionName, Object[] params, String[] signature)
            throws ReflectionException {
        throw new ReflectionException(new NoSuchMethodException(actionName),
                actionName);
    }

    /*
     * (non-Javadoc)
     * @see javax.management.DynamicMBean#getMBeanInfo()
     */
    public MBeanInfo getMBeanInfo() {
        return MBEAN_INFO;
    }
}
