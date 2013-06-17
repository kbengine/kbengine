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

import java.io.File;

import org.hyperic.sigar.Sigar;
import org.hyperic.sigar.SigarLoader;

abstract class VMwareObject {
    public static final boolean LOADED;

    int ptr = 0;
    long ptr64 = 0;

    private static native boolean init(String lib);

    static {
        LOADED = loadLibraries();
    };

    private static boolean loadLibraries() {
        if (!(SigarLoader.IS_LINUX || SigarLoader.IS_WIN32)) {
            return false;
        }

        try {
            Sigar.load();
            String lib =
                VMControlLibrary.getSharedLibrary();

            if (lib == null) {
                return false;
            }

            if (SigarLoader.IS_WIN32) {
                File root = new File(lib).getParentFile();
                String[] libs = {
                    "libeay32.dll",
                    "ssleay32.dll"
                };

                for (int i=0; i<libs.length; i++) {
                    File ssllib =
                        new File(root, libs[i]);
                    if (!ssllib.exists()) {
                        return false;
                    }

                    try {
                        System.load(ssllib.getPath());
                    } catch (UnsatisfiedLinkError e) {
                        e.printStackTrace();
                        return false;
                    }
                }
            }

            return init(lib);
        } catch (Exception e) {
            //e.printStackTrace();
            return false;
        }
    }

    abstract void destroy();

    public void dispose() {
        if ((this.ptr != 0) || (this.ptr64 != 0)) {
            destroy();
            this.ptr = 0;
            this.ptr64 = 0;
        }
    }

    protected void finalize() {
        dispose();
    }
}
