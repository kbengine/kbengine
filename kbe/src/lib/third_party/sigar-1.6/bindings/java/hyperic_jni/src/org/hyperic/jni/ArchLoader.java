/*
 * Copyright (c) 2009 Hyperic, Inc.
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

import java.io.File;
import java.util.StringTokenizer;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.URLDecoder;

public class ArchLoader {
    private Object loadLock = new Object();

    private boolean loaded = false;

    private final static String osName = System.getProperty("os.name");

    public final static boolean IS_WIN32   = osName.startsWith("Windows");
    public final static boolean IS_AIX     = osName.equals("AIX");
    public final static boolean IS_HPUX    = osName.equals("HP-UX");
    public final static boolean IS_SOLARIS = osName.equals("SunOS");
    public final static boolean IS_LINUX   = osName.equals("Linux");
    public final static boolean IS_DARWIN  = osName.equals("Mac OS X") || osName.equals("Darwin");
    public final static boolean IS_OSF1    = osName.equals("OSF1");
    public final static boolean IS_FREEBSD = osName.equals("FreeBSD");
    public final static boolean IS_NETWARE = osName.equals("NetWare");

    private String packageName;
    private String name;
    private String resourcePath;
    private Class loaderClass;
    private String jarName;
    private String libName = null;
    private File nativeLibrary;
    private String version;

    public ArchLoader() { }

    public ArchLoader(Class loaderClass) {
        setLoaderClass(loaderClass); //e.g. Sigar.class

        //argh. possible for loaderClass.getPackage()
        //to return null depending on ClassLoader
        String pname = loaderClass.getName();
        int ix = pname.lastIndexOf(".");
        pname = pname.substring(0, ix);
        setPackageName(pname); //e.g. org.hyperic.sigar

        ix = pname.lastIndexOf(".");
        setName(pname.substring(ix+1)); //e.g. sigar

        setJarName(getName() + ".jar");

        setResourcePath(toResName(pname)); //e.g. org.hyperic/sigar
    }

    public Class getLoaderClass() {
        return this.loaderClass;
    }

    public void setLoaderClass(Class value) {
        this.loaderClass = value;
    }

    public ClassLoader getClassLoader() {
        return getLoaderClass().getClassLoader();
    }

    public String getName() {
        return this.name;
    }

    public void setName(String value) {
        this.name = value;
    }

    public String getPackageName() {
        return this.packageName;
    }

    public void setPackageName(String value) {
        this.packageName = value;
    }

    public String getResourcePath() {
        return this.resourcePath;
    }

    public void setResourcePath(String value) {
        this.resourcePath = value;
    }

    public String getJarName() {
        return this.jarName;
    }

    public void setJarName(String value) {
        this.jarName = value;
    }

    public String getLibName() {
        return this.libName;
    }

    public void setLibName(String value) {
        this.libName = value;
    }

    public String getArchLibName()
        throws ArchNotSupportedException {

        return getName() + "-" + ArchName.getName();
    }

    public String getDefaultLibName()
        throws ArchNotSupportedException  {

        return
            System.getProperty(getPackageName() + ".libname",
                               //e.g. javasigar-x86-linux
                               "java" + getArchLibName());
    }

    public File getNativeLibrary() {
        return this.nativeLibrary;
    }

    private String toResName(String name) {
        StringBuffer sb = new StringBuffer(name);
        for (int i=0; i < sb.length(); i++) {
            if (sb.charAt(i) == '.') {
                sb.setCharAt(i, '/');
            }
        }
        return sb.toString();
    }

    public static String getLibraryPrefix() {
        if (IS_WIN32 || IS_NETWARE) {
            return "";
        }

        return "lib";
    }

    public static String getLibraryExtension() {
        if (IS_WIN32) {
            return ".dll";
        }

        if (IS_NETWARE) {
            return ".nlm";
        }

        if (IS_DARWIN) {
            return ".dylib";
        }

        if (IS_HPUX) {
            return ".sl";
        }

        return ".so"; 
    }

    public String getLibraryName()
        throws ArchNotSupportedException {
        String libName;

        if ((libName = getLibName()) == null) {
            libName = getDefaultLibName();
            setLibName(libName);
        }

        String prefix = getLibraryPrefix();
        String ext = getLibraryExtension();

        return prefix + libName + ext;
    }

    public String getVersionedLibraryName() {

        if (this.version == null) {
            return null;
        }
        try {
            getLibraryName();
        } catch (ArchNotSupportedException e) {
            return null;
        }
        String prefix = getLibraryPrefix();
        String ext = getLibraryExtension();

        return prefix + this.libName + '-' + this.version + ext;
    }

    private boolean isJarURL(URL url) {
        if (url == null) {
            return false;
        }
        String name = url.getFile();

        String jarName = getJarName();
        if (name.indexOf(jarName) != -1) {
            return true;
        }

        int ix = jarName.indexOf(".jar");
        if (ix != -1) {
            //check for a versioned name-x.x.jar
            jarName = jarName.substring(0, ix) + "-";

            //lastIndex else could match "sigar-bin/"
            ix = name.lastIndexOf(jarName);
            if (ix != -1) {
                jarName = name.substring(ix);
                ix = jarName.indexOf(".jar");
                if (ix == -1) {
                    return false;
                }
                this.version =
                    jarName.substring(jarName.indexOf('-')+1, ix);
                jarName = jarName.substring(0, ix+4);
                setJarName(jarName); //for future reference
                return true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }

    public String findJarPath(String libName)
        throws ArchLoaderException {
        return findJarPath(libName, true);
    }

    private String findJarPath(String libName, boolean isRequired)
        throws ArchLoaderException {
        /*
         * native libraries should be installed along side
         * ${this.name}.jar, try to find where ${this.name}.jar
         * is on disk and use that path.
         */
        if (getJarName() == null) {
            throw new ArchLoaderException("jarName is null");
        }

        String path = getResourcePath();
        ClassLoader loader = getClassLoader();
        URL url = loader.getResource(path);

        if (!isJarURL(url)) {
            url = null;
        }

        if ((url == null) && (loader instanceof URLClassLoader)) {
            URL[] urls = ((URLClassLoader)loader).getURLs();

            for (int i=0; i<urls.length; i++) {
                if (isJarURL(urls[i])) {
                    url = urls[i];
                    break;
                }
            }
        }

        if (url == null) {
            if (isRequired) {
                throw new ArchLoaderException("Unable to find " +
                                              getJarName());
            }
            else {
                return null;
            }
        }
        
        path = url.getFile();

        if (path.startsWith("file:")) {
            path = path.substring(5);
        }
        File file = new File(path);
        String jarName = getJarName();

        while ((file != null) &&
               !file.getName().startsWith(jarName))
        {
            file = file.getParentFile();
        }

        if (libName == null) {
            //return path containing ${this.name}.jar
            //which may or may not be -x.x.x.x versioned
            libName = jarName;
        }

        if ((file != null) &&
            ((file = file.getParentFile()) != null))
        {
            String dir = URLDecoder.decode(file.toString()); 
            if (findNativeLibrary(dir, libName)) {
                return dir;
            }
        }

        return null;
    }

    //java.lang.ClassLoader does not delegate native linking
    //to the parent ClassLoader.
    //overriding these methods provides a workaround.
    protected void systemLoadLibrary(String name) {
        System.loadLibrary(name);
    }

    protected void systemLoad(String name) {
        System.load(name);
    }

    protected boolean containsNativeLibrary(File dir, String name) {
        if (name == null) {
            return false;
        }
        File file = new File(dir, name);
        if (file.exists()) {
            this.nativeLibrary = file;
            return true;
        }

        return false;
    }

    protected boolean findNativeLibrary(String dir, String name) {
        //must be an absolute path; this allows relative to $PWD
        File path = new File(dir).getAbsoluteFile();
        if (containsNativeLibrary(path, name)) {
            return true;
        }
        else if (containsNativeLibrary(path, getVersionedLibraryName())) {
            return true;
        }
        //try w/o arch name (e.g. "libsigar.so")
        return containsNativeLibrary(path,
                                     getLibraryPrefix() +
                                     getName() +
                                     getLibraryExtension());
    }

    protected boolean findInJavaLibraryPath(String libName) {
        String path = System.getProperty("java.library.path", "");
        StringTokenizer tok =
            new StringTokenizer(path, File.pathSeparator);
        while (tok.hasMoreTokens()) {
            path = tok.nextToken();
            if (findNativeLibrary(path, libName)) {
                return true;
            }
        }
        return false;
    }

    protected void loadLibrary(String path)
        throws ArchNotSupportedException,
               ArchLoaderException {

        try {
            String libName = getLibraryName();

            if (path == null) {
                //e.g. org.hyperic.sigar.path
                path = System.getProperty(getPackageName() + ".path");
            }

            if (path != null) {
                if (path.equals("-")) {
                    return; //assume library is already loaded
                }
                findJarPath(null, false); //check for versioned .jar
                findNativeLibrary(path, libName);
            }
            else {
                if (findJarPath(libName, false) == null) {
                    findInJavaLibraryPath(libName);
                }
            }

            //nativeLibrary set when findJarPath() calls findNativeLibrary()
            if (this.nativeLibrary != null) {
                systemLoad(this.nativeLibrary.toString());
            }
            else {
                //LD_LIBRARY_PATH must be set for linux and solaris
                //SHLIB_PATH must be set for hpux
                //PATH must be set for windows
                systemLoadLibrary(libName);
            }
        } catch (RuntimeException e) {
            String reason = e.getMessage();
            if (reason == null) {
                reason = e.getClass().getName();
            }

            String msg =
                "Failed to load " + libName +
                ": " + reason;

            throw new ArchLoaderException(msg);
        }
    }

    public void load()
        throws ArchNotSupportedException,
               ArchLoaderException {
        load(null);
    }

    public void load(String path)
        throws ArchNotSupportedException,
               ArchLoaderException {
        /*
         * System.loadLibrary() is only supposed to load the library
         * once per-classloader.  but we make extra sure that is the case.
         */
        synchronized (this.loadLock) {
            if (this.loaded) {
                return;
            }
            loadLibrary(path);
            this.loaded = true;
        }
    }
}
