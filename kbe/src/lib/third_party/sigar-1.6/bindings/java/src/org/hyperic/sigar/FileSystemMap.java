/*
 * Copyright (c) 2006 Hyperic, Inc.
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

import java.util.HashMap;

import java.io.IOException;
import java.io.File;

/**
 * Helper class to build a map of mounted file systems.
 */
public class FileSystemMap extends HashMap {

    /**
     * FileSystemMap is read-only, this method is unsupported.
     * @see #init
     */
    public Object put(Object key, Object value) {
        throw new UnsupportedOperationException();
    }

    /**
     * Populate the map.  FileSystem.getDirName is used as the map key.
     */
    public void init(FileSystem[] fslist) {
        super.clear();

        for (int i=0; i<fslist.length; i++) {
            super.put(fslist[i].getDirName(), fslist[i]);
        }
    }

    public FileSystem getFileSystem(String name) {
        return (FileSystem)get(name);
    }

    public boolean isMounted(String name) {
        return (get(name) != null);
    }

    /**
     * Find the file system the given file or directory is within.
     * @return FileSystem or null if file or directory name does not exist.
     */
    public FileSystem getMountPoint(String name) {
        FileSystem fs = getFileSystem(name);
        if (fs != null) {
            return fs;
        }

        File dir = new File(name);
        if (!dir.exists()) {
            return null;
        }

        try {
            dir = dir.getCanonicalFile();
        } catch (IOException e) {
            throw new IllegalArgumentException(e.getMessage());
        }

        if (!dir.isDirectory()) {
            dir = dir.getParentFile();
        }

        do {
            fs = getFileSystem(dir.toString());
            if (fs != null) {
                return fs;
            }
            dir = dir.getParentFile();
        } while (dir != null);

        return null;
    }
}
