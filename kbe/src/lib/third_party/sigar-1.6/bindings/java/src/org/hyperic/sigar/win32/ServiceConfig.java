/*
 * Copyright (c) 2006-2008 Hyperic, Inc.
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

package org.hyperic.sigar.win32;

import java.io.PrintStream;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;

public class ServiceConfig {

    // Start type
    /**
     * A device driver started by the system loader. This value is valid only for driver services.
     */
    public static final int START_BOOT   = 0x00000000;
    /**
     * A device driver started by the IoInitSystem function. This value is valid only for driver services.
     */
    public static final int START_SYSTEM   = 0x00000001;
    /**
     * A service started automatically by the service control manager during system startup.
     */
    public static final int START_AUTO     = 0x00000002;
    /**
     * A service started by the service control manager when a process calls the StartService function.
     */
    public static final int START_MANUAL   = 0x00000003;
    /**
     * A service that cannot be started.
     * Attempts to start the service result in the error code ERROR_SERVICE_DISABLED.
     */
    public static final int START_DISABLED = 0x00000004;

    /**
     * Driver service.
     */
    public static final int TYPE_KERNEL_DRIVER       = 0x00000001;
    /**
     * File system driver service.
     */
    public static final int TYPE_FILE_SYSTEM_DRIVER  = 0x00000002;

    public static final int TYPE_ADAPTER             = 0x00000004;

    public static final int TYPE_RECOGNIZER_DRIVER   = 0x00000008;
    /**
     * Service that runs in its own process.
     */
    public static final int TYPE_WIN32_OWN_PROCESS   = 0x00000010;
    /**
     * Service that shares a process with other services.
     */
    public static final int TYPE_WIN32_SHARE_PROCESS = 0x00000020;
    /**
     * The service can interact with the desktop.
     */
    public static final int TYPE_INTERACTIVE_PROCESS = 0x00000100;

    // Error control type
    /**
     * The startup (boot) program logs the error but continues the startup operation.
     */
    public static final int ERROR_IGNORE   = 0x00000000;
    /**
     * The startup program logs the error and displays a message box pop-up but continues the startup operation.
     */
    public static final int ERROR_NORMAL   = 0x00000001;
    /**
     * The startup program logs the error.
     * If the last-known good configuration is being started, the startup operation continues.
     * Otherwise, the system is restarted with the last-known-good configuration.
     */
    public static final int ERROR_SEVERE   = 0x00000002;
    /**
     * The startup program logs the error, if possible.
     * If the last-known good configuration is being started, the startup operation fails.
     * Otherwise, the system is restarted with the last-known good configuration.
     */
    public static final int ERROR_CRITICAL = 0x00000003;

    private static final String[] START_TYPES = {
        "Boot", "System", "Auto", "Manual", "Disabled"      
    };
    
    private static final String[] ERROR_TYPES = {
        "Ignore", "Normal", "Severe", "Critical"
    };

    int type;
    int startType;
    int errorControl;
    String path;
    String exe;
    String[] argv;
    String loadOrderGroup;
    int tagId;
    String[] dependencies;
    String startName;
    String displayName;
    String description;
    String password;
    String name;
    ServiceConfig() {}

    public ServiceConfig(String name) {
        this.name = name;
        this.type = TYPE_WIN32_OWN_PROCESS;
        this.startType = START_AUTO;
        this.errorControl = ERROR_NORMAL;
        //msdn docs:
        //"Specify an empty string if the account has no password
        //or if the service runs in the LocalService, NetworkService,
        //or LocalSystem account"
        this.password = "";
    }
    
    /**
     * @return Returns the path.
     */
    public String getPath() {
        return path;
    }
    /**
     * @param path The path to set.
     */
    public void setPath(String path) {
        this.path = path;
    }

    public String[] getArgv() {
        return this.argv;
    }

    public String getExe() {
        if (this.exe == null) {
            String[] argv = getArgv();
            if ((argv != null) && (argv.length != 0)) {
                this.exe = argv[0];
            }
        }
        return this.exe;
    }

    /**
     * @return Returns the dependencies.
     */
    public String[] getDependencies() {
        if (this.dependencies == null) {
            return new String[0];
        }
        return dependencies;
    }
    /**
     * @param dependencies The dependencies to set.
     */
    public void setDependencies(String[] dependencies) {
        this.dependencies = dependencies;
    }
    /**
     * @return Returns the displayName.
     */
    public String getDisplayName() {
        return displayName;
    }
    /**
     * @param displayName The displayName to set.
     */
    public void setDisplayName(String displayName) {
        this.displayName = displayName;
    }
    /**
     * @return Returns the errorControl, one of ERROR_* constants.
     */
    public int getErrorControl() {
        return errorControl;
    }
    /**
     * @param errorControl The errorControl to set, one of ERROR_* constants.
     */
    public void setErrorControl(int errorControl) {
        this.errorControl = errorControl;
    }
    
    public String getErrorControlString() {
        return ERROR_TYPES[getErrorControl()];
    }

    /**
     * @return Returns the loadOrderGroup.
     */
    public String getLoadOrderGroup() {
        return loadOrderGroup;
    }
    /**
     * @param loadOrderGroup The loadOrderGroup to set.
     */
    public void setLoadOrderGroup(String loadOrderGroup) {
        this.loadOrderGroup = loadOrderGroup;
    }
    /**
     * @return Returns the startName.
     */
    public String getStartName() {
        return startName;
    }
    /**
     * @param startName The startName to set.
     */
    public void setStartName(String startName) {
        this.startName = startName;
    }
    /**
     * @return Returns the startType, one of START_* constants.
     */
    public int getStartType() {
        return startType;
    }
    /**
     * @param startType The startType to set, one of START_* constants.
     */
    public void setStartType(int startType) {
        this.startType = startType;
    }
    
    public String getStartTypeString() {
        return START_TYPES[getStartType()];
    }

    /**
     * @return Returns the tagId.
     */
    public int getTagId() {
        return tagId;
    }
    /**
     * @param tagId The tagId to set.
     */
    public void setTagId(int tagId) {
        this.tagId = tagId;
    }
    /**
     * @return Returns the type, one of TYPE_* constants.
     */
    public int getType() {
        return type;
    }

    public List getTypeList() {
        ArrayList types = new ArrayList();

        if ((this.type & TYPE_KERNEL_DRIVER) != 0) {
            types.add("Kernel Driver");
        }
        if ((this.type & TYPE_FILE_SYSTEM_DRIVER) != 0) {
            types.add("File System Driver");
        }
        if ((this.type & TYPE_ADAPTER) != 0) {
            types.add("Adapter");
        }
        if ((this.type & TYPE_RECOGNIZER_DRIVER) != 0) {
            types.add("Recognizer Driver");
        }
        if ((this.type & TYPE_WIN32_OWN_PROCESS) != 0) {
            types.add("Own Process");
        }
        if ((this.type & TYPE_WIN32_SHARE_PROCESS) != 0) {
            types.add("Share Process");
        }
        if ((this.type & TYPE_INTERACTIVE_PROCESS) != 0) {
            types.add("Interactive Process");
        }
        
        return types;
    }

    /**
     * @param type The type to set, one of TYPE_* constants.
     */
    public void setType(int type) {
        this.type = type;
    }
    /**
     * @return Returns the description.
     */
    public String getDescription() {
        return description;
    }
    /**
     * @param description The description to set.
     */
    public void setDescription(String description) {
        this.description = description;
    }
    /**
     * @return Returns the password.
     */
    public String getPassword() {
        return password;
    }
    /**
     * @param password The password to set.
     */
    public void setPassword(String password) {
        this.password = password;
    }
    /**
     * @return Returns the name.
     */
    public String getName() {
        return name;
    }
    /**
     * @param name The name to set.
     */
    public void setName(String name) {
        this.name = name;
    }

    public void list(PrintStream out) throws Win32Exception {
        out.println("name..........[" + getName() + "]");
        out.println("display.......[" + getDisplayName() + "]");
        out.println("description...[" + getDescription() + "]");
        out.println("start type....[" + getStartTypeString() + "]");
        out.println("start name....[" + getStartName() + "]"); 

        out.println("type.........."  + getTypeList());
        out.println("path..........[" + getPath() + "]");
        out.println("exe...........[" + getExe() + "]");
        out.println("deps.........."  + Arrays.asList(getDependencies()));
        out.println("error ctl.....[" + getErrorControlString() + "]");
    }
}
