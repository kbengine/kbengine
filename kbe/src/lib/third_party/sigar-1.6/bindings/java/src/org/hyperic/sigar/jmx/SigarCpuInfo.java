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

/**
 * Sigar JMX MBean implementation for the <code>CpuInfo</code> information 
 * package. Provides an OpenMBean conform implementation.
 * 
 * @author Bjoern Martin
 * @since 1.5
 */
public class SigarCpuInfo extends AbstractMBean {

    private static final String MBEAN_TYPE = "CpuInfoList";

    private static final MBeanInfo MBEAN_INFO;

    private static final MBeanAttributeInfo MBEAN_ATTR_CPUINDEX;

    private static final MBeanAttributeInfo MBEAN_ATTR_CACHESIZE;

    private static final MBeanAttributeInfo MBEAN_ATTR_MHZ;

    private static final MBeanAttributeInfo MBEAN_ATTR_MODEL;

    private static final MBeanAttributeInfo MBEAN_ATTR_VENDOR;

    private static final MBeanConstructorInfo MBEAN_CONSTR_CPUINDEX;

    private static final MBeanConstructorInfo MBEAN_CONSTR_CPUINDEX_SIGAR;

    private static final MBeanParameterInfo MBEAN_PARAM_CPUINDEX;

    private static final MBeanParameterInfo MBEAN_PARAM_SIGAR;

    static {
        MBEAN_ATTR_CPUINDEX = new MBeanAttributeInfo("CpuIndex", "int",
                "The index of the CPU, typically starting at 0", true, false,
                false);
        MBEAN_ATTR_CACHESIZE = new MBeanAttributeInfo("CacheSize", "long",
                "The cache size of the CPU, in [byte]", true, false, false);
        MBEAN_ATTR_MHZ = new MBeanAttributeInfo("Mhz", "int",
                "The clock speed of the CPU, in [MHz]", true, false, false);
        MBEAN_ATTR_MODEL = new MBeanAttributeInfo("Model", "java.lang.String",
                "The CPU model reported", true, false, false);
        MBEAN_ATTR_VENDOR = new MBeanAttributeInfo("Vendor",
                "java.lang.String", "The CPU vendor reported", true, false,
                false);
        MBEAN_PARAM_CPUINDEX = new MBeanParameterInfo("cpuIndex", "int",
                "The index of the CPU to read data for. Must be >= 0 "
                        + "and not exceed the CPU count of the system");
        MBEAN_PARAM_SIGAR = new MBeanParameterInfo("sigar", Sigar.class
                .getName(), "The Sigar instance to use to fetch data from");
        MBEAN_CONSTR_CPUINDEX = new MBeanConstructorInfo(SigarCpuInfo.class
                .getName(),
                "Creates a new instance for the CPU index specified, "
                        + "using a new Sigar instance to fetch the data. "
                        + "Fails if the CPU index is out of range.",
                new MBeanParameterInfo[] { MBEAN_PARAM_CPUINDEX });
        MBEAN_CONSTR_CPUINDEX_SIGAR = new MBeanConstructorInfo(
                SigarCpuInfo.class.getName(),
                "Creates a new instance for the CPU index specified, "
                        + "using the Sigar instance specified to fetch the data. "
                        + "Fails if the CPU index is out of range.",
                new MBeanParameterInfo[] { MBEAN_PARAM_SIGAR,
                        MBEAN_PARAM_CPUINDEX });
        MBEAN_INFO = new MBeanInfo(
                SigarCpuInfo.class.getName(),
                "Sigar CPU Info MBean, provides overall information for a "
                        + "single CPU. This information only changes if, for example, "
                        + "a CPU is reducing its clock frequency or shutting down "
                        + "part of its cache. Subsequent requests are satisfied from "
                        + "within a cache that invalidates after 30 seconds.",
                new MBeanAttributeInfo[] { MBEAN_ATTR_CPUINDEX,
                        MBEAN_ATTR_CACHESIZE, MBEAN_ATTR_MHZ, MBEAN_ATTR_MODEL,
                        MBEAN_ATTR_VENDOR }, new MBeanConstructorInfo[] {
                        MBEAN_CONSTR_CPUINDEX, MBEAN_CONSTR_CPUINDEX_SIGAR },
                null, null);

    }

    /**
     * Index of the CPU processed by the instance.
     */
    private int cpuIndex;

    /**
     * Object name this instance will give itself when being registered to an 
     * MBeanServer.
     */
    private String objectName;

    /**
     * Creates a new instance for the CPU index specified, using a new Sigar 
     * instance to fetch the data. Fails if the CPU index is out of range.
     * 
     * @param cpuIndex The index of the CPU to read data for. Must be 
     *         <code>&gt;= 0</code> and not exceed the CPU count of the system.
     * 
     * @throws IllegalArgumentException If the CPU index is out of range or 
     *         an unexpected Sigar error occurs
     */
    public SigarCpuInfo(int index) throws IllegalArgumentException {
        this(new Sigar(), index);
    }

    /**
     * Creates a new instance for the CPU index specified, using the Sigar 
     * instance specified to fetch the data. Fails if the CPU index is out 
     * of range.
     * 
     * @param sigar The Sigar instance to use to fetch data from
     * @param cpuIndex The index of the CPU to read data for. Must be 
     *         <code>&gt;= 0</code> and not exceed the CPU count of the system.
     * 
     * @throws IllegalArgumentException If the CPU index is out of range or 
     *         an unexpected Sigar error occurs
     */
    public SigarCpuInfo(Sigar sigar, int index) {
        super(sigar, DEFAULT);

        // check index
        if (index < 0)
            throw new IllegalArgumentException(
                    "CPU index has to be non-negative: " + index);
        try {
            int cpuCount;
            if ((cpuCount = sigar.getCpuInfoList().length) < index)
                throw new IllegalArgumentException(
                        "CPU index out of range (found " + cpuCount
                                + " CPU(s)): " + index);

        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }

        // all fine
        this.cpuIndex = index;
        this.objectName = SigarInvokerJMX.DOMAIN_NAME + ":" + MBEAN_ATTR_TYPE
                + "=CpuInfo,"
                + MBEAN_ATTR_CPUINDEX.getName().substring(0, 1).toLowerCase()
                + MBEAN_ATTR_CPUINDEX.getName().substring(1) + "=" + cpuIndex;
    }

    /**
     * Object name this instance will give itself when being registered to an 
     * MBeanServer.
     */
    public String getObjectName() {
        return this.objectName;
    }

    /** 
     * @return The index of the CPU, typically starting at 0
     */
    public int getCpuIndex() {
        return this.cpuIndex;
    }

    /**
     * @return The cache size of the CPU, in [byte]
     */
    public long getCacheSize() {
        try {
            return sigar.getCpuInfoList()[this.cpuIndex].getCacheSize();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The clock speed of the CPU, in [MHz]
     */
    public int getMhz() {
        try {
            return sigar.getCpuInfoList()[this.cpuIndex].getMhz();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The CPU model reported
     */
    public String getModel() {
        try {
            return sigar.getCpuInfoList()[this.cpuIndex].getModel();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The CPU vendor reported
     */
    public String getVendor() {
        try {
            return sigar.getCpuInfoList()[this.cpuIndex].getVendor();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    // -------
    // Implementation of the DynamicMBean interface
    // -------

    /*
     * (non-Javadoc)
     * @see DynamicMBean#getAttribute(String)
     */
    public Object getAttribute(String attr) throws AttributeNotFoundException {

        if (MBEAN_ATTR_CACHESIZE.getName().equals(attr)) {
            return new Long(getCacheSize());

        } else if (MBEAN_ATTR_CPUINDEX.getName().equals(attr)) {
            return new Integer(getCpuIndex());

        } else if (MBEAN_ATTR_MHZ.getName().equals(attr)) {
            return new Integer(getMhz());

        } else if (MBEAN_ATTR_MODEL.getName().equals(attr)) {
            return getModel();

        } else if (MBEAN_ATTR_VENDOR.getName().equals(attr)) {
            return getVendor();

        } else {
            throw new AttributeNotFoundException(attr);
        }
    }

    /*
     * (non-Javadoc)
     * @see DynamicMBean#setAttribute(Attribute)
     */
    public void setAttribute(Attribute attr) throws AttributeNotFoundException {
        throw new AttributeNotFoundException(attr.getName());
    }

    /*
     * (non-Javadoc)
     * @see DynamicMBean#invoke(String, Object[], String[])
     */
    public Object invoke(String actionName, Object[] params, String[] signature)
            throws ReflectionException {
        throw new ReflectionException(new NoSuchMethodException(actionName),
                actionName);
    }

    /*
     * (non-Javadoc)
     * @see DynamicMBean#getMBeanInfo()
     */
    public MBeanInfo getMBeanInfo() {
        return MBEAN_INFO;
    }
}
