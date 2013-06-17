/*
 * Copyright (c) 2009 Hyperic, Inc.
 * Copyright (c) 2009 SpringSource, Inc.
 * Copyright (c) 2009-2010 VMware, Inc.
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

package org.hyperic.jni;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.FileReader;
import java.io.Reader;
import java.util.Arrays;

import org.apache.tools.ant.Task;
import org.apache.tools.ant.BuildException;

public class ArchNameTask extends Task {

    public void execute() throws BuildException {
        String osArch = System.getProperty("os.arch");
        String osVers = System.getProperty("os.version");

        if (getProject().getProperty("jni.dmalloc") != null) {
            ArchName.useDmalloc = true;
        }

        String archName;

        try {
            archName = ArchName.getName();
        } catch (ArchNotSupportedException e) {
            //ok: can still compile *.java
            System.out.println(e.getMessage());
            return;
        }

        System.out.println(archName);
        getProject().setProperty("jni.libarch", archName);
        getProject().setProperty("jni.libpre",
                                 ArchLoader.getLibraryPrefix());
        getProject().setProperty("jni.libext",
                                 ArchLoader.getLibraryExtension());

        String compiler;
        if (ArchLoader.IS_WIN32) {
            compiler = "msvc";
        }
        else if (ArchLoader.IS_HPUX) {
            compiler = "hp";
        }
        else if (ArchLoader.IS_AIX) {
            compiler = "xlc_r";
        }
        else {
            compiler = "gcc";
            getProject().setProperty("jni.compiler.isgcc", "true");
        }

        getProject().setProperty("jni.compiler", compiler);

        if (ArchName.is64()) {
            getProject().setProperty("jni.arch64", "true");
            if (ArchLoader.IS_LINUX) {
                if (!osArch.equals("ia64")) {
                    getProject().setProperty("jni.gccm", "-m64");
                }
            }
        }
        else {
            if (ArchLoader.IS_LINUX && osArch.equals("s390")) {
                //gcc defaults to m64 on s390x platforms
                getProject().setProperty("jni.gccm", "-m31");
            }
        }

        if (ArchLoader.IS_DARWIN) {
            //default to most recent SDK
            //MacOSX10.3.9.sdk, MacOSX10.4u.sdk, MacOSX10.5.sdk,etc.
            File[] sdks =
                new File("/Developer/SDKs").listFiles(new FileFilter() {
                    public boolean accept(File file) {
                        String name = file.getName();
                        return
                            name.startsWith("MacOSX10.") &&
                            name.endsWith(".sdk");
                    }
                });
            if (sdks != null) {
                Arrays.sort(sdks);
                String prop = "uni.sdk";
                String sdk = getProject().getProperty(prop);
                String defaultMin = "10.3";

                if (sdk == null) {
                    int ix = sdks.length-1;
                    sdk = sdks[ix].getPath();
                    if ((sdk.indexOf("10.6") != -1) && (ix > 0)) {
                        sdk = sdks[ix-1].getPath(); //downgrade due to 64-bit ppc issues XXX
                        defaultMin = "10.5";
                    }
                    getProject().setProperty(prop, sdk);
                }

                String version = osVers.substring(0, 4);
                int minorVers = Integer.parseInt(osVers.substring(3,4));
                boolean usingLatestSDK = (sdk.indexOf(version) != -1);

                System.out.println("Using SDK=" + sdk);
                if ((minorVers >= 6) && ArchName.is64() && usingLatestSDK) {
                    //64-bit 10.6 sdk does not support ppc64
                    //switch from universal build to i386 only
                    getProject().setProperty("jni.cc", "jni-cc");
                    getProject().setProperty("uni.arch", "i386");
                    System.out.println("Note: SDK version does not support ppc64");
                }

                prop = "osx.min";
                String min = getProject().getProperty(prop);
                if (min == null) {
                    min = defaultMin;
                    getProject().setProperty(prop, min);
                }
                System.out.println("Using -mmacosx-version-min=" + min);
            }
        }
        getProject().setProperty("jni.scmrev", getSourceRevision());

        //jni.javahome required to find include/jni.h
        String home = getProject().getProperty("jni.javahome");
        if (home == null) {
            home = System.getProperty("java.home");
        }
        File dir = new File(home);
        if (!new File(dir, "include").exists()) {
            dir = dir.getParentFile(); //was /jre
        }
        getProject().setProperty("jni.javahome", dir.getPath());
    }

    //XXX source rev stuff should be in another task
    private String readLine(String filename) {
        Reader reader = null;
        try {
            reader = new FileReader(filename);
            return new BufferedReader(reader).readLine();
        } catch (Exception e) {
        } finally {
            if (reader != null) {
                try { reader.close(); } catch (Exception e) {}
            }
        }
        return null;
    }

    private String getSourceRevision() {
        final String exported = "exported";
        String sha1 = getGitSourceRevision();
        if (sha1 == null) {
            return exported;
        }
        else {
            return sha1;
        }
    }

    //same as: git rev-parse --short HEAD
    //same as: (cd .git && cat HEAD | awk '{print $2}' | xargs cat | cut -b 1-7)
    private String getGitSourceRevision() {
        String git = getProject().getProperty("jni.git");
        if (git == null) {
            git = ".git";
        }
        if (new File(git).exists()) {
            String head = readLine(git + "/HEAD");

            if (head != null) {
                String sha1;
                final String refp = "ref: ";
                if (head.startsWith(refp)) {
                    //branch
                    String ref = head.substring(refp.length()).trim();
                    sha1 = readLine(git + "/" + ref);
                }
                else {
                    //git checkout -f origin/branch-name (no branch)
                    sha1 = head;
                }
                return sha1.substring(0, 7);
            }
        }
        return null;
    }
}
