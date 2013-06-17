/*
 * Copyright (c) 2008 Hyperic, Inc.
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

import java.util.LinkedHashMap;
import java.util.Map;

public class FileVersion {
    private int
        product_major, product_minor, product_build, product_revision;
    private int
        file_major, file_minor, file_build, file_revision;
    private Map string_file_info = new LinkedHashMap();

    native boolean gather(String name);

    FileVersion() {}

    public int getProductMajor() {
        return this.product_major;
    }

    public int getProductMinor() {
        return this.product_minor;
    }

    public int getProductBuild() {
        return this.product_build;
    }

    public int getProductRevision() {
        return this.product_revision;
    }

    public int getFileMajor() {
        return this.file_major;
    }

    public int getFileMinor() {
        return this.file_minor;
    }

    public int getFileBuild() {
        return this.file_build;
    }

    public int getFileRevision() {
        return this.file_revision;
    }

    public Map getInfo() {
        return this.string_file_info;
    }

    private String toVersion(int major, int minor,
                             int build, int revision) {
        return
            major + "." +
            minor + "." +
            build + "." +
            revision;
    }

    public String getProductVersion() {
        return toVersion(this.product_major,
                         this.product_minor,
                         this.product_build,
                         this.product_revision);
    }

    public String getFileVersion() {
        return toVersion(this.file_major,
                         this.file_minor,
                         this.file_build,
                         this.file_revision);
    }
}
