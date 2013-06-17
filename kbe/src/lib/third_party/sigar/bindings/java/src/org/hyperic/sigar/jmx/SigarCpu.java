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
 * Sigar JMX MBean implementation for the <code>Cpu</code> information
 * package. Provides an OpenMBean conform implementation.
 * 
 * @author Bjoern Martin
 * @since 1.5
 */
public class SigarCpu extends AbstractMBean {

    private static final String MBEAN_TYPE = "CpuList";

    private static final MBeanInfo MBEAN_INFO;

    private static final MBeanAttributeInfo MBEAN_ATTR_CPUINDEX;

    private static final MBeanAttributeInfo MBEAN_ATTR_IDLE;

    private static final MBeanAttributeInfo MBEAN_ATTR_NICE;

    private static final MBeanAttributeInfo MBEAN_ATTR_SYS;

    private static final MBeanAttributeInfo MBEAN_ATTR_TOTAL;

    private static final MBeanAttributeInfo MBEAN_ATTR_USER;

    private static final MBeanAttributeInfo MBEAN_ATTR_WAIT;

    private static final MBeanConstructorInfo MBEAN_CONSTR_CPUINDEX;

    private static final MBeanConstructorInfo MBEAN_CONSTR_CPUINDEX_SIGAR;

    private static MBeanParameterInfo MBEAN_PARAM_CPUINDEX;

    private static MBeanParameterInfo MBEAN_PARAM_SIGAR;

    static {
        MBEAN_ATTR_CPUINDEX = new MBeanAttributeInfo("CpuIndex", "int",
                "The index of the CPU, typically starting at 0", true, false,
                false);
        MBEAN_ATTR_IDLE = new MBeanAttributeInfo("Idle", "long",
                "The idle time of the CPU, in [ms]", true, false, false);
        MBEAN_ATTR_NICE = new MBeanAttributeInfo("Nice", "long",
                "The time of the CPU spent on nice priority, in [ms]", true,
                false, false);
        MBEAN_ATTR_SYS = new MBeanAttributeInfo("Sys", "long",
                "The time of the CPU used by the system, in [ms]", true, false,
                false);
        MBEAN_ATTR_TOTAL = new MBeanAttributeInfo("Total", "long",
                "The total time of the CPU, in [ms]", true, false, false);
        MBEAN_ATTR_USER = new MBeanAttributeInfo("User", "long",
                "The time of the CPU used by user processes, in [ms]", true,
                false, false);
        MBEAN_ATTR_WAIT = new MBeanAttributeInfo("Wait", "long",
                "The time the CPU had to wait for data to be loaded, in [ms]",
                true, false, false);
        MBEAN_PARAM_CPUINDEX = new MBeanParameterInfo("cpuIndex", "int",
                "The index of the CPU to read data for. Must be >= 0 "
                        + "and not exceed the CPU count of the system");
        MBEAN_PARAM_SIGAR = new MBeanParameterInfo("sigar", Sigar.class
                .getName(), "The Sigar instance to use to fetch data from");
        MBEAN_CONSTR_CPUINDEX = new MBeanConstructorInfo(SigarCpu.class
                .getName(),
                "Creates a new instance for the CPU index specified, "
                        + "using a new Sigar instance to fetch the data. "
                        + "Fails if the CPU index is out of range.",
                new MBeanParameterInfo[] { MBEAN_PARAM_CPUINDEX });
        MBEAN_CONSTR_CPUINDEX_SIGAR = new MBeanConstructorInfo(
                SigarCpu.class.getName(),
                "Creates a new instance for the CPU index specified, "
                        + "using the Sigar instance specified to fetch the data. "
                        + "Fails if the CPU index is out of range.",
                new MBeanParameterInfo[] { MBEAN_PARAM_SIGAR,
                        MBEAN_PARAM_CPUINDEX });
        MBEAN_INFO = new MBeanInfo(
                SigarCpu.class.getName(),
                "Sigar CPU MBean. Provides raw timing data for a single "
                        + "CPU. The data is cached for 500ms, meaning each request "
                        + "(and as a result each block request to all parameters) "
                        + "within half a second is satisfied from the same dataset.",
                new MBeanAttributeInfo[] { MBEAN_ATTR_CPUINDEX,
                        MBEAN_ATTR_IDLE, MBEAN_ATTR_NICE, MBEAN_ATTR_SYS,
                        MBEAN_ATTR_TOTAL, MBEAN_ATTR_USER, MBEAN_ATTR_WAIT },
                new MBeanConstructorInfo[] { MBEAN_CONSTR_CPUINDEX,
                        MBEAN_CONSTR_CPUINDEX_SIGAR }, null, null);

    }

    /**
     * Index of the CPU processed by the instance.
     */
    private final int cpuIndex;

    /**
     * Object name this instance will give itself when being registered to an
     * MBeanServer.
     */
    private final String objectName;

    /**
     * Creates a new instance for the CPU index specified, using a new Sigar
     * instance to fetch the data. Fails if the CPU index is out of range.
     * 
     * @param cpuIndex 
     *      The index of the CPU to read data for. Must be <code>&gt;= 0</code> 
     *      and not exceed the CPU count of the system.
     * 
     * @throws IllegalArgumentException
     *      If the CPU index is out of range or an unexpected Sigar error
     *      occurs.
     */
    public SigarCpu(int cpuIndex) throws IllegalArgumentException {
        this(new Sigar(), cpuIndex);
    }

    /**
     * Creates a new instance for the CPU index specified, using the Sigar
     * instance specified to fetch the data. Fails if the CPU index is out of
     * range.
     * 
     * @param sigar
     *            The Sigar instance to use to fetch data from
     * @param cpuIndex
     *            The index of the CPU to read data for. Must be
     *            <code>&gt;= 0</code> and not exceed the CPU count of the
     *            system.
     * 
     * @throws IllegalArgumentException
     *             If the CPU index is out of range or an unexpected Sigar error
     *             occurs
     */
    public SigarCpu(Sigar sigar, int cpuIndex) throws IllegalArgumentException {
        super(sigar, CACHED_500MS);

        // check index
        if (cpuIndex < 0)
            throw new IllegalArgumentException(
                    "CPU index has to be non-negative: " + cpuIndex);
        try {
            int cpuCount;
            if ((cpuCount = sigar.getCpuList().length) < cpuIndex)
                throw new IllegalArgumentException(
                        "CPU index out of range (found " + cpuCount
                                + " CPU(s)): " + cpuIndex);

        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }

        // all fine
        this.cpuIndex = cpuIndex;
        this.objectName = SigarInvokerJMX.DOMAIN_NAME + ":" + MBEAN_ATTR_TYPE
                + "=Cpu,"
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
     * @return The idle time of the CPU, in [ms]
     */
    public long getIdle() {
        try {
            return sigar.getCpuList()[this.cpuIndex].getIdle();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The time of the CPU spent on nice priority, in [ms]
     */
    public long getNice() {
        try {
            return sigar.getCpuList()[this.cpuIndex].getNice();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The time of the CPU used by the system, in [ms]
     */
    public long getSys() {
        try {
            return sigar.getCpuList()[this.cpuIndex].getSys();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The total time of the CPU, in [ms]
     */
    public long getTotal() {
        try {
            return sigar.getCpuList()[this.cpuIndex].getTotal();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The time of the CPU used by user processes, in [ms]
     */
    public long getUser() {
        try {
            return sigar.getCpuList()[this.cpuIndex].getUser();
        } catch (SigarException e) {
            throw unexpectedError(MBEAN_TYPE, e);
        }
    }

    /**
     * @return The time the CPU had to wait for data to be loaded, in [ms]
     */
    public long getWait() {
        try {
            return sigar.getCpuList()[this.cpuIndex].getWait();
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

        if (MBEAN_ATTR_CPUINDEX.getName().equals(attr)) {
            return new Integer(getCpuIndex());

        } else if (MBEAN_ATTR_IDLE.getName().equals(attr)) {
            return new Long(getIdle());

        } else if (MBEAN_ATTR_NICE.getName().equals(attr)) {
            return new Long(getNice());

        } else if (MBEAN_ATTR_SYS.getName().equals(attr)) {
            return new Long(getSys());

        } else if (MBEAN_ATTR_TOTAL.getName().equals(attr)) {
            return new Long(getTotal());

        } else if (MBEAN_ATTR_USER.getName().equals(attr)) {
            return new Long(getUser());

        } else if (MBEAN_ATTR_WAIT.getName().equals(attr)) {
            return new Long(getWait());

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
