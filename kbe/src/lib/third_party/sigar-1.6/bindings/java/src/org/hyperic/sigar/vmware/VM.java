/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
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

package org.hyperic.sigar.vmware;

public class VM extends VMwareObject {
    public static final int EXECUTION_STATE_ON        = 1;
    public static final int EXECUTION_STATE_OFF       = 2;
    public static final int EXECUTION_STATE_SUSPENDED = 3;
    public static final int EXECUTION_STATE_STUCK     = 4;
    public static final int EXECUTION_STATE_UNKNOWN   = 5;

    public static final String[] EXECUTION_STATES = {
        "INVALID", "ON", "OFF", "SUSPENDED", "STUCK", "UNKNOWN"
    };

    public static final int POWEROP_MODE_HARD = 1;
    public static final int POWEROP_MODE_SOFT = 2;
    public static final int POWEROP_MODE_TRYSOFT = 3;
    private static final int POWEROP_MODE_DEFAULT =
        POWEROP_MODE_TRYSOFT;

    public static final int PRODUCT_WS      = 1;
    public static final int PRODUCT_GSX     = 2;
    public static final int PRODUCT_ESX     = 3;
    public static final int PRODUCT_SERVER  = 4;
    public static final int PRODUCT_UNKNOWN = 5;

    public static final String GSX = "GSX";
    public static final String ESX = "ESX";
    public static final String SERVER = "Server";

    public static final String[] PRODUCTS = {
        "INVALID", "Workstation", GSX, ESX, SERVER, "UNKNOWN"
    };

    public static final int PLATFORM_WINDOWS = 1;
    public static final int PLATFORM_LINUX   = 2;
    public static final int PLATFORM_VMNIX   = 3;
    public static final int PLATFORM_UNKNOWN = 4;

    public static final String[] PLATFORMS = {
        "INVALID", "Windows", "Linux", "VmNix", "UNKNOWN"
    };

    public static final int PRODINFO_PRODUCT          = 1;
    public static final int PRODINFO_PLATFORM         = 2;
    public static final int PRODINFO_BUILD            = 3;
    public static final int PRODINFO_VERSION_MAJOR    = 4;
    public static final int PRODINFO_VERSION_MINOR    = 5;
    public static final int PRODINFO_VERSION_REVISION = 6;

    public static final int PERM_READ    = 4;
    public static final int PERM_WRITE   = 2;
    public static final int PERM_EXECUTE = 1;

    native void destroy();

    private native void create();

    private native void connect(ConnectParams params, String config, int mks)
        throws VMwareException;

    public void connect(ConnectParams params, String config, boolean mks)
        throws VMwareException {
        connect(params, config, mks ? 1 : 0);
    }

    public void connect(ConnectParams params, String config)
        throws VMwareException {
        connect(params, config, 0);
    }

    public native void disconnect();

    public native boolean isConnected();

    public native int getExecutionState()
        throws VMwareException;

    public native int getRemoteConnections()
        throws VMwareException;

    public native int getUptime()
        throws VMwareException;

    public native int getHeartbeat()
        throws VMwareException;

    public native int getToolsLastActive()
        throws VMwareException;

    public native String getRunAsUser()
        throws VMwareException;

    public native int getPermissions()
        throws VMwareException;

    public String getPermissionsString() {
        char[] perms = { '-', '-', '-' };

        try {
            int bits = getPermissions();
            if ((bits & PERM_READ) != 0) {
                perms[0] = 'r';
            }
            if ((bits & PERM_WRITE) != 0) {
                perms[1] = 'w';
            }
            if ((bits & PERM_EXECUTE) != 0) {
                perms[2] = 'x';
            }
        } catch (VMwareException e) {}

        return new String(perms);
    }

    private boolean checkPermission(int perm) {
        try {
            return (getPermissions() & perm) != 0;
        } catch (VMwareException e) {
            return false;
        }
    }

    public boolean canRead() {
        return checkPermission(PERM_READ);
    }

    public boolean canWrite() {
        return checkPermission(PERM_WRITE);
    }

    public boolean canExecute() {
        return checkPermission(PERM_EXECUTE);
    }

    public native String getConfig(String key)
        throws VMwareException;

    public native String getResource(String key)
        throws VMwareException;

    public native String getGuestInfo(String key)
        throws VMwareException;

    public native void setGuestInfo(String key, String value)
        throws VMwareException;

    public native int getProductInfo(int type)
        throws VMwareException;

    public native long getPid()
        throws VMwareException;

    public native int getId()
        throws VMwareException;

    public String getVersion() throws VMwareException {
        return
            getProductInfo(PRODINFO_VERSION_MAJOR) + "." +
            getProductInfo(PRODINFO_VERSION_MINOR);
    }

    public String getFullVersion() throws VMwareException {
        return
            getVersion() + "." +
            getProductInfo(PRODINFO_VERSION_REVISION) + "." +
            getProductInfo(PRODINFO_BUILD);
    }

    private String getConfigEx(String key) {
        try {
            return getConfig(key);
        } catch (VMwareException e) {
            return null;
        }
    }

    public String getDisplayName() {
        return getConfigEx("displayName");
    }

    public String getGuestOS() {
        return getConfigEx("guestOS");
    }

    public String getMemSize() {
        return getConfigEx("memsize");
    }

    public String getProductName() {
        try {
            int info = getProductInfo(PRODINFO_PRODUCT);
            return PRODUCTS[info] + " " + getVersion();
        } catch (VMwareException e) {
            return null;
        }
    }

    public String getProductPlatform() {
        try {
            int info = getProductInfo(PRODINFO_PLATFORM);
            return PLATFORMS[info];
        } catch (VMwareException e) {
            return null;
        }
    }

    private boolean isState(int state) {
        try {
            return getExecutionState() == state;
        } catch (VMwareException e) {
            return false;
        }
    }

    public boolean isOn() {
        return isState(EXECUTION_STATE_ON);
    }

    public boolean isOff() {
        return isState(EXECUTION_STATE_OFF);
    }

    public boolean isSuspended() {
        return isState(EXECUTION_STATE_SUSPENDED);
    }

    public boolean isStuck() {
        return isState(EXECUTION_STATE_STUCK);
    }

    public boolean isESX() {
        try {
            return getProductInfo(PRODINFO_PRODUCT) == PRODUCT_ESX;
        } catch (VMwareException e) {
            return false;
        }
    }

    public boolean isGSX() {
        try {
            return getProductInfo(PRODINFO_PRODUCT) == PRODUCT_GSX;
        } catch (VMwareException e) {
            return false;
        }
    }

    public native void start(int mode)
        throws VMwareException;

    public void start() throws VMwareException {
        start(POWEROP_MODE_DEFAULT);
    }

    public native void stop(int mode)
        throws VMwareException;

    public void stop() throws VMwareException {
        stop(POWEROP_MODE_DEFAULT);
    }

    public native void reset(int mode)
        throws VMwareException;

    public void reset() throws VMwareException {
        reset(POWEROP_MODE_DEFAULT);
    }

    public native void suspend(int mode)
        throws VMwareException;

    public void suspend() throws VMwareException {
        suspend(POWEROP_MODE_DEFAULT);
    }

    public void resume(int mode)
        throws VMwareException {

        int state = getExecutionState();
        if (state != EXECUTION_STATE_SUSPENDED) {
            throw new VMwareException("VM state is not suspended: " +
                                      EXECUTION_STATES[state]);
        }
        start(mode);
    }

    public void resume() throws VMwareException {
        resume(POWEROP_MODE_DEFAULT);
    }

    private native void createNamedSnapshot(String name,
                                            String description,
                                            boolean quiesce,
                                            boolean memory) throws VMwareException;

    private native void createDefaultSnapshot() throws VMwareException;

    public void createSnapshot(String name,
                               String description,
                               boolean quiesce,
                               boolean memory) throws VMwareException {

        if (isESX()) {
            createNamedSnapshot(name, description, quiesce, memory);
        }
        else {
            createDefaultSnapshot();
        }
    }

    public native void revertToSnapshot() throws VMwareException;

    public native void removeAllSnapshots() throws VMwareException;

    public native boolean hasSnapshot() throws VMwareException;

    public native void saveScreenshot(String name)
        throws VMwareException;

    public native void deviceConnect(String device)
        throws VMwareException;

    public native void deviceDisconnect(String device)
        throws VMwareException;

    public native boolean deviceIsConnected(String device)
        throws VMwareException;

    public VM() {
        create();
    }
}
